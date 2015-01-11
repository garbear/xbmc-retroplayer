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

#include "input/joysticks/IJoystickInputHandler.h"

#include <vector>

/*!
 * \brief Convert between digital and analog button events
 *
 * A game peripheral may contain a digital button mapped to a driver axis, or
 * an analog button mapped to a driver's digital button.
 *
 * This class converts these button presses to the format expected by the input
 * handler:
 *    - Digital button events are converted to analog events with magnitude 0.0 or 1.0
 *    - Analog button events are thresholded around 0.5 and reported as pressed/unpressed
 */
class CDigitalAnalogButtonConverter : public IJoystickInputHandler
{
public:
  CDigitalAnalogButtonConverter(IJoystickInputHandler* handler);

  // implementation of IJoystickInputHandler
  virtual std::string DeviceID(void) const;
  virtual bool OnButtonPress(unsigned int featureIndex, bool bPressed);
  virtual bool OnButtonMotion(unsigned int featureIndex, float magnitude);
  virtual bool OnAnalogStickMotion(unsigned int featureIndex, float x, float y);
  virtual bool OnAccelerometerMotion(unsigned int featureIndex, float x, float y, float z);

private:
  IJoystickInputHandler* const m_handler;
  std::vector<unsigned int>    m_pressedButtons; // for tracking digital buttons mapped to axes
};
