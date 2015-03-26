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

#include "JoystickTypes.h"

#include <string>

/*!
 * \ingroup joysticks
 * \brief Interface for handling input events for game peripherals
 */
class IJoystickInputHandler
{
public:
  virtual ~IJoystickInputHandler(void) { }

  /*!
   * \brief The add-on ID of the game peripheral associated with this input handler
   *
   * The device ID provided by the implementation serves as the context for the
   * feature indices below.
   *
   * \return The ID of the add-on extending kodi.game.peripheral
   */
  virtual std::string DeviceID(void) const = 0;

  /*!
   * \brief A digital button has been pressed or released
   *
   * \param featureIndex The index of the button declared in the add-on's layout
   * \param bPressed     True if pressed, false if released
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnButtonPress(unsigned int featureIndex, bool bPressed) { return false; }

  /*!
   * \brief An analog button (trigger or a pressure-sensitive button) has changed state
   *
   * \param featureIndex The index of the button declared in the add-on's layout
   * \param magnitude    The button pressure or trigger travel distance in the
   *                     closed interval [0, 1]
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnButtonMotion(unsigned int featureIndex, float magnitude) { return false; }

  /*!
   * \brief An analog stick has moved
   *
   * \param featureIndex The index of the analog stick declared in the add-on's layout
   * \param x            The x coordinate in the closed interval [-1, 1]
   * \param y            The y coordinate in the closed interval [-1, 1]
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnAnalogStickMotion(unsigned int featureIndex, float x, float y) { return false; }

  /*!
   * \brief An accelerometer's state has changed
   *
   * \param featureIndex The index of the accelerometer declared in the add-on's layout
   * \param x            The x coordinate in the closed interval [-1, 1]
   * \param y            The y coordinate in the closed interval [-1, 1]
   * \param z            The z coordinate in the closed interval [-1, 1]
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnAccelerometerMotion(unsigned int featureIndex, float x, float y, float z) { return false; }
};
