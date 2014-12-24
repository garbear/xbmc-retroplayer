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

#include "PeripheralJoystick.h"
#include "addons/AddonManager.h"
#include "guilib/LocalizeStrings.h"
#include "peripherals/Peripherals.h"
#include "peripherals/bus/PeripheralBusAddon.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

using namespace ADDON;
using namespace PERIPHERALS;

CPeripheralJoystick::CPeripheralJoystick(const PeripheralScanResult& scanResult) :
  CPeripheral(scanResult)
{
  SetVendorID(scanResult.m_iVendorId);
  SetProductID(scanResult.m_iProductId);

  CPeripheralBusAddon* addonBus = static_cast<CPeripheralBusAddon*>(g_peripherals.GetBusByType(PERIPHERAL_BUS_ADDON));
  if (addonBus)
  {
    unsigned int index;
    if (addonBus->SplitLocation(scanResult.m_strLocation, m_addon, index))
      SetIndex(index);
    else
      CLog::Log(LOG_ERROR, "CPeripheralJoystick: Invalid location (%s)", scanResult.m_strLocation.c_str());
  }

  m_features.push_back(FEATURE_JOYSTICK);
}

bool CPeripheralJoystick::InitialiseFeature(const PeripheralFeature feature)
{
  if (!CPeripheral::InitialiseFeature(feature))
    return false;

  bool bReturn(false);

  if (feature == FEATURE_JOYSTICK)
  {
    if (m_addon)
      bReturn |= m_addon->GetJoystickInfo(Index(), *this);
    else
      bReturn = false;
  }

  return bReturn;
}

void CPeripheralJoystick::OnEvent(const PeripheralEvent& event)
{
  // TODO
  CLog::Log(LOG_DEBUG, "PERIPHERAL - received event for %s", Name().c_str());
}
