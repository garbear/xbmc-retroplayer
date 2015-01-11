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

#include "AddonJoystickButtonMapRO.h"
#include "AddonJoystickButtonMapWO.h"
#include "input/joysticks/IJoystickButtonMap.h"
#include "peripherals/addons/PeripheralAddon.h"

namespace PERIPHERALS
{
  class CAddonJoystickButtonMap : public IJoystickButtonMap
  {
  public:
    CAddonJoystickButtonMap(CPeripheral* device, const std::string& strControllerId);

    virtual ~CAddonJoystickButtonMap(void);

    // Implementation of IJoystickButtonMap
    virtual std::string ControllerID(void) const { return m_buttonMapRO.ControllerID(); }
    virtual bool Load(void);
    virtual bool GetFeature(const CJoystickDriverPrimitive& primitive, std::string& feature);
    virtual bool GetButton(const std::string& feature, CJoystickDriverPrimitive& button);
    virtual bool MapButton(const std::string& feature, const CJoystickDriverPrimitive& primitive);
    virtual bool GetAnalogStick(const std::string& feature, int& horizIndex, bool& horizInverted,
                                                            int& vertIndex,  bool& vertInverted);
    virtual bool MapAnalogStick(const std::string& feature, int horizIndex, bool horizInverted,
                                                            int vertIndex,  bool vertInverted);
    virtual bool GetAccelerometer(const std::string& feature, int& xIndex, bool& xInverted,
                                                              int& yIndex, bool& yInverted,
                                                              int& zIndex, bool& zInverted);
    virtual bool MapAccelerometer(const std::string& feature, int xIndex, bool xInverted,
                                                              int yIndex, bool yInverted,
                                                              int zIndex, bool zInverted);

  private:
    PeripheralAddonPtr        m_addon;
    CAddonJoystickButtonMapRO m_buttonMapRO;
    CAddonJoystickButtonMapWO m_buttonMapWO;
  };
}
