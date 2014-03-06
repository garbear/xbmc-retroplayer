#pragma once
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

namespace INPUT
{

enum HAT_DIRECTION
{
  HAT_UP    = 0,
  HAT_RIGHT = 1,
  HAT_DOWN  = 2,
  HAT_LEFT  = 3,
};

enum HAT_STATE
{
  S_CENTERED,
  S_UP,
  S_UP_RIGHT,
  S_RIGHT,
  S_DOWN_RIGHT,
  S_DOWN,
  S_DOWN_LEFT,
  S_LEFT,
  S_UP_LEFT
};

class CJoystickHat;

/**
 * A hat is an arrow-based device on a gamepad. Each direction corresponds to a
 * button: up, right, down or left. CJoystickHat contains four buttons of class
 * CJoystickHatButton. This class wraps a CJoystickButton and encapsulates the
 * code specific to a hat's buttons.
 */
class CJoystickHatButton : public CJoystickButton
{
public:
  CJoystickHatButton(CJoystick* joystick, unsigned int hatId, HAT_DIRECTION direction);
  virtual ~CJoystickHatButton() { }

private:
  static int CalculateID(unsigned int hatId, HAT_DIRECTION direction);
};

class CJoystickHat
{
public:
  CJoystickHat(CJoystick* joystick, unsigned int hatId);

  unsigned int GetID() const { return m_id; }

  void SetDirection(HAT_STATE state);

private:
  /**
    * Helper function to translate this hat into a cardinal direction
    * ("N", "NE", "E", ...) or "centered".
    */
  const char *CJoystickHat::GetDirection() const;

  CJoystick*   m_joystick;
  unsigned int m_id;
  HAT_STATE    m_state;

  // Direction buttons
  CJoystickHatButton m_up;
  CJoystickHatButton m_right;
  CJoystickHatButton m_down;
  CJoystickHatButton m_left;
};

}
