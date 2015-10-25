/*
 *      Copyright (C) 2014-2015 Team XBMC
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
#pragma once

#include "JoystickTypes.h"

inline JOYSTICK::HAT_DIRECTION& operator|=(JOYSTICK::HAT_DIRECTION& lhs, JOYSTICK::HAT_DIRECTION rhs)
{
  return lhs = static_cast<JOYSTICK::HAT_DIRECTION>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline JOYSTICK::HAT_STATE& operator|=(JOYSTICK::HAT_STATE& lhs, JOYSTICK::HAT_STATE rhs)
{
  return lhs = static_cast<JOYSTICK::HAT_STATE>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline JOYSTICK::SEMIAXIS_DIRECTION operator*(JOYSTICK::SEMIAXIS_DIRECTION lhs, int rhs)
{
  return static_cast<JOYSTICK::SEMIAXIS_DIRECTION>(static_cast<int>(lhs) * rhs);
}
