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

#include "games/GameTypes.h"
#include "input/joysticks/IJoystickButtonMapper.h"
#include "peripherals/addons/AddonJoystickButtonMap.h"
#include "peripherals/addons/PeripheralAddon.h"

#include <string>

namespace PERIPHERALS
{
  class CAddonJoystickButtonMapper : public IJoystickButtonMapper
  {
  public:
    CAddonJoystickButtonMapper(CPeripheral* device, const std::string& strDeviceId);

    virtual ~CAddonJoystickButtonMapper(void) { }

    // Implementation of IJoystickButtonMapper
    virtual std::string DeviceID(void) const { return m_strDeviceId; }
    virtual bool Load(void);
    virtual bool MapButton(unsigned int featureIndex, const CJoystickDriverPrimitive& primitive);
    virtual bool MapAnalogStick(unsigned int featureIndex, int horizIndex, bool horizInverted,
                                                           int vertIndex,  bool vertInverted);
    virtual bool MapAccelerometer(unsigned int featureIndex, int xIndex, bool xInverted,
                                                             int yIndex, bool yInverted,
                                                             int zIndex, bool zInverted);

  private:
    const std::string& GetFeatureName(unsigned int featureIndex) const;

    // Utility functions
    static JOYSTICK_DRIVER_HAT_DIRECTION      ToHatDirection(HatDirection dir);
    static JOYSTICK_DRIVER_SEMIAXIS_DIRECTION ToSemiAxisDirection(SemiAxisDirection dir);

    CPeripheral* const              m_device;
    const PeripheralAddonPtr        m_addon;
    const std::string               m_strDeviceId;
    mutable GAME::GamePeripheralPtr m_gameDevice; // Loaded on demand
  };
}
