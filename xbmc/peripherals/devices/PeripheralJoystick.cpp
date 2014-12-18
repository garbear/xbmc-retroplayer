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
#include "utils/StringUtils.h"

using namespace ADDON;
using namespace PERIPHERALS;

CPeripheralJoystick::CPeripheralJoystick(const PeripheralScanResult& scanResult) :
  CPeripheral(scanResult),
  m_joystickIndex(0)
{
  m_strDeviceName = scanResult.m_strDeviceName.empty() ? g_localizeStrings.Get(35011) : scanResult.m_strDeviceName;
  m_features.push_back(FEATURE_JOYSTICK);

  std::vector<std::string> parts = StringUtils::Split(scanResult.m_strLocation, "/");
  if (parts.size() == 2)
  {
    // Set m_addon
    const std::string& strAddonId = parts[0];
    AddonPtr addon;
    if (CAddonMgr::Get().GetAddon(strAddonId, addon, ADDON_PERIPHERALDLL))
      m_addon = boost::dynamic_pointer_cast<CPeripheralAddon>(addon);

    if (!m_addon)
      CLog::Log(LOG_ERROR, "CPeripheralJoystick: Couldn't get add-on %s", strAddonId.c_str());

    // Set m_joystickIndex
    const std::string& strJoystickIndex = parts[1];
    char* p = NULL;
    m_joystickIndex = strtol(strJoystickIndex.c_str(), &p, 10);
  }
  else
  {
    CLog::Log(LOG_ERROR, "CPeripheralJoystick: Invalid location (%s)", scanResult.m_strLocation.c_str());
  }
}

bool CPeripheralJoystick::InitialiseFeature(const PeripheralFeature feature)
{
  if (feature == FEATURE_JOYSTICK)
  {
    // TODO
  }

  return CPeripheral::InitialiseFeature(feature);
}
 