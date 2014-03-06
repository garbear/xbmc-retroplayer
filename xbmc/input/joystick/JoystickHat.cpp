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

#include "JoystickHat.h"
#include "Joystick.h"

using namespace INPUT;

CJoystickHatButton::CJoystickHatButton(CJoystick* joystick, unsigned int hatId, HAT_DIRECTION direction)
 : CJoystickButton(joystick, CalculateID(hatId, direction))
{
}

int CJoystickHatButton::CalculateID(unsigned int hatId, HAT_DIRECTION direction)
{
  return (1 << (int)direction) << 16 | hatId;
}

CJoystickHat::CJoystickHat(CJoystick* joystick, unsigned int hatId)
  : m_joystick(joystick),
    m_id(hatId),
    m_up(joystick, hatId, HAT_UP),
    m_right(joystick, hatId, HAT_RIGHT),
    m_down(joystick, hatId, HAT_DOWN),
    m_left(joystick, hatId, HAT_LEFT)
{
}

void CJoystickHat::SetDirection(HAT_STATE newState)
{
  // Which directions are activated in a given state
  CJoystickHatButton* pressMatrix[][4] =
  {
    { NULL,  NULL,     NULL,    NULL    }, // S_CENTERED
    { &m_up, NULL,     NULL,    NULL    }, // S_UP
    { &m_up, &m_right, NULL,    NULL    }, // S_UP_RIGHT
    { NULL,  &m_right, NULL,    NULL    }, // S_RIGHT
    { NULL,  &m_right, &m_down, NULL    }, // S_DOWN_RIGHT
    { NULL,  NULL,     &m_down, NULL    }, // S_DOWN
    { NULL,  NULL,     &m_down, &m_left }, // S_DOWN_LEFT
    { NULL,  NULL,     NULL,    &m_left }, // S_LEFT
    { &m_up, NULL,     NULL,    &m_left }  // S_UP_LEFT
  };

  CJoystickHatButton** before = pressMatrix[m_state];
  CJoystickHatButton** after = pressMatrix[newState];

  for (unsigned int i = HAT_UP; i < HAT_LEFT; i++) // UP, RIGHT, DOWN, LEFT
  {
    // Button was pressed
    if (before[i] == NULL && after[i] != NULL)
      after[i]->Activate();
    
    // Button was depressed
    else if (before[i] != NULL && after[i] == NULL)
      before[i]->Deactivate();
  }

  m_state = newState;
}

const char *CJoystickHat::GetDirection() const
{
  switch (m_state)
  {
    case S_UP:         return "N";
    case S_UP_RIGHT:   return "NE";
    case S_RIGHT:      return "E";
    case S_DOWN_RIGHT: return "SE";
    case S_DOWN:       return "S";
    case S_DOWN_LEFT:  return "SW";
    case S_LEFT:       return "W";
    case S_UP_LEFT:    return "NW";
  }
  return "centered";
}
