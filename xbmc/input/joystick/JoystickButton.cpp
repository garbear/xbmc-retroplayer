/*
 *      Copyright (C) 2007-2014 Team XBMC
 *      http://www.xbmc.org
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

#include "JoystickButton.h"
#include "Joystick.h"
#include "guilib/Key.h"
#include "input/ButtonTranslator.h"
#include "input/IInputHandler.h"
#include "utils/log.h"
#include "utils/StdString.h"

#include "Application.h" // ick (TODO)

#include <algorithm>

using namespace INPUT;
using namespace std;

CJoystickButton::CJoystickButton(CJoystick* joystick, unsigned int id)
 : m_joystick(joystick),
   m_id(id),
   m_index(id + 1),
   m_state(BUTTON_DEPRESSED)
{
}

void CJoystickButton::Activate()
{
  if (m_state == BUTTON_DEPRESSED)
  {
    CLog::Log(LOGDEBUG, "Joystick %d button %d pressed", m_joystick->GetID(), m_id);

    m_state = BUTTON_PRESSED;
    
    // Check to see if an action is registered for the button first
    int        actionID;
    CStdString actionName;
    bool       fullrange;
    if (!CButtonTranslator::GetInstance().TranslateJoystickString(g_application.GetActiveWindowID(), m_joystick->GetName(), m_index, JACTIVE_BUTTON, actionID, actionName, fullrange))
    {
      CLog::Log(LOGDEBUG, "-> Joystick %d button %d no registered action", m_joystick->GetID(), m_id);
      return;
    }
    
    CAction action(actionID, 1.0f, 0.0f, actionName);

    for (vector<IInputHandler*>::iterator it = m_inputHandlers.begin(); it != m_inputHandlers.end(); ++it)
      (*it)->ProcessButtonDown(m_joystick->GetID(), m_id, action);
  }
}

void CJoystickButton::Deactivate()
{
  if (m_state == BUTTON_PRESSED)
  {
    CLog::Log(LOGDEBUG, "Joystick %d button %d depressed", m_joystick->GetID(), m_id);

    m_state = BUTTON_DEPRESSED;

    for (vector<IInputHandler*>::iterator it = m_inputHandlers.begin(); it != m_inputHandlers.end(); ++it)
      (*it)->ProcessButtonUp(m_joystick->GetID(), m_id);
  }
}

void CJoystickButton::Attach(IInputHandler* inputHandler)
{
  m_inputHandlers.push_back(inputHandler);
}

void CJoystickButton::Detach(IInputHandler* inputHandler)
{
  vector<IInputHandler*>::iterator it = std::find(m_inputHandlers.begin(), m_inputHandlers.end(), inputHandler);
  if (it != m_inputHandlers.end())
    m_inputHandlers.erase(it);
}
