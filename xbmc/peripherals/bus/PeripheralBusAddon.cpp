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

#include "PeripheralBusAddon.h"
#include "addons/AddonManager.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

#include <algorithm>

using namespace ADDON;
using namespace PERIPHERALS;

CPeripheralBusAddon::CPeripheralBusAddon(CPeripherals *manager) :
    CPeripheralBus("PeripBusAddon", manager, PERIPHERAL_BUS_ADDON)
{
}

CPeripheralBusAddon::~CPeripheralBusAddon(void)
{
}

bool CPeripheralBusAddon::PerformDeviceScan(PeripheralScanResults &results)
{
  VECADDONS addons;
  CAddonMgr::Get().GetAddons(ADDON_PERIPHERALDLL, addons, true);

  {
    CSingleLock lock(m_critSection);

    PeripheralAddonVector createdAddons = m_addons;
    m_addons.clear();

    for (VECADDONS::const_iterator it = addons.begin(); it != addons.end(); ++it)
    {
      PeripheralAddonPtr addon = boost::dynamic_pointer_cast<CPeripheralAddon>(*it);
      if (!addon)
        continue;

      // If add-on hasn't been created, try to create it now
      if (std::find(createdAddons.begin(), createdAddons.end(), addon) == createdAddons.end())
      {
        if (addon->Create() != ADDON_STATUS_OK)
          continue;
      }

      m_addons.push_back(addon);
    }

    for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
    {
      std::vector<JoystickConfiguration> joysticks;
      if (!(*itAddon)->PerformJoystickScan(joysticks))
        continue;

      for (std::vector<JoystickConfiguration>::const_iterator itJoystick = joysticks.begin(); itJoystick != joysticks.end(); ++itJoystick)
      {
        PeripheralScanResult result(m_type);
        result.m_type          = PERIPHERAL_JOYSTICK;
        result.m_strLocation   = StringUtils::Format("%s/%d", (*itAddon)->ID().c_str(), itJoystick->Index());
        result.m_iVendorId     = 0; // itJoystick->VendorID(); // TODO
        result.m_iProductId    = 0; // itJoystick->ProductID(); // TODO
        result.m_mappedType    = PERIPHERAL_JOYSTICK;
        result.m_mappedBusType = PERIPHERAL_BUS_ADDON;
        result.m_iSequence     = GetNumberOfPeripheralsWithId(result.m_iVendorId, result.m_iProductId);

        if (!results.ContainsResult(result))
          results.m_results.push_back(result);
      }
    }
  }

  return true;
}

bool CPeripheralBusAddon::GetAddon(const std::string &strId, AddonPtr &addon) const
{
  CSingleLock lock(m_critSection);
  for (PeripheralAddonVector::const_iterator it = m_addons.begin(); it != m_addons.end(); ++it)
  {
    if ((*it)->ID() == strId)
    {
      addon = *it;
      return true;
    }
  }
  return false;
}
