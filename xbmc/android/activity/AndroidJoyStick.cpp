/*
 *      Copyright (C) 2012-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "AndroidJoyStick.h"
#include "AndroidExtra.h"
#include "XBMCApp.h"
#include "android/jni/View.h"
#include "android/activity/AndroidFeatures.h"
#include "input/joysticks/generic/GenericJoystickInputHandler.h"
#include "guilib/Key.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"

#include <android/input.h>


#include <math.h>

//#define DEBUG_VERBOSE

enum {
  AXIS_LEFT_STICK_L_R  = 1,
  AXIS_LEFT_STICK_U_D  = 2,
  AXIS_TRIGGER         = 3,
  AXIS_RIGHT_STICK_L_R = 4,
  AXIS_RIGHT_STICK_U_D = 5,
  AXIS_DPAD_L_R        = 6,
  AXIS_DPAD_U_D        = 7,
};

typedef struct {
  int32_t nativeKey;
  int16_t xbmcID;
} KeyMap;

// mapping to button codes
// TODO: Map to key values instead of ints
static const KeyMap ButtonMap[] = {
  { AKEYCODE_BUTTON_A        , 1 },
  { AKEYCODE_BUTTON_B        , 2 },
  { AKEYCODE_BUTTON_X        , 3 },
  { AKEYCODE_BUTTON_Y        , 4 },
  { AKEYCODE_BUTTON_L1       , 5 },
  { AKEYCODE_BUTTON_R1       , 6 },
  { AKEYCODE_BUTTON_SELECT   , 7 },
  { AKEYCODE_BUTTON_START    , 8 },
  { AKEYCODE_BUTTON_THUMBL   , 9 },
  { AKEYCODE_BUTTON_THUMBR   , 10 },
  { AKEYCODE_DPAD_UP         , 11 },
  { AKEYCODE_DPAD_DOWN       , 12 },
  { AKEYCODE_DPAD_LEFT       , 13 },
  { AKEYCODE_DPAD_RIGHT      , 14 },
  { AKEYCODE_BUTTON_L2       , 15 },
  { AKEYCODE_BUTTON_R2       , 16 },
  { AKEYCODE_BUTTON_C        , 17 },
  { AKEYCODE_BUTTON_Z        , 18 },
};

/************************************************************************/
/************************************************************************/
static float AxisClampAsButton(const APP_InputDeviceAxis &axis, float value)
{
  // Clamp Axis so it acts like a D-Pad, return -1, 0 or +1
  if (fabs(value) < axis.buttonclamp)
    return 0.0;
  else
    return value < 0.0 ? -1.0:1.0;
}

static void LogAxisValues(int axis_id, const APP_InputDeviceAxis &axis)
{
  CLog::Log(LOGDEBUG, "LogAxisValues: "
    "axis(%d) Enabled(%d) Max(%f) Min(%f) Range(%f) Flat(%f) Fuzz(%f)",
    axis_id, axis.enabled, axis.max, axis.min, axis.range, axis.flat, axis.fuzz);
}

static void SetAxisFromValues(const float min, const float max,
  const float flat, const float fuzz, const float range, APP_InputDeviceAxis &axis)
{
  axis.min  = min;
  axis.max  = max;
  axis.flat = flat;
  axis.fuzz = fuzz;
  axis.range= range;
  // precalc some internals
  axis.deadzone= axis.flat + axis.fuzz;
  if (axis.deadzone < 0.1f)
    axis.deadzone = 0.1f;
  axis.buttonclamp = axis.range / 4.0f;
}

static void SetupAxis(const CJNIViewInputDevice &input_device, APP_InputDeviceAxis &axis, int axis_id, int source)
{
  CJNIViewInputDeviceMotionRange range = input_device.getMotionRange(axis_id, source);

  SetAxisFromValues(range.getMin(), range.getMax(), range.getFlat(), range.getFuzz(), range.getRange(), axis);
  axis.enabled = true;
}

static void SetupJoySticks(APP_JoystickDevice *device_ptr, int device)
{
  device_ptr->id = device;
  memset(&device_ptr->x_hat,  0x00, sizeof(APP_InputDeviceAxis));
  memset(&device_ptr->y_hat,  0x00, sizeof(APP_InputDeviceAxis));
  memset(&device_ptr->x_axis, 0x00, sizeof(APP_InputDeviceAxis));
  memset(&device_ptr->y_axis, 0x00, sizeof(APP_InputDeviceAxis));
  memset(&device_ptr->z_axis, 0x00, sizeof(APP_InputDeviceAxis));
  memset(&device_ptr->rz_axis,0x00, sizeof(APP_InputDeviceAxis));

  CJNIViewInputDevice  input_device = CJNIViewInputDevice::getDevice(device_ptr->id);
  int device_sources = input_device.getSources();
  std::string device_name = input_device.getName();

  strcpy(device_ptr->name, device_name.c_str());

  device_ptr->vid = input_device.getVendorId();
  device_ptr->pid = input_device.getProductId();

  // TODO
  device_ptr->input_handler = new CGenericJoystickInputHandler(device_ptr->id, device_ptr->name, device_ptr->vid, device_ptr->pid);

  CLog::Log(LOGDEBUG, "SetupJoySticks:caching  id(%d), sources(%d), device(%s)",
    device_ptr->id, device_sources, device_name.c_str());

  CJNIList<CJNIViewInputDeviceMotionRange> device_ranges = input_device.getMotionRanges();
  for (int i = 0; i < device_ranges.size(); i++)
  {
    int axis = device_ranges.get(i).getAxis();
    int source = device_ranges.get(i).getSource();
#ifdef DEBUG_VERBOSE
    CLog::Log(LOGDEBUG, "SetupJoySticks:range(%d), axis(%d), source(%d)", i, axis, source);
#endif

    // ignore anything we do not understand
    if (source != AINPUT_SOURCE_JOYSTICK)
      continue;

    // match axis/source to our handlers
    // anything that is not present, will be disabled
    switch(axis)
    {
      // Left joystick
      case AMOTION_EVENT_AXIS_X:
        SetupAxis(input_device, device_ptr->x_axis,  axis, source);
        break;
      break;
      case AMOTION_EVENT_AXIS_Y:
        SetupAxis(input_device, device_ptr->y_axis,  axis, source);
        break;

      // Right joystick
      case AMOTION_EVENT_AXIS_Z:
        SetupAxis(input_device, device_ptr->z_axis,  axis, source);
        break;
      case AMOTION_EVENT_AXIS_RZ:
        SetupAxis(input_device, device_ptr->rz_axis, axis, source);
        break;

      // D-Pad
      case AMOTION_EVENT_AXIS_HAT_X:
        SetupAxis(input_device, device_ptr->x_hat,   axis, source);
        break;
      case AMOTION_EVENT_AXIS_HAT_Y:
        SetupAxis(input_device, device_ptr->y_hat,   axis, source);
      break;
    }
  }

  if (device_name.find("GameStick Controller") != std::string::npos)
  {
    // Right joystick seems to have a range of -0.5 to 0.5, fix the range
    // Production GameStick Controllers should not have this problem
    // and this quirk can vanish once verified.
    SetAxisFromValues(-0.5f, 0.5f, 0.1f, 0.0f, 1.0f, device_ptr->z_axis);
    SetAxisFromValues(-0.5f, 0.5f, 0.1f, 0.0f, 1.0f, device_ptr->rz_axis);
  }

#ifdef DEBUG_VERBOSE
  LogAxisValues(AMOTION_EVENT_AXIS_X,     device_ptr->x_axis);
  LogAxisValues(AMOTION_EVENT_AXIS_Y,     device_ptr->y_axis);
  LogAxisValues(AMOTION_EVENT_AXIS_Z,     device_ptr->z_axis);
  LogAxisValues(AMOTION_EVENT_AXIS_RZ,    device_ptr->rz_axis);
  LogAxisValues(AMOTION_EVENT_AXIS_HAT_X, device_ptr->x_hat);
  LogAxisValues(AMOTION_EVENT_AXIS_HAT_Y, device_ptr->y_hat);
#endif
}

static void ReleaseJoySticks(APP_JoystickDevice *device_ptr)
{
  delete device_ptr->input_handler; // TODO
}

/************************************************************************/
/************************************************************************/
CAndroidJoyStick::~CAndroidJoyStick()
{
  while (!m_input_devices.empty())
  {
    APP_JoystickDevice *device = m_input_devices.back();
    ReleaseJoySticks(device);
    delete device;
    m_input_devices.pop_back();
  }
}

bool CAndroidJoyStick::onJoyStickKeyEvent(AInputEvent *event)
{
  if (event == NULL)
    return false;

  // watch this check, others might be different.
  // AML IR Controller is       AINPUT_SOURCE_GAMEPAD | AINPUT_SOURCE_KEYBOARD | AINPUT_SOURCE_DPAD
  // Gamestick Controller    == AINPUT_SOURCE_GAMEPAD | AINPUT_SOURCE_KEYBOARD
  // NVidiaShield Controller == AINPUT_SOURCE_GAMEPAD | AINPUT_SOURCE_KEYBOARD
  // we want to reject AML IR Controller.
  if (AInputEvent_getSource(event) != (AINPUT_SOURCE_GAMEPAD | AINPUT_SOURCE_KEYBOARD))
    return false;

  // GamePad events are AINPUT_EVENT_TYPE_KEY events,
  // trap them here and revector valid ones as JoyButtons
  // so we get keymap handling.
  uint8_t button  = 0;
  int32_t keycode = AKeyEvent_getKeyCode(event);

  for (size_t i = 0; i < sizeof(ButtonMap) / sizeof(KeyMap); i++)
  {
    if (keycode == ButtonMap[i].nativeKey)
    {
      button = ButtonMap[i].xbmcID;
      break;
    }
  }

  if (button == 0)
    return false;

  IJoystickInputHandler *input_handler = NULL;
  int32_t                device        = AInputEvent_getDeviceId(event);

  for (size_t i = 0; i < m_input_devices.size(); i++)
  {
    if (m_input_devices[i]->id == device)
    {
      input_handler = m_input_devices[i]->input_handler;
      break;
    }
  }

  if (!input_handler)
    return false;

  int32_t  action   = AKeyEvent_getAction(event);
  bool     bPressed = (action != AKEY_EVENT_ACTION_UP);

  return input_handler->HandleJoystickEvent(JoystickEventRawButton, button, 0, bPressed);
}

bool CAndroidJoyStick::onJoyStickMotionEvent(AInputEvent *event)
{
  if (event == NULL)
    return false;

  // match this device to a created device struct,
  // create it if we do not find it.
  APP_JoystickDevice *device_ptr = NULL;
  int32_t device = AInputEvent_getDeviceId(event);

  // look for device name in our inputdevice cache.
  for (size_t i = 0; i < m_input_devices.size(); i++)
  {
    if (m_input_devices[i]->id == device)
      device_ptr = m_input_devices[i];
  }
  if (!device_ptr)
  {
    // as we see each device, create a device and cache it.
    device_ptr = new APP_JoystickDevice;
    SetupJoySticks(device_ptr, device);
    m_input_devices.push_back(device_ptr);
  }

  // handle queued motion events, we
  // ingnore history as it only relates to touch.
  for (size_t p = 0; p < AMotionEvent_getPointerCount(event); p++)
    ProcessMotionEvents(event, p, device, device_ptr);

  return true;
}

void CAndroidJoyStick::ProcessMotionEvents(AInputEvent *event,
  size_t pointer_index, int32_t device, APP_JoystickDevice *device_ptr)
{
  // Left joystick
  if (device_ptr->y_axis.enabled)
    ProcessAxis(event, pointer_index, device_ptr->y_axis, device, AXIS_LEFT_STICK_L_R, AMOTION_EVENT_AXIS_Y);
  if (device_ptr->x_axis.enabled)
    ProcessAxis(event, pointer_index, device_ptr->x_axis, device, AXIS_LEFT_STICK_U_D, AMOTION_EVENT_AXIS_X);

  // Right joystick
  if (device_ptr->z_axis.enabled)
    ProcessAxis(event, pointer_index, device_ptr->z_axis, device, AXIS_RIGHT_STICK_L_R, AMOTION_EVENT_AXIS_Z);
  if (device_ptr->rz_axis.enabled)
    ProcessAxis(event, pointer_index, device_ptr->rz_axis,device, AXIS_RIGHT_STICK_U_D, AMOTION_EVENT_AXIS_RZ);

  // Dpad
  if (device_ptr->y_hat.enabled)
    ProcessAxis(event, pointer_index,  device_ptr->y_hat,  device, AXIS_DPAD_L_R, AMOTION_EVENT_AXIS_HAT_Y);
  if (device_ptr->x_hat.enabled)
    ProcessAxis(event, pointer_index,  device_ptr->x_hat,  device, AXIS_DPAD_U_D, AMOTION_EVENT_AXIS_HAT_X);

#ifdef DEBUG_VERBOSE
  CLog::Log(LOGDEBUG, "joystick event. x(%f),  y(%f)", device_ptr->x_axis.value, device_ptr->y_axis.value);
  CLog::Log(LOGDEBUG, "joystick event. z(%f), rz(%f)", device_ptr->z_axis.value, device_ptr->rz_axis.value);
  CLog::Log(LOGDEBUG, "joystick event. xhat(%f), yhat(%f)", device_ptr->x_hat.value, device_ptr->y_hat.value);
#endif
}

void CAndroidJoyStick::ProcessAxis(AInputEvent *event, size_t pointer_index,
  APP_InputDeviceAxis &axis, int device, int keymap_axis, int android_axis)
{
  float value = AMotionEvent_getAxisValue(event, android_axis, pointer_index);
  CLog::Log(LOGDEBUG, "ProcessAxis: keymap_axis(%d), value(%f)", keymap_axis, value);

  value = AxisClampAsButton(axis, value);
  if (value != axis.value)
  {
    for (size_t i = 0; i < m_input_devices.size(); i++)
    {
      if (m_input_devices[i]->id == device)
        m_input_devices[i]->input_handler->HandleJoystickEvent(JoystickEventRawAxis, keymap_axis, 0, false, HatDirectionNone, value);
    }
  }
  axis.value = value;
}
