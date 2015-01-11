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

#include "GenericJoystickKeyboardHandler.h"
#include "input/joysticks/IJoystickDriverHandler.h"
#include "input/Key.h"

#include <assert.h>

CGenericJoystickKeyboardHandler::CGenericJoystickKeyboardHandler(IJoystickDriverHandler* handler)
 : m_handler(handler)
{
  assert(m_handler);
}

bool CGenericJoystickKeyboardHandler::OnKeyPress(const CKey& key)
{
  unsigned int buttonIndex = GetButtonIndex(key);
  if (buttonIndex != 0)
    return m_handler->OnButtonMotion(buttonIndex, true);

  return false;
}

void CGenericJoystickKeyboardHandler::OnKeyRelease(const CKey& key)
{
  unsigned int buttonIndex = GetButtonIndex(key);
  if (buttonIndex != 0)
    m_handler->OnButtonMotion(buttonIndex, false);
}

unsigned int CGenericJoystickKeyboardHandler::GetButtonIndex(const CKey& key)
{
  return key.GetVKey();
}
