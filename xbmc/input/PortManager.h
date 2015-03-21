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

#include <map>
#include <string>
#include <vector>

class IJoystickInputHandler;

namespace PERIPHERALS { class CPeripheral; }

struct SPort
{
  IJoystickInputHandler*                 handler;
  std::vector<PERIPHERALS::CPeripheral*> devices;
  std::string                            controllerId;
};

class CPortManager
{
private:
  CPortManager(void);

public:
  static CPortManager& Get(void);

  ~CPortManager(void) { ClearDevices(); }

  void OpenPort(IJoystickInputHandler* handler, const std::string& strDeviceId);
  void ClosePort(IJoystickInputHandler* handler);

private:
  void ClearDevices(void);

  std::vector<PERIPHERALS::CPeripheral*> ScanPeripherals(void) const;

  void ProcessDevices(void);
  void AssignDevices(const std::vector<PERIPHERALS::CPeripheral*>& devices);
  void AssignDevice(PERIPHERALS::CPeripheral* device, int requestedPort);
  void ProcessHandlers(std::map<PERIPHERALS::CPeripheral*, IJoystickInputHandler*>& oldDeviceMap) const;

  // Utility functions
  unsigned int GetNextOpenPort(unsigned int startPort = 0) const;
  std::map<PERIPHERALS::CPeripheral*, IJoystickInputHandler*> GetDeviceMap(void) const;
  unsigned int GetMinDeviceDepth(void) const;

  std::vector<SPort> m_ports;
  size_t             m_minDeviceDepth;
};
