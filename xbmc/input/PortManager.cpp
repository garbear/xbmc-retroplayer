/*
 *      Copyright (C) 2015 Team XBMC
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
 */

#include "PortManager.h"
#include "peripherals/devices/PeripheralJoystick.h"
#include "peripherals/Peripherals.h"
#include "threads/SingleLock.h"

#include <algorithm>

using namespace PERIPHERALS;

// --- InputHandlerEqual -------------------------------------------------------

struct InputHandlerEqual
{
  InputHandlerEqual(IJoystickInputHandler* handler) : m_handler(handler) { }

  bool operator()(const SPort& port) { return port.handler == m_handler; }

private:
  IJoystickInputHandler* const m_handler;
};

// --- CPortManager ------------------------------------------------------------

CPortManager::CPortManager(void)
{
  g_peripherals.RegisterObserver(this);
}

CPortManager& CPortManager::Get()
{
  static CPortManager instance;
  return instance;
}

CPortManager::~CPortManager(void)
{
  g_peripherals.UnregisterObserver(this);
}

void CPortManager::ClearDevices(void)
{
  for (std::vector<SPort>::iterator itPort = m_ports.begin(); itPort != m_ports.end(); ++itPort)
    itPort->devices.clear();
}

void CPortManager::OpenPort(IJoystickInputHandler* handler)
{
  CSingleLock lock(m_mutex);

  SPort port;

  port.handler = handler;

  m_ports.push_back(port);

  ProcessDevices();
}

void CPortManager::ClosePort(IJoystickInputHandler* handler)
{
  CSingleLock lock(m_mutex);

  m_ports.erase(std::remove_if(m_ports.begin(), m_ports.end(), InputHandlerEqual(handler)), m_ports.end());

  ProcessDevices();
}

void CPortManager::Notify(const Observable &obs, const ObservableMessage msg)
{
  switch (msg)
  {
    case ObservableMessagePeripheralsChanged:
    {
      CSingleLock lock(m_mutex);

      ProcessDevices();

      break;
    }
    default:
      break;
  }
}

void CPortManager::ProcessDevices(void)
{
  // Record the old handler-device map for later processing
  DeviceMap oldDeviceMap = GetDeviceMap();

  // Clear the previous assignments
  ClearDevices();

  // Scan for peripherals
  std::vector<CPeripheral*> peripherals = ScanPeripherals();

  // Assign devices to ports
  AssignDevices(peripherals);

  // Notify devices whose ports have changed
  ProcessHandlers(oldDeviceMap);
}

std::vector<CPeripheral*> CPortManager::ScanPeripherals(void) const
{
  std::vector<CPeripheral*> peripherals;

  g_peripherals.GetPeripheralsWithFeature(peripherals, FEATURE_JOYSTICK);

  return peripherals;
}

void CPortManager::AssignDevices(const std::vector<CPeripheral*>& devices)
{
  for (std::vector<CPeripheral*>::const_iterator it = devices.begin(); it != devices.end(); ++it)
  {
    int requestedPort = JOYSTICK_PORT_UNKNOWN;
    if ((*it)->Type() == PERIPHERAL_JOYSTICK)
    {
      CPeripheralJoystick* joystick = static_cast<CPeripheralJoystick*>(*it);
      if (joystick->RequestedPort() != JOYSTICK_PORT_UNKNOWN && joystick->RequestedPort() <= (int)m_ports.size())
        requestedPort = joystick->RequestedPort() - 1;
    }

    const unsigned int targetPort = GetNextOpenPort(requestedPort);

    m_ports[targetPort].devices.push_back(*it);
  }
}

void CPortManager::ProcessHandlers(const DeviceMap& oldDeviceMap) const
{
  DeviceMap newDeviceMap = GetDeviceMap();

  for (DeviceMap::const_iterator itNew = newDeviceMap.begin(); itNew != newDeviceMap.end(); ++itNew)
  {
    DeviceMap::const_iterator itOld = oldDeviceMap.find(itNew->first);

    IJoystickInputHandler* oldHandler = itOld != oldDeviceMap.end() ? itOld->second : NULL;
    IJoystickInputHandler* newHandler = itNew->second;

    if (oldHandler != newHandler) // Check if handler changed
    {
      CPeripheralJoystick* joystick = static_cast<CPeripheralJoystick*>(itNew->first);

      // Unregister old handler
      if (oldHandler != NULL)
        joystick->UnregisterInputHandler(oldHandler);

      // Register new handler
      joystick->RegisterInputHandler(newHandler);
    }
  }
}

unsigned int CPortManager::GetNextOpenPort(unsigned int startPort /* = 0 */) const
{
  unsigned int minDeviceDepth = GetMinDeviceDepth();

  for (unsigned int i = 0, port = startPort; i < m_ports.size(); i++, port = (port + 1) % m_ports.size())
  {
    if (m_ports[port].devices.size() == minDeviceDepth)
      return port;
  }

  return 0;
}

CPortManager::DeviceMap CPortManager::GetDeviceMap(void) const
{
  DeviceMap deviceMap;

  for (std::vector<SPort>::const_iterator itPort = m_ports.begin(); itPort != m_ports.end(); ++itPort)
  {
    for (std::vector<CPeripheral*>::const_iterator itDevice = itPort->devices.begin();
         itDevice != itPort->devices.end(); ++itDevice)
    {
      deviceMap[*itDevice] = itPort->handler;
    }
  }

  return deviceMap;
}

unsigned int CPortManager::GetMinDeviceDepth(void) const
{
  size_t minDeviceDepth = 0xffff;

  for (std::vector<SPort>::const_iterator itPort = m_ports.begin(); itPort != m_ports.end(); ++itPort)
    minDeviceDepth = std::min(minDeviceDepth, itPort->devices.size());

  return minDeviceDepth;
}
