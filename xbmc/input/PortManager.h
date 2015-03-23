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
 *
 */
#pragma once

#include "threads/CriticalSection.h"
#include "utils/Observer.h"

#include <map>
#include <vector>

class IJoystickInputHandler;

namespace PERIPHERALS { class CPeripheral; }

/*!
 * \ingroup input
 *
 * \brief Class to manage ports opened by game clients
 */
class CPortManager : public Observable
{
private:
  CPortManager(void) { }

public:
  static CPortManager& Get(void);

  /*!
   * \brief Request a new port be opened with input on that port sent to the
   *        specified handler.
   *
   * \param handler  The instance accepting all input delivered to the port
   */
  void OpenPort(IJoystickInputHandler* handler);

  /*!
   * \brief Close an opened port
   *
   * \param handler  The handler used to open the port
   */
  void ClosePort(IJoystickInputHandler* handler);

  /*!
   * \brief Map a list of devices to the available ports
   *
   * \param devices  The devices capable of providing input to the ports
   * \param portMap  The resulting map of devices to ports
   *
   * If there are more devices than open ports, multiple devices may be assigned
   * to the same port. If a device requests a specific port, this function will
   * attempt to honor that request.
   */
  void MapDevices(const std::vector<PERIPHERALS::CPeripheral*>& devices,
                  std::map<PERIPHERALS::CPeripheral*, IJoystickInputHandler*>& deviceToPortMap) const;

private:
  struct SPort
  {
    IJoystickInputHandler* handler;     // Input handler for this port
    unsigned int           deviceCount; // Number of devices attached to this port
  };

  /*!
   * \brief Functor to match ports against a specified input handler
   */
  struct PortInputHandlerEqual
  {
    PortInputHandlerEqual(IJoystickInputHandler* handler) : handler(handler) { }

    bool operator()(const SPort& port) { return port.handler == handler; }

    IJoystickInputHandler* const handler;
  };

  /*!
   * \brief Helper function to find the next open port, round-robin fashion,
   *        starting from the given port.
   *
   * A port is considered open if no other ports have a fewer number of devices
   * connected.
   */
  static unsigned int GetNextOpenPort(const std::vector<SPort>& ports, unsigned int startPort = 0);

  /*!
   * \brief Helper function to return the number of devices attached to the
   *        port with the fewest devices.
   */
  static unsigned int GetMinDeviceCount(const std::vector<SPort>& ports);

  std::vector<SPort> m_ports;
  CCriticalSection   m_mutex;
};
