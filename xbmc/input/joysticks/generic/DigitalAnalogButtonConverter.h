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

#include "input/joysticks/IJoystickInputHandler.h"

#include <vector>

namespace JOYSTICK
{
  /*!
   * \brief Convert between digital and analog button events
   *
   * If the input type is digital, driver axes are converted to digital buttons by
   * thresholding around 0.5.
   *
   * If the input type is analog, driver buttons/hats are converted to analog
   * events with magnitude 0.0 or 1.0.
   */
  class CDigitalAnalogButtonConverter : public IJoystickInputHandler
  {
  public:
    CDigitalAnalogButtonConverter(IJoystickInputHandler* handler);

    // implementation of IJoystickInputHandler
    virtual std::string ControllerID(void) const;
    virtual InputType GetInputType(const JoystickFeature& feature) const;
    virtual bool OnButtonPress(const JoystickFeature& feature, bool bPressed);
    virtual bool OnButtonMotion(const JoystickFeature& feature, float magnitude);
    virtual bool OnAnalogStickMotion(const JoystickFeature& feature, float x, float y);
    virtual bool OnAccelerometerMotion(const JoystickFeature& feature, float x, float y, float z);

  private:
    bool IsActivated(const JoystickFeature& feature) const;
    void Activate(const JoystickFeature& feature);
    void Deactivate(const JoystickFeature& feature);

    IJoystickInputHandler* const  m_handler;
    std::vector<JoystickFeature>  m_activatedFeatures; // for tracking analog features mapped to digital input
  };
}
