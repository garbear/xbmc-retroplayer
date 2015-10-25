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
#include "input/joysticks/DriverPrimitive.h"
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
    bool GetFeature(const JOYSTICK::CDriverPrimitive& primitive, JOYSTICK::JoystickFeature& feature);
    bool GetPrimitiveFeature(const JOYSTICK::JoystickFeature& feature,
                             JOYSTICK::CDriverPrimitive& primitive);
    bool GetAnalogStick(const JOYSTICK::JoystickFeature& feature,
                        JOYSTICK::CDriverPrimitive& up,
                        JOYSTICK::CDriverPrimitive& down,
                        JOYSTICK::CDriverPrimitive& right,
                        JOYSTICK::CDriverPrimitive& left);
    bool GetAccelerometer(const JOYSTICK::JoystickFeature& feature,
                          JOYSTICK::CDriverPrimitive& positiveX,
                          JOYSTICK::CDriverPrimitive& positiveY,
                          JOYSTICK::CDriverPrimitive& positiveZ);

  private:
    typedef std::string Feature;
    typedef std::map<JOYSTICK::CDriverPrimitive, Feature> DriverMap;

    // Utility functions
    static DriverMap                    CreateLookupTable(const JoystickFeatureMap& features);
    static JOYSTICK::CDriverPrimitive   ToPrimitive(const ADDON::DriverPrimitive& primitive);
    static JOYSTICK::HAT_DIRECTION      ToHatDirection(JOYSTICK_DRIVER_HAT_DIRECTION driverDirection);
    static JOYSTICK::SEMIAXIS_DIRECTION ToSemiAxisDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION dir);

    CPeripheral* const  m_device;
    PeripheralAddonPtr  m_addon;
    const std::string   m_strControllerId;
    JoystickFeatureMap  m_features;
    DriverMap           m_driverMap;
  };
}
