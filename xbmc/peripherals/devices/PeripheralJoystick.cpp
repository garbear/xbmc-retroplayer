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
#include "input/joysticks/generic/GenericJoystickDriverHandler.h"
#include "peripherals/Peripherals.h"
#include "peripherals/addons/AddonJoystickButtonMap.h"
#include "peripherals/bus/PeripheralBusAddon.h"
#include "utils/log.h"

#include <algorithm>

using namespace PERIPHERALS;

#ifndef SAFE_DELETE
#define SATE_DELETE(x)  do { delete (x); (x) = NULL; } while (0)
#endif

CPeripheralJoystick::CPeripheralJoystick(const PeripheralScanResult& scanResult) :
  CPeripheral(scanResult),
  m_requestedPort(JOYSTICK_PORT_UNKNOWN),
  m_buttonMap(NULL),
  m_driverHandler(NULL)
{
  m_features.push_back(FEATURE_JOYSTICK);
}

CPeripheralJoystick::~CPeripheralJoystick(void)
{
  delete m_driverHandler;
  delete m_buttonMap;
}

bool CPeripheralJoystick::InitialiseFeature(const PeripheralFeature feature)
{
  if (!CPeripheral::InitialiseFeature(feature))
    return false;

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
        {
          m_requestedPort = addon->GetRequestedPort(index);

          m_buttonMap = new CAddonJoystickButtonMap(addon, index);
          if (m_buttonMap->Load())
            m_driverHandler = new CGenericJoystickDriverHandler(this, m_buttonMap);
          else
            SAFE_DELETE(m_buttonMap);
        }
        else
          CLog::Log(LOGERROR, "CPeripheralJoystick: Invalid location (%s)", m_strLocation.c_str());
      }
    }
  }

  return m_driverHandler != NULL;
}

bool CPeripheralJoystick::OnButtonPress(JoystickFeatureID id, bool bPressed)
{
  bool bHandled = false;

  for (std::vector<IJoystickInputHandler*>::iterator it = m_handlers.begin(); it != m_handlers.end(); ++it)
    bHandled |= (*it)->OnButtonPress(id, bPressed);

  return bHandled || m_fallbackHandler.OnButtonPress(id, bPressed);
}

bool CPeripheralJoystick::OnButtonMotion(JoystickFeatureID id, float magnitude)
{
  bool bHandled = false;

  for (std::vector<IJoystickInputHandler*>::iterator it = m_handlers.begin(); it != m_handlers.end(); ++it)
    bHandled |= (*it)->OnButtonMotion(id, magnitude);

  return bHandled || m_fallbackHandler.OnButtonMotion(id, magnitude);
}

bool CPeripheralJoystick::OnAnalogStickMotion(JoystickFeatureID id, float x, float y)
{
  bool bHandled = false;

  for (std::vector<IJoystickInputHandler*>::iterator it = m_handlers.begin(); it != m_handlers.end(); ++it)
    bHandled |= (*it)->OnAnalogStickMotion(id, x, y);

  return bHandled || m_fallbackHandler.OnAnalogStickMotion(id, x, y);
}

bool CPeripheralJoystick::OnAccelerometerMotion(JoystickFeatureID id, float x, float y, float z)
{
  bool bHandled = false;

  for (std::vector<IJoystickInputHandler*>::iterator it = m_handlers.begin(); it != m_handlers.end(); ++it)
    bHandled |= (*it)->OnAccelerometerMotion(id, x, y, z);

  return bHandled || m_fallbackHandler.OnAccelerometerMotion(id, x, y, z);
}

void CPeripheralJoystick::RegisterInputHandler(IJoystickInputHandler* handler)
{
  if (std::find(m_handlers.begin(), m_handlers.end(), handler) == m_handlers.end())
    m_handlers.push_back(handler);
}

void CPeripheralJoystick::UnregisterInputHandler(IJoystickInputHandler* handler)
{
  std::vector<IJoystickInputHandler*>::iterator it = std::find(m_handlers.begin(), m_handlers.end(), handler);
  if (it != m_handlers.end())
    m_handlers.erase(it);
}
