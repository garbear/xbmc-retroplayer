#pragma once
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

#include "peripherals/bus/PeripheralBus.h"
#include "peripherals/PeripheralAddon.h"
#include "threads/CriticalSection.h"

namespace PERIPHERALS
{
  class CPeripheralBusAddon : public CPeripheralBus
  {
  public:
    CPeripheralBusAddon(CPeripherals *manager);
    virtual ~CPeripheralBusAddon(void);

    /*!
     * @see PeripheralBus::PerformDeviceScan()
     */
    bool PerformDeviceScan(PeripheralScanResults &results);

    bool GetAddon(const std::string &strId, ADDON::AddonPtr &addon) const;

    virtual void GetFeatures(std::vector<PeripheralFeature> &features) const;
    virtual bool HasFeature(const PeripheralFeature feature) const;

    virtual CPeripheral *GetPeripheral(const CStdString &strLocation) const;

  private:
    PeripheralAddonVector m_addons;
    CCriticalSection      m_critSection;
  };
}
