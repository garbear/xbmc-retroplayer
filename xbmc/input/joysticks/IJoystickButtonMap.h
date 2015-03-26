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

#include <stdint.h>
#include <string>

/*!
 * \ingroup joysticks
 *
 * \brief Button map interface to translate between the driver's raw
 *        button/hat/axis elements and physical joystick features.
 */
class IJoystickButtonMap
{
public:
  virtual ~IJoystickButtonMap(void) { }

  /*!
   * \brief Load the button map into memory
   *
   * \return True if button map is ready to start translating buttons, false otherwise
   */
  virtual bool Load(void) = 0;

  /*!
   * \brief Get the feature associated with a driver primitive
   *
   * \param button       The driver primitive (a button, hat direction or semi-axis)
   * \param featureIndex The resolved feature index, or unmodified if GetFeature() returns false
   *
   * \return True if the driver primitive is associated with a feature, false otherwise
   */
   virtual bool GetFeature(const CJoystickDriverPrimitive& button, unsigned int& featureIndex) = 0;

  /*!
   * \brief Get the driver primitive associated with a digital or analog button
   *
   * \param featureIndex   The feature's index from the game peripheral's layout
   * \param button         The resolved driver primitive
   *
   * \return True if the index resolved to a driver primitive, false if the feature
   *         didn't resolve or isn't a digital or analog button
   */
  virtual bool GetButton(unsigned int featureIndex, CJoystickDriverPrimitive& button) = 0;

  /*!
   * \brief Get the raw axis indices and polarity for the given analog stick
   *
   * \param featureIndex   The feature's index from the game peripheral's layout
   * \param horizIndex     The index of the axis corresponding to the analog
   *                           stick's horizontal motion, or -1 if unknown
   * \param horizInverted  False if right is positive, true if right is negative
   * \param vertIndex      The index of the axis corresponding to the analog
   *                           stick's vertical motion, or -1 if unknown
   * \param vertInverted   False if up is positive, true if up is negative
   *
   * \return True if the feature resolved to an analog stick with at least 1 known axis
   */
  virtual bool GetAnalogStick(unsigned int featureIndex, int& horizIndex, bool& horizInverted,
                                                         int& vertIndex,  bool& vertInverted) = 0;

  /*!
   * \brief Get the raw axis indices and polarity for the given accelerometer
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
   * \return True if the feature resolved to an accelerometer with at least 1 known axis
   */
  virtual bool GetAccelerometer(unsigned int featureIndex, int& xIndex, bool& xInverted,
                                                           int& yIndex, bool& yInverted,
                                                           int& zIndex, bool& zInverted) = 0;
};
