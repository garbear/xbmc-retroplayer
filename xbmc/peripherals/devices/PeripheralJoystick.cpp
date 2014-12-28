/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "PeripheralJoystick.h"
#include "addons/AddonManager.h"
#include "guilib/Key.h"s
#include "peripherals/Peripherals.h"
#include "peripherals/bus/PeripheralBusAddon.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

using namespace PERIPHERALS;

CPeripheralJoystick::CPeripheralJoystick(const PeripheralScanResult& scanResult) :
  CPeripheral(scanResult),
  m_index(0),
  m_requestedPort(0),
  m_buttonCount(0),
  m_hatCount(0),
  m_axisCount(0)
{
  CPeripheralBusAddon* addonBus = static_cast<CPeripheralBusAddon*>(g_peripherals.GetBusByType(PERIPHERAL_BUS_ADDON));
  if (addonBus)
  {
    if (!addonBus->SplitLocation(scanResult.m_strLocation, m_addon, m_index))
      CLog::Log(LOGERROR, "CPeripheralJoystick: Invalid location (%s)", scanResult.m_strLocation.c_str());
  }

  m_features.push_back(FEATURE_JOYSTICK);
}

bool CPeripheralJoystick::InitialiseFeature(const PeripheralFeature feature)
{
  if (!CPeripheral::InitialiseFeature(feature))
    return false;

  bool bReturn(false);

  if (feature == FEATURE_JOYSTICK)
  {
    ADDON::Joystick joystickInfo;
    if (m_addon && m_addon->GetJoystickInfo(Index(), joystickInfo))
    {
      bReturn = true;
      m_strDeviceName = joystickInfo.Name();
      m_requestedPort = joystickInfo.RequestedPlayer();
      m_buttonCount = joystickInfo.ButtonCount();
      m_hatCount = joystickInfo.HatCount();
      m_axisCount = joystickInfo.AxisCount();
    }
  }

  return bReturn;
}

/*
void CPeripheralJoystick::OnEvent(const ADDON::PeripheralEvent& event)
{
  // TODO
  CLog::Log(LOGDEBUG, "PERIPHERAL - received event for %s", m_strDeviceName.c_str());
  
  switch (event.Type())
  {
    case JOYSTICK_EVENT_TYPE_VIRTUAL_BUTTON:
      m_lastVirtualIndex = event.VirtualIndex();
      SetChanged();
      NotifyObservers(ObservableMessageButtonChanged);
      break;

    case JOYSTICK_EVENT_TYPE_VIRTUAL_HAT:
      m_lastVirtualIndex = event.VirtualIndex();
      SetChanged();
      NotifyObservers(ObservableMessageHatChanged);
      break;

    case JOYSTICK_EVENT_TYPE_VIRTUAL_AXIS:
      m_lastVirtualIndex = event.VirtualIndex();
      SetChanged();
      NotifyObservers(ObservableMessageAxisChanged);
      break;

    case JOYSTICK_EVENT_TYPE_BUTTON_DIGITAL:
    case JOYSTICK_EVENT_TYPE_BUTTON_ANALOG:
    case JOYSTICK_EVENT_TYPE_ANALOG_STICK:
    case JOYSTICK_EVENT_TYPE_ANALOG_STICK_THRESHOLD:
    case JOYSTICK_EVENT_TYPE_ACCELEROMETER:
    {
      int actionID = ACTION_NONE;
      switch (event.ButtonID())
      {
      case JOYSTICK_ID_BUTTON_1:
        actionID = KEY_BUTTON_A;
        break;
      case JOYSTICK_ID_BUTTON_2:
        actionID = KEY_BUTTON_B;
        break;
      case JOYSTICK_ID_BUTTON_3:
        actionID = KEY_BUTTON_X;
        break;
      case JOYSTICK_ID_BUTTON_4:
        actionID = KEY_BUTTON_Y;
        break;
      case JOYSTICK_ID_BUTTON_5:
        actionID = KEY_BUTTON_BLACK;
        break;
      case JOYSTICK_ID_BUTTON_6:
        actionID = KEY_BUTTON_WHITE;
        break;
      case JOYSTICK_ID_BUTTON_START:
        actionID = KEY_BUTTON_START;
        break;
      case JOYSTICK_ID_BUTTON_SELECT:
        actionID = KEY_BUTTON_BACK;
        break;
      case JOYSTICK_ID_BUTTON_HOME:
        //actionID = KEY_BUTTON_A; // TODO
        break;
      case JOYSTICK_ID_BUTTON_UP:
        actionID = KEY_BUTTON_DPAD_UP;
        break;
      case JOYSTICK_ID_BUTTON_DOWN:
        actionID = KEY_BUTTON_DPAD_DOWN;
        break;
      case JOYSTICK_ID_BUTTON_LEFT:
        actionID = KEY_BUTTON_DPAD_LEFT;
        break;
      case JOYSTICK_ID_BUTTON_RIGHT:
        actionID = KEY_BUTTON_DPAD_RIGHT;
        break;
      case JOYSTICK_ID_BUTTON_L:
        actionID = KEY_BUTTON_LEFT_SHOULDER;
        break;
      case JOYSTICK_ID_BUTTON_R:
        actionID = KEY_BUTTON_RIGHT_SHOULDER;
        break;
      case JOYSTICK_ID_BUTTON_L_STICK:
        actionID = KEY_BUTTON_LEFT_THUMB_BUTTON; // TODO
        break;
      case JOYSTICK_ID_BUTTON_R_STICK:
        actionID = KEY_BUTTON_RIGHT_THUMB_BUTTON; // TODO
        break;
      case JOYSTICK_ID_TRIGGER_L:
        actionID = KEY_BUTTON_LEFT_TRIGGER;
        break;
      case JOYSTICK_ID_TRIGGER_R:
        actionID = KEY_BUTTON_RIGHT_TRIGGER;
        break;
      case JOYSTICK_ID_ANALOG_STICK_LEFT:
        actionID = KEY_BUTTON_LEFT_THUMB_STICK_UP; // TODO
        break;
      case JOYSTICK_ID_ANALOG_STICK_RIGHT:
        actionID = KEY_BUTTON_RIGHT_THUMB_STICK_UP; // TODO
        break;
      case JOYSTICK_ID_ACCELEROMETER:
      case JOYSTICK_ID_GYRO:
        // TODO
        break;
      case JOYSTICK_ID_BUTTON_UNKNOWN:
      default:
        break;
      }
      if (event.DigitalState() == JOYSTICK_STATE_BUTTON_PRESSED)
      {
        m_keys.push_back(new CKey())
        CKey newKey(cecDevice->GetButton(), cecDevice->GetHoldTime());
        JOYSTICK_ID id = event.ButtonID();
      }
      break;
    }
    case JOYSTICK_EVENT_TYPE_NONE:
    default:
      break;
  }
}
*/

void CPeripheralJoystick::SetLastVirtualIndex(unsigned int index)
{
  m_lastVirtualIndex = index;
  SetChanged();
}
