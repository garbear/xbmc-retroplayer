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

#include "addons/include/kodi_peripheral_types.h"
#include "input/joysticks/JoystickDriverPrimitive.h"
#include "input/joysticks/JoystickTypes.h"
#include "peripherals/addons/PeripheralAddon.h"

#include <map>
#include <string>

namespace PERIPHERALS
{
  class CAddonJoystickButtonMapRO
  {
  public:
    CAddonJoystickButtonMapRO(CPeripheral* device, const PeripheralAddonPtr& addon, const std::string& strControllerId);

    std::string ControllerID(void) const { return m_strControllerId; }
    bool Load(void);
    bool GetFeature(const CDriverPrimitive& primitive, JoystickFeature& feature);
    bool GetPrimitiveFeature(const JoystickFeature& feature, CDriverPrimitive& primitive);
    bool GetAnalogStick(const JoystickFeature& feature, CDriverPrimitive& up,
                                                        CDriverPrimitive& down,
                                                        CDriverPrimitive& right,
                                                        CDriverPrimitive& left);
    bool GetAccelerometer(const JoystickFeature& feature, CDriverPrimitive& positiveX,
                                                          CDriverPrimitive& positiveY,
                                                          CDriverPrimitive& positiveZ);

  private:
    typedef std::string Feature;
    typedef std::map<CDriverPrimitive, Feature> DriverMap;

    // Utility functions
    static DriverMap          CreateLookupTable(const JoystickFeatureMap& features);
    static CDriverPrimitive   ToPrimitive(const ADDON::DriverPrimitive& primitive);
    static HAT_DIRECTION      ToHatDirection(JOYSTICK_DRIVER_HAT_DIRECTION driverDirection);
    static SEMIAXIS_DIRECTION ToSemiAxisDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION dir);

    CPeripheral* const  m_device;
    PeripheralAddonPtr  m_addon;
    const std::string   m_strControllerId;
    JoystickFeatureMap  m_features;
    DriverMap           m_driverMap;
  };
}
