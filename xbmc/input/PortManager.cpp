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
  HandlerEqual(IJoystickFeatureHandler* handler) : m_handler(handler) { }

  bool operator()(const SPort& port)
  {
    return port.handler == m_handler;
  }

private:
  IJoystickFeatureHandler* const m_handler;
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

void CPortManager::OpenPort(IJoystickFeatureHandler* handler, const std::string& strDeviceId)
{
  SPort port;

  port.handler      = handler;
  port.controllerId = strDeviceId;

  m_ports.push_back(port);

  AssignDevices();
}

void CPortManager::ClosePort(IJoystickFeatureHandler* handler)
{
  m_ports.erase(std::remove_if(m_ports.begin(), m_ports.end(), HandlerEqual(handler)), m_ports.end());

  AssignDevices();
}

size_t CPortManager::DevicesAttached(int portNumber) const
{
  unsigned int devicesAttached = 0;

  if (portNumber != JOYSTICK_PORT_UNKNOWN)
  {
    unsigned int portIndex = portNumber - 1;
    if (portIndex < m_ports.size())
      devicesAttached = m_ports[portIndex].devices.size();
  }

  return devicesAttached;
}

void CPortManager::AssignDevices(void)
{
  ClearDevices();

  std::vector<CPeripheral*> peripherals;
  g_peripherals.GetPeripheralsWithFeature(peripherals, FEATURE_JOYSTICK);

  peripherals.swap(m_devices); // TODO: Sort for greedy processing

  for (std::vector<CPeripheral*>::iterator it = m_devices.begin(); it != m_devices.end(); ++it)
  {
    int requestedPort = JOYSTICK_PORT_UNKNOWN;
    if ((*it)->Type() == PERIPHERAL_JOYSTICK)
    {
      CPeripheralJoystick* joystick = static_cast<CPeripheralJoystick*>(*it);
      if (joystick->RequestedPort() <= (int)m_ports.size())
        requestedPort = joystick->RequestedPort();
    }

    AddDevice(*it, requestedPort);
  }
}

void CPortManager::ClearDevices(void)
{
  for (std::vector<SPort>::iterator it = m_ports.begin(); it != m_ports.end(); ++it)
    it->devices.clear();
  m_deviceDepth = 0;
}

void CPortManager::AddDevice(CPeripheral *device, int requestedPort)
{
  const int          targetPort  = GetNextOpenPort(requestedPort);
  const unsigned int targetIndex = targetPort - 1;

  m_ports[targetIndex].devices.push_back(device);

  // Update max device count
  m_deviceDepth = std::max(DevicesAttached(targetPort), m_deviceDepth);
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
