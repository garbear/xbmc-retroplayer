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

// --- HandlerEqual ------------------------------------------------------------

struct HandlerEqual
{
  HandlerEqual(IJoystickInputHandler* handler) : m_handler(handler) { }

  bool operator()(const SPort& port)
  {
    return port.handler == m_handler;
  }

private:
  IJoystickInputHandler* const m_handler;
};

// --- CPortManager ------------------------------------------------------------

CPortManager::CPortManager(void)
  : m_deviceDepth(0)
{
}

CPortManager& CPortManager::Get()
{
  static CPortManager instance;
  return instance;
}

void CPortManager::OpenPort(IJoystickInputHandler* handler, const std::string& strDeviceId)
{
  SPort port;

  port.handler = handler;
  port.controllerId = strDeviceId;

  m_ports.push_back(port);

  ProcessDevices();
}

void CPortManager::ClosePort(IJoystickInputHandler* handler)
{
  m_ports.erase(std::remove_if(m_ports.begin(), m_ports.end(), HandlerEqual(handler)), m_ports.end());

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

void CPortManager::ClearDevices(void)
{
  for (std::vector<SPort>::iterator itPort = m_ports.begin(); itPort != m_ports.end(); ++itPort)
    itPort->devices.clear();

  m_deviceDepth = 0;
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

    if (oldHandler != newHandler)
    {
      CPeripheralJoystick* joystick = static_cast<CPeripheralJoystick*>(itNew->first);

      if (oldHandler != NULL)
        joystick->UnregisterInputHandler(oldHandler);
      joystick->RegisterInputHandler(newHandler);
    }

    if (oldHandler != NULL)
      oldDeviceMap.erase(oldDeviceMap.find(itNew->first));
  }

  for (std::map<CPeripheral*, IJoystickInputHandler*>::const_iterator itOld = oldDeviceMap.begin();
       itOld != newDeviceMap.end(); ++itOld)
  {
    CPeripheralJoystick* joystick = static_cast<CPeripheralJoystick*>(itOld->first);
    joystick->UnregisterInputHandler(itOld->second);
  }
}

void CPortManager::AssignDevice(CPeripheral* device, int requestedPort)
{
  const int          targetPort  = GetNextOpenPort(requestedPort);
  const unsigned int targetIndex = targetPort - 1;

  m_ports[targetIndex].devices.push_back(device);

  // Update max device count
  m_deviceDepth = std::max(DevicesAttached(targetPort), m_deviceDepth);
}

size_t CPortManager::DevicesAttached(int portNumber) const
{
  unsigned int devicesAttached = 0;

  if (portNumber != JOYSTICK_PORT_UNKNOWN)
  {
    const unsigned int portIndex = portNumber - 1;
    if (portIndex < m_ports.size())
      devicesAttached = m_ports[portIndex].devices.size();
  }

  return devicesAttached;
}

int CPortManager::GetNextOpenPort(int startPort /* = 1 */) const
{
  if (startPort == JOYSTICK_PORT_UNKNOWN)
    startPort = 1;

  for (std::vector<SPort>::const_iterator it = m_ports.begin(); it != m_ports.end(); ++it)
  {
    if (DevicesAttached(startPort) < m_deviceDepth)
      break; // Found an open slot

    startPort++;

    if (startPort > (int)m_ports.size())
      startPort = 1;
  }

  return startPort;
}

IJoystickInputHandler* CPortManager::GetInputHandler(CPeripheral* device) const
{
  for (std::vector<SPort>::const_iterator itPort = m_ports.begin(); itPort != m_ports.end(); ++itPort)
  {
    std::vector<CPeripheral*>::const_iterator itDevice = std::find(itPort->devices.begin(),
                                                                   itPort->devices.end(),
                                                                   device);
    if (itDevice != itPort->devices.end())
      return itPort->handler;
  }

  return NULL;
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
