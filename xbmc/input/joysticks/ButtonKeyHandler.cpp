/*
 *      Copyright (C) 2015 Team XBMC
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

#include "ButtonKeyHandler.h"
#include "guilib/GUIWindowManager.h"
#include "input/ButtonTranslator.h"
#include "input/Key.h"
#include "ApplicationMessenger.h"

#include <algorithm>

#define HOLD_TIMEOUT_MS     500
#define REPEAT_TIMEOUT_MS   50

CButtonKeyHandler::CButtonKeyHandler(void)
  : m_holdTimer(this),
    m_lastButtonPress(0)
{
}

InputType CButtonKeyHandler::GetInputType(unsigned int buttonKeyId) const
{
  if (buttonKeyId != 0)
  {
    CAction action(CButtonTranslator::GetInstance().GetAction(g_windowManager.GetActiveWindowID(), CKey(buttonKeyId, 0)));
    if (action.GetID() > 0)
    {
      if (action.IsAnalog())
        return INPUT_TYPE_ANALOG;
      else
        return INPUT_TYPE_DIGITAL;
    }
  }

  return INPUT_TYPE_UNKNOWN;
}

void CButtonKeyHandler::OnDigitalButtonKey(unsigned int buttonKeyId, bool bPressed)
{
  if (buttonKeyId != 0)
  {
    if (bPressed && !IsHeld(buttonKeyId))
      ProcessButtonPress(buttonKeyId);
    else if (!bPressed && IsHeld(buttonKeyId))
      ProcessButtonRelease(buttonKeyId);
  }
}

void CButtonKeyHandler::OnAnalogButtonKey(unsigned int buttonKeyId, float magnitude)
{
  if (buttonKeyId != 0)
    SendAnalogAction(buttonKeyId, magnitude);
}

void CButtonKeyHandler::OnTimeout(void)
{
  const unsigned int holdTimeMs = (unsigned int)m_holdTimer.GetTotalElapsedMilliseconds();

  if (m_lastButtonPress != 0 && holdTimeMs >= HOLD_TIMEOUT_MS)
    SendDigitalAction(m_lastButtonPress, holdTimeMs);
}

bool CButtonKeyHandler::ProcessButtonPress(unsigned int buttonKeyId)
{
  ClearHoldTimer();

  m_pressedButtons.push_back(buttonKeyId);

  if (SendDigitalAction(buttonKeyId))
  {
    StartHoldTimer(buttonKeyId);
    return true;
  }

  return false;
}

void CButtonKeyHandler::ProcessButtonRelease(unsigned int buttonKeyId)
{
  m_pressedButtons.erase(std::remove(m_pressedButtons.begin(), m_pressedButtons.end(), buttonKeyId), m_pressedButtons.end());

  if (buttonKeyId == m_lastButtonPress || m_pressedButtons.empty())
    ClearHoldTimer();
}

bool CButtonKeyHandler::IsHeld(unsigned int buttonKeyId) const
{
  return std::find(m_pressedButtons.begin(), m_pressedButtons.end(), buttonKeyId) != m_pressedButtons.end();
}

void CButtonKeyHandler::StartHoldTimer(unsigned int buttonKeyId)
{
  m_lastButtonPress = buttonKeyId;
  m_holdTimer.Start(REPEAT_TIMEOUT_MS, true);
}

void CButtonKeyHandler::ClearHoldTimer(void)
{
  m_holdTimer.Stop(true);
  m_lastButtonPress = 0;
}

bool CButtonKeyHandler::SendDigitalAction(unsigned int buttonKeyId, unsigned int holdTimeMs /* = 0 */)
{
  CAction action(CButtonTranslator::GetInstance().GetAction(g_windowManager.GetActiveWindowID(), CKey(buttonKeyId, holdTimeMs)));
  if (action.GetID() > 0)
  {
    CApplicationMessenger::Get().SendAction(action);
    return true;
  }

  return false;
}

bool CButtonKeyHandler::SendAnalogAction(unsigned int buttonKeyId, float magnitude)
{
  CAction action(CButtonTranslator::GetInstance().GetAction(g_windowManager.GetActiveWindowID(), CKey(buttonKeyId, 0)));
  if (action.GetID() > 0)
  {
    CAction actionWithAmount(action.GetID(), magnitude, 0.0f, action.GetName());
    CApplicationMessenger::Get().SendAction(actionWithAmount);
    return true;
  }

  return false;
}
