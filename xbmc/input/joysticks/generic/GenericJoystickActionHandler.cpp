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

#include "GenericJoystickActionHandler.h"
#include "guilib/GUIMessage.h"
#include "guilib/GUIWindowManager.h"
#include "windowing/WinEvents.h"

CGenericJoystickActionHandler &CGenericJoystickActionHandler::Get()
{
  static CGenericJoystickActionHandler sJoystickAction;
  return sJoystickAction;
}

bool CGenericJoystickActionHandler::OnButtonPress(unsigned int actionId, bool bPressed)
{
  return true; // TODO
}

bool CGenericJoystickActionHandler::OnButtonMotion(unsigned int id, float magnitude)
{
  return true; // TODO
}

bool CGenericJoystickActionHandler::OnAnalogStickMotion(unsigned int id, float x, float y)
{
  return true; // TODO
}

bool CGenericJoystickActionHandler::OnAnalogStickThreshold(unsigned int id, bool bPressed, HatDirection direction /* = HatDirectionNone */)
{
  return true; // TODO
}

bool CGenericJoystickActionHandler::OnAccelerometerMotion(unsigned int id, float x, float y, float z)
{
  return true; // TODO
}

int CGenericJoystickActionHandler::QuerySupportedGestures(float x, float y)
{
  CGUIMessage msg(GUI_MSG_GESTURE_NOTIFY, 0, 0, (int)x, (int)y);
  if (!g_windowManager.SendMessage(msg))
    return 0;

  return msg.GetParam1();
}

void CGenericJoystickActionHandler::focusControl(float x, float y)
{
  XBMC_Event newEvent;
  memset(&newEvent, 0, sizeof(newEvent));

  newEvent.type = XBMC_SETFOCUS;
  newEvent.focus.type = XBMC_SETFOCUS;
  newEvent.focus.x = (uint16_t)x;
  newEvent.focus.y = (uint16_t)y;

  CWinEvents::MessagePush(&newEvent);
}
