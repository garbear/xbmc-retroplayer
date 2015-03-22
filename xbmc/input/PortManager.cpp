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
}

CPortManager& CPortManager::Get()
{
  static CPortManager instance;
  return instance;
}

void CPortManager::ClearDevices(void)
{
  for (std::vector<SPort>::iterator itPort = m_ports.begin(); itPort != m_ports.end(); ++itPort)
    itPort->devices.clear();
}

void CPortManager::OpenPort(IJoystickInputHandler* handler)
{
  SPort port;

  port.handler = handler;

  m_ports.push_back(port);

  ProcessDevices();
}

void CPortManager::ClosePort(IJoystickInputHandler* handler)
{
  m_ports.erase(std::remove_if(m_ports.begin(), m_ports.end(), InputHandlerEqual(handler)), m_ports.end());

  ProcessDevices();
}

void CPortManager::ProcessDevices(void)
{
  // Record the old handler-device map for later processing
  std::map<CPeripheral*, IJoystickInputHandler*> oldDeviceMap = GetDeviceMap();

  // Clear the previous assignments
  ClearDevices();

  // Scan for peripherals
  std::vector<CPeripheral*> peripherals = ScanPeripherals();

  // Assign devices
  AssignDevices(peripherals);

  // Notify devices whose ports have changed (TODO: refactor this)
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
      if (joystick->RequestedPort() <= (int)m_ports.size())
        requestedPort = joystick->RequestedPort();
    }

    AssignDevice(*it, requestedPort);
  }
}

void CPortManager::ProcessHandlers(std::map<CPeripheral*, IJoystickInputHandler*>& oldDeviceMap) const
{
  std::map<CPeripheral*, IJoystickInputHandler*> newDeviceMap = GetDeviceMap();

  for (std::map<CPeripheral*, IJoystickInputHandler*>::const_iterator itNew = newDeviceMap.begin();
       itNew != newDeviceMap.end(); ++itNew)
  {
    IJoystickInputHandler* oldHandler = oldDeviceMap[itNew->first];
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

    // Remove this device from oldDeviceMap to indicate we've seen it
    if (oldHandler != NULL)
      oldDeviceMap.erase(oldDeviceMap.find(itNew->first));
  }

  // Devices left in oldDeviceMap have no input handlers
  for (std::map<CPeripheral*, IJoystickInputHandler*>::const_iterator itOld = oldDeviceMap.begin();
       itOld != newDeviceMap.end(); ++itOld)
  {
    static_cast<CPeripheralJoystick*>(itOld->first)->UnregisterInputHandler(itOld->second);
  }
}

void CPortManager::AssignDevice(CPeripheral* device, int requestedPort)
{
  const unsigned int index = GetNextOpenPort(requestedPort - 1);
  SPort&             port  = m_ports[index];

  port.devices.push_back(device);
}

unsigned int CPortManager::GetNextOpenPort(unsigned int startPort /* = 0 */) const
{
  unsigned int minDeviceDepth = GetMinDeviceDepth();

  for (unsigned int i = 0, port = startPort; i < m_ports.size(); i++, port = (port + 1) % m_ports.size())
  {
    if (m_ports[port].devices.size() == minDeviceDepth)
      return port;
  }

  return startPort;
}

std::map<CPeripheral*, IJoystickInputHandler*> CPortManager::GetDeviceMap(void) const
{
  std::map<CPeripheral*, IJoystickInputHandler*> deviceMap;

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
