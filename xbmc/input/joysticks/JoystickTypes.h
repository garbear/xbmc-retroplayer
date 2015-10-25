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

#include <string>

/*!
 * \brief Add-on ID of the default game controller
 */
#define DEFAULT_GAME_CONTROLLER    "game.controller.default"

namespace JOYSTICK
{
  /*!
   * \brief Physical feature of the joystick - button, analog stick, etc
   */
  typedef std::string JoystickFeature;

  /*!
   * \brief Direction arrows on the hat (directional pad)
   */
  enum class HAT_DIRECTION
  {
    UNKNOWN = 0x0,
    UP      = 0x1,
    DOWN    = 0x2,
    RIGHT   = 0x4,
    LEFT    = 0x8,
  };

  /*!
   * \brief Generic typedef for cardinal directions
   */
  typedef HAT_DIRECTION CARDINAL_DIRECTION;

  /*!
   * \brief States in which a hat can be
   */
  enum class HAT_STATE
  {
    UNPRESSED = 0x0,    /*!< @brief no directions are pressed */
    UP        = 0x1,    /*!< @brief only up is pressed */
    DOWN      = 0x2,    /*!< @brief only down is pressed */
    RIGHT     = 0x4,    /*!< @brief only right is pressed */
    LEFT      = 0x8,    /*!< @brief only left is pressed */
    RIGHTUP   = RIGHT | UP,
    RIGHTDOWN = RIGHT | DOWN,
    LEFTUP    = LEFT  | UP,
    LEFTDOWN  = LEFT  | DOWN,
  };

  /*!
   * \brief Generic typedef for intercardinal directions
   */
  typedef HAT_STATE  INTERCARDINAL_DIRECTION;

  /*!
   * \ingroup joysticks
   * \brief Directions in which a semiaxis can point
   */
  enum class SEMIAXIS_DIRECTION
  {
    NEGATIVE = -1,  // semiaxis lies in the interval [-1.0, 0.0]
    UNKNOWN  =  0,  // semiaxis is unknown or invalid
    POSITIVE =  1,  // semiaxis lies in the interval [0.0, 1.0]
  };
}
