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
#pragma once

#define DEFAULT_GAME_CONTROLLER   "game.controller.default"

/*!
 * \ingroup joysticks
 * \brief Types of physical elements on a joystick that emit input events
 */
enum JoystickInputType
{
  JoystickDigitalButton,   /*!< @brief Button with two states, pressed and released */
  JoystickAnalogButton,    /*!< @brief Trigger or pressure-sensitive button */
  JoystickAnalogStick,     /*!< @brief Control stick capable of motion in two directions */
  JoystickAccelerometer,   /*!< @brief Accelerometer with x, y and z axes */
};

/*!
 * \ingroup joysticks
 * \brief Features available on the default controller
 */
enum JoystickFeatureID
{
  JoystickIDButtonUnknown = 0,
  JoystickIDButtonA,
  JoystickIDButtonB,
  JoystickIDButtonX,
  JoystickIDButtonY,
  JoystickIDButtonC,
  JoystickIDButtonZ,
  JoystickIDButtonStart,
  JoystickIDButtonSelect,
  JoystickIDButtonGuide,
  JoystickIDButtonLeftBumper,
  JoystickIDButtonRightBumper,
  JoystickIDButtonLeftStick,
  JoystickIDButtonRightStick,
  JoystickIDButtonUp,
  JoystickIDButtonRight,
  JoystickIDButtonDown,
  JoystickIDButtonLeft,
  JoystickIDTriggerLeft,
  JoystickIDTriggerRright,
  JoystickIDAnalogStickLeft,
  JoystickIDAnalogStickRight,
  JoystickIDAccelerometer,
};

/*!
 * \ingroup joysticks
 * \brief States in which a hat (directional pad) can be
 */
enum HatDirection
{
  HatDirectionNone      = 0x0,    /*!< @brief no directions are pressed */
  HatDirectionLeft      = 0x1,    /*!< @brief only left is pressed */
  HatDirectionRight     = 0x2,    /*!< @brief only right is pressed */
  HatDirectionUp        = 0x4,    /*!< @brief only up is pressed */
  HatDirectionDown      = 0x8,    /*!< @brief only down is pressed */
  HatDirectionLeftUp    = HatDirectionLeft  | HatDirectionUp,
  HatDirectionLeftDown  = HatDirectionLeft  | HatDirectionDown,
  HatDirectionRightUp   = HatDirectionRight | HatDirectionUp,
  HatDirectionRightDown = HatDirectionRight | HatDirectionDown,
};

/*!
 * \ingroup joysticks
 * \brief Directions in which a semiaxis can point
 */
enum SemiAxisDirection
{
  SemiAxisDirectionNegative = -1,  // semiaxis lies in the interval [-1.0, 0.0]
  SemiAxisDirectionUnknown  =  0,  // semiaxis is unknown or invalid
  SemiAxisDirectionPositive =  1,  // semiaxis lies in the interval [0.0, 1.0]
};
