#pragma once
/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "Peripheral.h"
#include "addons/include/xbmc_peripheral_utils.hpp"
#include "input/joysticks/generic/GenericJoystickInputHandler.h"
#include "peripherals/PeripheralAddon.h"

class CKey;

namespace PERIPHERALS
{
  class CPeripheralJoystick : public CPeripheral, // TODO: extend CPeripheralHID
                              public CGenericJoystickInputHandler
  {
  public:
    CPeripheralJoystick(const PeripheralScanResult& scanResult);
    virtual ~CPeripheralJoystick(void) { }

    virtual bool InitialiseFeature(const PeripheralFeature feature);

    unsigned int Index(void) const { return m_index; }

    /*
    unsigned int RequestedPort(void) const { return m_requestedPort; }
    unsigned int ButtonCount(void) const { return m_buttonCount; }
    unsigned int HatCount(void) const { return m_hatCount; }
    unsigned int AxisCount(void) const { return m_axisCount; }
    */

  private:
    unsigned int       m_index;
    unsigned int       m_requestedPort;
    unsigned int       m_buttonCount;
    unsigned int       m_hatCount;
    unsigned int       m_axisCount;
    //std::vector<CKey*> m_keys;
    PeripheralAddonPtr m_addon;
  };
}
