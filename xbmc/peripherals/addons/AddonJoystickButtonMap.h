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
    virtual bool GetFeature(const CDriverPrimitive& primitive, JoystickFeature& feature);
    virtual bool GetPrimitiveFeature(const JoystickFeature& feature, CDriverPrimitive& primitive);
    virtual bool AddPrimitiveFeature(const JoystickFeature& feature, const CDriverPrimitive& primitive);
    virtual bool GetAnalogStick(const JoystickFeature& feature, CDriverPrimitive& up,
                                                                CDriverPrimitive& down,
                                                                CDriverPrimitive& right,
                                                                CDriverPrimitive& left);
    virtual bool AddAnalogStick(const JoystickFeature& feature, const CDriverPrimitive& up,
                                                                const CDriverPrimitive& down,
                                                                const CDriverPrimitive& right,
                                                                const CDriverPrimitive& left);
    virtual bool GetAccelerometer(const JoystickFeature& feature, CDriverPrimitive& positiveX,
                                                                  CDriverPrimitive& positiveY,
                                                                  CDriverPrimitive& positiveZ);
    virtual bool AddAccelerometer(const JoystickFeature& feature, const CDriverPrimitive& positiveX,
                                                                  const CDriverPrimitive& positiveY,
                                                                  const CDriverPrimitive& positiveZ);

  private:
    PeripheralAddonPtr        m_addon;
    CAddonJoystickButtonMapRO m_buttonMapRO;
    CAddonJoystickButtonMapWO m_buttonMapWO;
  };
}
