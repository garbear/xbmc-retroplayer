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

using namespace PERIPHERALS;

CPortManager& CPortManager::Get(void)
{
  static CPortManager instance;
  return instance;
}

void CPortManager::OpenPort(IJoystickInputHandler* handler, unsigned int port)
{
  CSingleLock lock(m_mutex);

  SPort newPort = { handler, port, 0 };
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

void CPortManager::MapDevices(const std::vector<PERIPHERALS::CPeripheral*>& devices,
                              std::map<PERIPHERALS::CPeripheral*, IJoystickInputHandler*>& deviceToPortMap) const
{
  std::vector<SPort> ports;

  {
    CSingleLock lock(m_mutex);
    ports = m_ports; // Modify copy so device count starts from zero next time
  }

  if (!ports.empty())
  {
    for (std::vector<CPeripheral*>::const_iterator it = devices.begin(); it != devices.end(); ++it)
    {
      int requestedPort = JOYSTICK_PORT_UNKNOWN;
      if ((*it)->Type() == PERIPHERAL_JOYSTICK)
      {
        CPeripheralJoystick* joystick = static_cast<CPeripheralJoystick*>(*it);
        if (joystick->RequestedPort() != JOYSTICK_PORT_UNKNOWN && joystick->RequestedPort() <= (int)m_ports.size())
          requestedPort = joystick->RequestedPort();
      }

      unsigned int targetPort = GetNextOpenPort(ports, requestedPort);

      deviceToPortMap[*it] = ports[targetPort].handler;
      ports[targetPort].deviceCount++;
    }
  }
}

unsigned int CPortManager::GetNextOpenPort(const std::vector<SPort>& ports, int requestedPort)
{
  const unsigned int minDeviceCount = GetMinDeviceCount(ports);

  // Look for requested port first, if provided
  if (requestedPort != JOYSTICK_PORT_UNKNOWN)
  {
    for (unsigned int i = 0; i < ports.size(); i++)
    {
      if ((int)ports[i].port == requestedPort && ports[i].deviceCount == minDeviceCount)
        return i;
    }
  }

  // Fall back to next open port round-robin style
  for (unsigned int i = 0; i < ports.size(); i++)
  {
    if (ports[i].deviceCount == minDeviceCount)
      return i;
  }

  return 0;
}

unsigned int CPortManager::GetMinDeviceCount(const std::vector<SPort>& ports)
{
  unsigned int minDeviceCount = 0;

  if (!ports.empty())
  {
    minDeviceCount = 9999; // large enough

    for (std::vector<SPort>::const_iterator itPort = ports.begin(); itPort != ports.end(); ++itPort)
      minDeviceCount = std::min(minDeviceCount, itPort->deviceCount);
  }

  return minDeviceCount;
}
