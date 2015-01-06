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

#include "IJoystickInputHandling.h"

#include <stdlib.h>

IJoystickInputHandling::IJoystickInputHandling(void)
  : m_handler(NULL)
{
}

void IJoystickInputHandling::RegisterHandler(IJoystickActionHandler *joystickActionHandler)
{
  m_handler = joystickActionHandler;
}

void IJoystickInputHandling::UnregisterHandler()
{
  m_handler = NULL;
}

bool IJoystickInputHandling::OnButtonPress(unsigned int id, bool bPressed)
{
  if (m_handler)
    return m_handler->OnButtonPress(id, bPressed);

  return true;
}

bool IJoystickInputHandling::OnButtonMotion(unsigned int id, float magnitude)
{
  if (m_handler)
    return m_handler->OnButtonMotion(id, magnitude);

  return true;
}

bool IJoystickInputHandling::OnAnalogStickMotion(unsigned int id, float x, float y)
{
  if (m_handler)
    return m_handler->OnAnalogStickMotion(id, x, y);

  return true;
}

bool IJoystickInputHandling::OnAnalogStickThreshold(unsigned int id, bool bPressed, HatDirection direction /* = HatDirectionNone */)
{
  if (m_handler)
    return m_handler->OnAnalogStickThreshold(id, bPressed, direction);

  return true;
}

bool IJoystickInputHandling::OnAccelerometerMotion(unsigned int id, float x, float y, float z)
{
  if (m_handler)
    return m_handler->OnAccelerometerMotion(id, x, y, z);

  return true;
}
