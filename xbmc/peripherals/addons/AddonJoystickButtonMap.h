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
  class CAddonJoystickButtonMap : public JOYSTICK::IJoystickButtonMap
  {
  public:
    CAddonJoystickButtonMap(CPeripheral* device, const std::string& strControllerId);

    virtual ~CAddonJoystickButtonMap(void);

    // Implementation of IJoystickButtonMap
    virtual std::string ControllerID(void) const { return m_buttonMapRO.ControllerID(); }
    virtual bool Load(void);
    bool GetFeature(const JOYSTICK::CDriverPrimitive& primitive,
                    JOYSTICK::JoystickFeature& feature);
    bool GetPrimitiveFeature(const JOYSTICK::JoystickFeature& feature,
                             JOYSTICK::CDriverPrimitive& primitive);
    bool AddPrimitiveFeature(const JOYSTICK::JoystickFeature& feature,
                             const JOYSTICK::CDriverPrimitive& primitive);
    bool GetAnalogStick(const JOYSTICK::JoystickFeature& feature,
                        JOYSTICK::CDriverPrimitive& up,
                        JOYSTICK::CDriverPrimitive& down,
                        JOYSTICK::CDriverPrimitive& right,
                        JOYSTICK::CDriverPrimitive& left);
    bool AddAnalogStick(const JOYSTICK::JoystickFeature& feature,
                        const JOYSTICK::CDriverPrimitive& up,
                        const JOYSTICK::CDriverPrimitive& down,
                        const JOYSTICK::CDriverPrimitive& right,
                        const JOYSTICK::CDriverPrimitive& left);
    bool GetAccelerometer(const JOYSTICK::JoystickFeature& feature,
                          JOYSTICK::CDriverPrimitive& positiveX,
                          JOYSTICK::CDriverPrimitive& positiveY,
                          JOYSTICK::CDriverPrimitive& positiveZ);
    bool AddAccelerometer(const JOYSTICK::JoystickFeature& feature,
                          const JOYSTICK::CDriverPrimitive& positiveX,
                          const JOYSTICK::CDriverPrimitive& positiveY,
                          const JOYSTICK::CDriverPrimitive& positiveZ);
    
  private:
    PeripheralAddonPtr        m_addon;
    CAddonJoystickButtonMapRO m_buttonMapRO;
    CAddonJoystickButtonMapWO m_buttonMapWO;
  };
}
