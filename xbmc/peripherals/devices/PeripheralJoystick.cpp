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
#include "peripherals/Peripherals.h"
#include "peripherals/bus/PeripheralBusAddon.h"
#include "utils/log.h"

#include <algorithm>

using namespace PERIPHERALS;

CPeripheralJoystick::CPeripheralJoystick(const PeripheralScanResult& scanResult) :
  CPeripheral(scanResult),
  m_requestedPort(JOYSTICK_PORT_UNKNOWN)
{
  m_features.push_back(FEATURE_JOYSTICK);
}

CPeripheralJoystick::~CPeripheralJoystick(void)
{
  UnregisterJoystickInputHandler(&m_defaultInputHandler);
}

bool CPeripheralJoystick::InitialiseFeature(const PeripheralFeature feature)
{
  bool bSuccess = false;

  if (CPeripheral::InitialiseFeature(feature))
  {
    if (feature == FEATURE_JOYSTICK)
    {
      if (m_mappedBusType == PERIPHERAL_BUS_ADDON)
      {
        CPeripheralBusAddon* addonBus = static_cast<CPeripheralBusAddon*>(g_peripherals.GetBusByType(PERIPHERAL_BUS_ADDON));
        if (addonBus)
        {
          PeripheralAddonPtr addon;
          unsigned int index;
          if (addonBus->SplitLocation(m_strLocation, addon, index))
            bSuccess = addon->SetJoystickProperties(index, *this);
          else
            CLog::Log(LOGERROR, "CPeripheralJoystick: Invalid location (%s)", m_strLocation.c_str());
        }
      }
    }
  }

  if (bSuccess)
    RegisterJoystickInputHandler(&m_defaultInputHandler);

  return bSuccess;
}

void CPeripheralJoystick::RegisterJoystickDriverHandler(IJoystickDriverHandler* handler)
{
  if (handler && std::find(m_driverHandlers.begin(), m_driverHandlers.end(), handler) == m_driverHandlers.end())
    m_driverHandlers.push_back(handler);
}

void CPeripheralJoystick::UnregisterJoystickDriverHandler(IJoystickDriverHandler* handler)
{
  m_driverHandlers.erase(std::remove(m_driverHandlers.begin(), m_driverHandlers.end(), handler), m_driverHandlers.end());
}

void CPeripheralJoystick::OnButtonMotion(unsigned int buttonIndex, bool bPressed)
{
  for (std::vector<IJoystickDriverHandler*>::iterator it = m_driverHandlers.begin(); it != m_driverHandlers.end(); ++it)
    (*it)->OnButtonMotion(buttonIndex, bPressed);
}

void CPeripheralJoystick::OnHatMotion(unsigned int hatIndex, HatDirection direction)
{
  for (std::vector<IJoystickDriverHandler*>::iterator it = m_driverHandlers.begin(); it != m_driverHandlers.end(); ++it)
    (*it)->OnHatMotion(hatIndex, direction);
}

void CPeripheralJoystick::OnAxisMotion(unsigned int axisIndex, float position)
{
  for (std::vector<IJoystickDriverHandler*>::iterator it = m_driverHandlers.begin(); it != m_driverHandlers.end(); ++it)
    (*it)->OnAxisMotion(axisIndex, position);
}

void CPeripheralJoystick::ProcessAxisMotions(void)
{
  for (std::vector<IJoystickDriverHandler*>::iterator it = m_driverHandlers.begin(); it != m_driverHandlers.end(); ++it)
    (*it)->ProcessAxisMotions();
}
