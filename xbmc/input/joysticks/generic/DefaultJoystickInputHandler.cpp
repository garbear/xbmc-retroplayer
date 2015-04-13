/*
 *      Copyright (C) 2014 Team XBMC
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

#include "DefaultJoystickInputHandler.h"
#include "guilib/GUIWindowManager.h"
#include "input/ButtonTranslator.h"
#include "input/joysticks/JoystickTranslator.h"
#include "input/Key.h"
#include "threads/SingleLock.h"
#include "ApplicationMessenger.h"

#include <algorithm>

#define HOLD_TIMEOUT_MS    500 // TODO
#define REPEAT_TIMEOUT_MS  250 // TODO

#ifndef ABS
#define ABS(x)  ((x) >= 0 ? (x) : (-x))
#endif

#ifndef MAX
#define MAX(x, y)  ((x) >= (y) ? (x) : (y))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))
#endif

CDefaultJoystickInputHandler::CDefaultJoystickInputHandler(void) :
  m_holdTimer(this),
  m_lastButtonPress(0)
{
}

std::string CDefaultJoystickInputHandler::ControllerID(void) const
{
  return DEFAULT_GAME_CONTROLLER;
}

bool CDefaultJoystickInputHandler::OnButtonPress(unsigned int featureIndex, bool bPressed)
{
  const JoystickFeatureID id = GetFeatureID(featureIndex);

  unsigned int buttonKeyId = GetButtonKeyID(id);
  if (buttonKeyId != 0)
  {
    CAction action(CButtonTranslator::GetInstance().GetAction(g_windowManager.GetActiveWindowID(), CKey(buttonKeyId, 0)));
    if (action.GetID() > 0)
    {
      if (action.IsAnalog())
      {
        const float amount = bPressed ? 1.0f : 0.0f;
        CAction actionWithAmount(action.GetID(), amount, 0.0f, action.GetName());
        CApplicationMessenger::Get().SendAction(action);
      }
      else
      {
        if (bPressed)
          ProcessButtonPress(action);
        else
          ProcessButtonRelease(buttonKeyId);
      }
    }
  }

  return true;
}

bool CDefaultJoystickInputHandler::OnButtonMotion(unsigned int featureIndex, float magnitude)
{
  const JoystickFeatureID id = GetFeatureID(featureIndex);

  unsigned int buttonKeyId = GetButtonKeyID(id);
  if (buttonKeyId != 0)
  {
    CAction action(CButtonTranslator::GetInstance().GetAction(g_windowManager.GetActiveWindowID(), CKey(buttonKeyId, 0)));
    if (action.GetID() > 0)
    {
      if (action.IsAnalog())
      {
        CAction actionWithAmount(action.GetID(), magnitude, 0.0f, action.GetName());
        CApplicationMessenger::Get().SendAction(action);
      }
      else
      {
        if (magnitude >= 0.5f)
          ProcessButtonPress(action);
        else if (magnitude < 0.5f)
          ProcessButtonRelease(buttonKeyId);
      }
    }
  }

  return true;
}

bool CDefaultJoystickInputHandler::OnAnalogStickMotion(unsigned int featureIndex, float x, float y)
{
  const JoystickFeatureID id = GetFeatureID(featureIndex);

  unsigned int buttonKeyId  = GetButtonKeyID(id, x, y);

  float magnitude = MAX(ABS(x), ABS(y));

  unsigned int buttonRightId = GetButtonKeyID(id,  1.0f,  0.0f);
  unsigned int buttonUpId    = GetButtonKeyID(id,  0.0f,  1.0f);
  unsigned int buttonLeftId  = GetButtonKeyID(id, -1.0f,  0.0f);
  unsigned int buttonDownId  = GetButtonKeyID(id,  0.0f, -1.0f);

  unsigned int buttonKeyIds[] = {buttonRightId, buttonUpId, buttonLeftId, buttonDownId};

  for (unsigned int i = 0; i < ARRAY_SIZE(buttonKeyIds); i++)
  {
    if (!buttonKeyIds[i])
      continue;

    CAction action(CButtonTranslator::GetInstance().GetAction(g_windowManager.GetActiveWindowID(), CKey(buttonKeyId, 0)));
    if (action.GetID() > 0)
    {
      if (action.IsAnalog())
      {
        if (buttonKeyId == buttonKeyIds[i])
        {
          CAction actionWithAmount(action.GetID(), magnitude, 0.0f, action.GetName());
          CApplicationMessenger::Get().SendAction(action);
        }
      }
    }
    else
    {
      if (buttonKeyId == buttonKeyIds[i])
      {
        if (magnitude >= 0.5f)
          ProcessButtonPress(action);
        else if (magnitude < 0.5f)
          ProcessButtonRelease(buttonKeyId);
      }
      else
      {
        ProcessButtonRelease(buttonKeyId);
      }
    }
  }

  return true;
}

bool CDefaultJoystickInputHandler::OnAccelerometerMotion(unsigned int featureIndex, float x, float y, float z)
{
  return OnAnalogStickMotion(featureIndex, x, y); // TODO
}

void CDefaultJoystickInputHandler::OnTimeout(void)
{
  CSingleLock lock(m_digitalMutex);

  const unsigned int holdTimeMs = (unsigned int)m_holdTimer.GetElapsedMilliseconds();

  if (m_lastButtonPress != 0 && holdTimeMs >= HOLD_TIMEOUT_MS)
  {
    CAction action(CButtonTranslator::GetInstance().GetAction(g_windowManager.GetActiveWindowID(), CKey(m_lastButtonPress, holdTimeMs)));
    if (action.GetID() > 0)
      CApplicationMessenger::Get().SendAction(action);
  }
}

void CDefaultJoystickInputHandler::ProcessButtonPress(const CAction& action)
{
  if (std::find(m_pressedButtons.begin(), m_pressedButtons.end(), action.GetButtonCode()) == m_pressedButtons.end())
  {
    ClearHoldTimer();

    CApplicationMessenger::Get().SendAction(action);

    CSingleLock lock(m_digitalMutex);

    m_pressedButtons.push_back(action.GetButtonCode());
    StartHoldTimer(action.GetButtonCode());
  }
}

void CDefaultJoystickInputHandler::ProcessButtonRelease(unsigned int buttonKeyId)
{
  std::vector<unsigned int>::iterator it = std::find(m_pressedButtons.begin(), m_pressedButtons.end(), buttonKeyId);
  if (it != m_pressedButtons.end())
  {
    m_pressedButtons.erase(it);

    if (buttonKeyId == m_lastButtonPress || m_pressedButtons.empty())
      ClearHoldTimer();
  }
}

void CDefaultJoystickInputHandler::StartHoldTimer(unsigned int buttonKeyId)
{
  m_holdTimer.Start(REPEAT_TIMEOUT_MS, true);
  m_lastButtonPress = buttonKeyId;
}

void CDefaultJoystickInputHandler::ClearHoldTimer(void)
{
  m_holdTimer.Stop(true);
  m_lastButtonPress = 0;
}

JoystickFeatureID CDefaultJoystickInputHandler::GetFeatureID(unsigned int featureIndex)
{
  // TODO: Use game peripheral to translate
  switch (featureIndex)
  {
    case  0: return JoystickIDButtonA;
    case  1: return JoystickIDButtonB;
    case  2: return JoystickIDButtonX;
    case  3: return JoystickIDButtonY;
    case  4: return JoystickIDButtonStart;
    case  5: return JoystickIDButtonBack;
    case  6: return JoystickIDButtonGuide;
    case  7: return JoystickIDButtonLeftBumper;
    case  8: return JoystickIDButtonRightBumper;
    case  9: return JoystickIDButtonLeftStick;
    case 10: return JoystickIDButtonRightStick;
    case 11: return JoystickIDButtonUp;
    case 12: return JoystickIDButtonRight;
    case 13: return JoystickIDButtonDown;
    case 14: return JoystickIDButtonLeft;
    case 15: return JoystickIDTriggerLeft;
    case 16: return JoystickIDTriggerRight;
    case 17: return JoystickIDAnalogStickLeft;
    case 18: return JoystickIDAnalogStickRight;
    case 19: return JoystickIDAccelerometer;
    default:
      break;
  }
  return JoystickIDButtonA;
}

unsigned int CDefaultJoystickInputHandler::GetButtonKeyID(JoystickFeatureID id,
                                                          float x /* = 0.0f */,
                                                          float y /* = 0.0f */,
                                                          float z /* = 0.0f */)
{
  switch (id)
  {
    case JoystickIDButtonA:           return KEY_BUTTON_A;
    case JoystickIDButtonB:           return KEY_BUTTON_B;
    case JoystickIDButtonX:           return KEY_BUTTON_X;
    case JoystickIDButtonY:           return KEY_BUTTON_Y;
    case JoystickIDButtonStart:       return KEY_BUTTON_START;
    case JoystickIDButtonBack:        return KEY_BUTTON_BACK;
    case JoystickIDButtonGuide:       return KEY_BUTTON_GUIDE;
    case JoystickIDButtonLeftBumper:  return KEY_BUTTON_LEFT_SHOULDER;
    case JoystickIDButtonRightBumper: return KEY_BUTTON_RIGHT_SHOULDER;
    case JoystickIDButtonLeftStick:   return KEY_BUTTON_LEFT_THUMB_BUTTON;
    case JoystickIDButtonRightStick:  return KEY_BUTTON_RIGHT_THUMB_BUTTON;
    case JoystickIDButtonUp:          return KEY_BUTTON_DPAD_UP;
    case JoystickIDButtonRight:       return KEY_BUTTON_DPAD_RIGHT;
    case JoystickIDButtonDown:        return KEY_BUTTON_DPAD_DOWN;
    case JoystickIDButtonLeft:        return KEY_BUTTON_DPAD_LEFT;
    case JoystickIDTriggerLeft:       return KEY_BUTTON_LEFT_TRIGGER;
    case JoystickIDTriggerRight:      return KEY_BUTTON_RIGHT_TRIGGER;
    case JoystickIDAnalogStickLeft:
      if      (y >= x && y >  -x)       return KEY_BUTTON_LEFT_THUMB_STICK_UP;
      else if (y <  x && y >= -x)       return KEY_BUTTON_LEFT_THUMB_STICK_RIGHT;
      else if (y <= x && y <  -x)       return KEY_BUTTON_LEFT_THUMB_STICK_DOWN;
      else if (y >  x && y <= -x)       return KEY_BUTTON_LEFT_THUMB_STICK_LEFT;
      break;
    case JoystickIDAnalogStickRight:
      if      (y >= x && y >  -x)       return KEY_BUTTON_RIGHT_THUMB_STICK_UP;
      else if (y <  x && y >= -x)       return KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
      else if (y <= x && y <  -x)       return KEY_BUTTON_RIGHT_THUMB_STICK_DOWN;
      else if (y >  x && y <= -x)       return KEY_BUTTON_RIGHT_THUMB_STICK_LEFT;
      break;
    case JoystickIDAccelerometer:
      return 0; // TODO
    default:
      break;
  }
  return 0;
}
