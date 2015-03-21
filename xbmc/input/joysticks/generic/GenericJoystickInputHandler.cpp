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

#include "GenericJoystickInputHandler.h"
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

CGenericJoystickInputHandler::CGenericJoystickInputHandler(void) :
  m_holdTimer(this),
  m_lastButtonPress(0)
{
}

bool CGenericJoystickInputHandler::OnButtonPress(JoystickFeatureID id, bool bPressed)
{
  unsigned int buttonKeyId = CJoystickTranslator::GetButtonKeyID(id);
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

bool CGenericJoystickInputHandler::OnButtonMotion(JoystickFeatureID id, float magnitude)
{
  unsigned int buttonKeyId = CJoystickTranslator::GetButtonKeyID(id);
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

bool CGenericJoystickInputHandler::OnAnalogStickMotion(JoystickFeatureID id, float x, float y)
{
  unsigned int buttonKeyId  = CJoystickTranslator::GetButtonKeyID(id, x, y);

  float magnitude = MAX(ABS(x), ABS(y));

  unsigned int buttonRightId = CJoystickTranslator::GetButtonKeyID(id,  1.0f,  0.0f);
  unsigned int buttonUpId    = CJoystickTranslator::GetButtonKeyID(id,  0.0f,  1.0f);
  unsigned int buttonLeftId  = CJoystickTranslator::GetButtonKeyID(id, -1.0f,  0.0f);
  unsigned int buttonDownId  = CJoystickTranslator::GetButtonKeyID(id,  0.0f, -1.0f);

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

bool CGenericJoystickInputHandler::OnAccelerometerMotion(JoystickFeatureID id, float x, float y, float z)
{
  return OnAnalogStickMotion(id, x, y); // TODO
}

void CGenericJoystickInputHandler::OnTimeout(void)
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

void CGenericJoystickInputHandler::ProcessButtonPress(const CAction& action)
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

void CGenericJoystickInputHandler::ProcessButtonRelease(unsigned int buttonKeyId)
{
  std::vector<unsigned int>::iterator it = std::find(m_pressedButtons.begin(), m_pressedButtons.end(), buttonKeyId);
  if (it != m_pressedButtons.end())
  {
    m_pressedButtons.erase(it);

    if (buttonKeyId == m_lastButtonPress || m_pressedButtons.empty())
      ClearHoldTimer();
  }
}

void CGenericJoystickInputHandler::StartHoldTimer(unsigned int buttonKeyId)
{
  m_holdTimer.Start(REPEAT_TIMEOUT_MS, true);
  m_lastButtonPress = buttonKeyId;
}

void CGenericJoystickInputHandler::ClearHoldTimer(void)
{
  m_holdTimer.Stop(true);
  m_lastButtonPress = 0;
}
