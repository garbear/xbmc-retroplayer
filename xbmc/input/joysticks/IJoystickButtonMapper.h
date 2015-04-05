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

#include "JoystickDriverPrimitive.h"

#include <string>

/*!
 * \ingroup joysticks
 *
 * \brief Button mapper interface to assign the driver's raw button/hat/axis
 *        elements to physical joystick features.
 *
 * \sa IJoystickButtonMap
 */
class IJoystickButtonMapper
{
public:
  virtual ~IJoystickButtonMapper(void) { }

  /*!
   * \brief The add-on ID of the game peripheral associated with this button mapper
   *
   * The device ID provided by the implementation serves as the context for the
   * feature indices below.
   *
   * \return The ID of this button mapper's game peripheral add-on
   */
  virtual std::string DeviceID(void) const = 0;

  /*!
   * \brief Load the button mapper into memory
   *
   * \return True if button mapper is ready to start mapping buttons, false otherwise
   */
  virtual bool Load(void) = 0;

  /*!
   * \brief Map a digital or analog button to a driver primitive
   *
   * \param featureIndex   The feature's index from the game peripheral's layout
   * \param primitive      The driver primitive
   *
   * \return True if the button was updated, false otherwise
   */
  virtual bool MapButton(unsigned int featureIndex, const CJoystickDriverPrimitive& primitive) = 0;

  /*!
   * \brief Map an analog stick to horizontal and vertical axes
   *
   * \param featureIndex   The feature's index from the game peripheral's layout
   * \param horizIndex     The index of the axis corresponding to the analog
   *                           stick's horizontal motion, or -1 if unknown
   * \param horizInverted  False if right is positive, true if right is negative
   * \param vertIndex      The index of the axis corresponding to the analog
   *                           stick's vertical motion, or -1 if unknown
   * \param vertInverted   False if up is positive, true if up is negative
   *
   * \return True if the analog stick was updated, false otherwise
   */
  virtual bool MapAnalogStick(unsigned int featureIndex, int horizIndex, bool horizInverted,
                                                         int vertIndex,  bool vertInverted) = 0;

  /*!
   * \brief Map an accelerometer to x, y and z axes
   *
   * \param featureIndex  The feature's index from the game peripheral's layout
   * \param xIndex        The index of the axis corresponding to the accelerometer's
   *                          X-axis, or -1 if unknown
   * \param xInverted     False if positive X is positive, true if positive X is negative
   * \param yIndex        The index of the axis corresponding to the accelerometer's
   *                          Y-axis, or -1 if unknown
   * \param yInverted     False if positive Y is positive, true if positive Y is negative
   * \param zIndex        The index of the axis corresponding to the accelerometer's
   *                          Z-axis, or -1 if unknown
   * \param zInverted     False if positive X is positive, true if positive Z is negative
   *
   * \return True if the accelerometer was updated, false otherwise
   */
  virtual bool MapAccelerometer(unsigned int featureIndex, int xIndex, bool xInverted,
                                                           int yIndex, bool yInverted,
                                                           int zIndex, bool zInverted) = 0;
};
