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
#include "threads/SingleLock.h"

#include <algorithm>

using namespace JOYSTICK;
using namespace PERIPHERALS;

CPortManager& CPortManager::Get(void)
{
  static CPortManager instance;
  return instance;
}

void CPortManager::OpenPort(IJoystickInputHandler* handler, unsigned int port,
                            PERIPHERALS::PeripheralType device /* = PERIPHERALS::PERIPHERAL_UNKNOWN) */)
{
  CSingleLock lock(m_mutex);

  SPort newPort;
  newPort.handler = handler;
  newPort.port = port;
  newPort.type = device;
  m_ports.push_back(newPort);

  SetChanged();
  NotifyObservers(ObservableMessagePortsChanged);
}

void CPortManager::ClosePort(IJoystickInputHandler* handler)
{
  CSingleLock lock(m_mutex);

  m_ports.erase(std::remove_if(m_ports.begin(), m_ports.end(), PortInputHandlerEqual(handler)), m_ports.end());

  SetChanged();
  NotifyObservers(ObservableMessagePortsChanged);
}

void CPortManager::MapDevices(const std::vector<CPeripheral*>& devices,
                              std::map<CPeripheral*, IJoystickInputHandler*>& deviceToPortMap)
{
  CSingleLock lock(m_mutex);

  if (m_ports.empty())
    return; // Nothing to do

  // Force the keyboard to disconnect so that joysticks get priority
  for (std::vector<CPeripheral*>::const_iterator itDevice = devices.begin(); itDevice != devices.end(); ++itDevice)
  {
    if ((*itDevice)->Type() == PERIPHERAL_KEYBOARD)
    {
      const CPeripheral* keyboard = *itDevice;
      for (std::vector<SPort>::iterator itPort = m_ports.begin(); itPort != m_ports.end(); ++itPort)
      {
        std::vector<SDevice>& portDevices = itPort->devices;
        portDevices.erase(std::remove_if(portDevices.begin(), portDevices.end(),
          [keyboard](const SDevice& portDevice)
          {
            return portDevice.device == keyboard;
          }), portDevices.end());
      }
      break;
    }
  }

  // Look for connected/removed devices
  for (std::vector<SPort>::iterator itPort = m_ports.begin(); itPort != m_ports.end(); ++itPort)
  {
    for (std::vector<SDevice>::iterator itPortDevice = itPort->devices.begin(); itPortDevice != itPort->devices.end(); ++itPortDevice)
    {
      if (std::find(devices.begin(), devices.end(), itPortDevice->device) != devices.end())
      {
        itPortDevice->bConnected = true;
        deviceToPortMap[static_cast<CPeripheral*>(itPortDevice->device)] = itPort->handler;
      }
      else
      {
        itPortDevice->bConnected = false;
      }
    }
  }

  // Look for new devices
  std::vector<PERIPHERALS::CPeripheral*> newDevices;
  for (std::vector<CPeripheral*>::const_iterator itDevice = devices.begin(); itDevice != devices.end(); ++itDevice)
  {
    const CPeripheral* device = *itDevice;

    bool bFound = false;

    for (std::vector<SPort>::const_iterator itPort = m_ports.begin(); itPort != m_ports.end(); ++itPort)
    {
      bFound = std::count_if(itPort->devices.begin(), itPort->devices.end(),
        [device](const SDevice& portDevice)
        {
          return portDevice.device == device;
        }) > 0;

      if (bFound)
        break;
    }

    if (!bFound)
      newDevices.push_back(*itDevice);
  }

  // Add new devices to disconnected ports
  for (std::vector<CPeripheral*>::const_iterator itDevice = newDevices.begin(); itDevice != newDevices.end(); ++itDevice)
  {
    int targetPort = GetTargetPort(GetRequestedPort(*itDevice), (*itDevice)->Type());

    if (targetPort >= 0)
    {
      SPort& port = m_ports[targetPort];

      // Clear the port of all disconnected devices before adding this device
      // except for keyboard (we expect the keyboard to hop around)
      if ((*itDevice)->Type() != PERIPHERAL_KEYBOARD)
      {
        port.devices.erase(std::remove_if(port.devices.begin(), port.devices.end(),
          [](const SDevice& portDevice)
            {
              return !portDevice.bConnected; 
            }), port.devices.end());
      }

      SDevice portDevice = { *itDevice, true };
      port.devices.push_back(portDevice);
      deviceToPortMap[*itDevice] = m_ports[targetPort].handler;
    }
  }
}

int CPortManager::GetTargetPort(int requestedPort, PeripheralType deviceType) const
{
  for (unsigned int i = 0; i < m_ports.size(); i++)
  {
    // Check port number if port number was requested
    const bool bPortRequested = (requestedPort != JOYSTICK_PORT_UNKNOWN);
    if (bPortRequested && requestedPort != (int)m_ports[i].port)
      continue;

    // Check device type if port requires a specific device
    const bool bRequiresDeviceType = (m_ports[i].type != PERIPHERAL_UNKNOWN);
    if (bRequiresDeviceType && m_ports[i].type != deviceType)
      continue;

    unsigned int connectedDeviceCount = std::count_if(m_ports[i].devices.begin(), m_ports[i].devices.end(),
      [](const SDevice& portDevice)
      {
        return portDevice.bConnected;
      });

    if (connectedDeviceCount > 0)
      continue;

    return i;
  }

  return -1;
}

int CPortManager::GetRequestedPort(const CPeripheral* device)
{
  int requestedPort = JOYSTICK_PORT_UNKNOWN;

  if (device->Type() == PERIPHERAL_JOYSTICK)
  {
    const CPeripheralJoystick* joystick = static_cast<const CPeripheralJoystick*>(device);
    if (joystick->RequestedPort() != JOYSTICK_PORT_UNKNOWN)
      requestedPort = joystick->RequestedPort();
  }

  return requestedPort;
}
