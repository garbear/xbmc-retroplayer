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
#include <string>
#include <vector>

class IJoystickInputHandler;

namespace PERIPHERALS { class CPeripheral; }

struct SPort
{
  IJoystickInputHandler*                 handler;
  std::vector<PERIPHERALS::CPeripheral*> devices;
};

class CPortManager : public Observer
{
private:
  CPortManager(void);

public:
  static CPortManager& Get(void);

  virtual ~CPortManager(void);

  void OpenPort(IJoystickInputHandler* handler);
  void ClosePort(IJoystickInputHandler* handler);

  virtual void Notify(const Observable &obs, const ObservableMessage msg);

private:
  typedef std::map<PERIPHERALS::CPeripheral*, IJoystickInputHandler*> DeviceMap;

  void ClearDevices(void);

  void ProcessDevices(void);
  std::vector<PERIPHERALS::CPeripheral*> ScanPeripherals(void) const;
  void AssignDevices(const std::vector<PERIPHERALS::CPeripheral*>& devices);
  void ProcessHandlers(const DeviceMap& oldDeviceMap) const;

  // Utility functions
  unsigned int GetNextOpenPort(unsigned int startPort = 0) const;
  DeviceMap GetDeviceMap(void) const;
  unsigned int GetMinDeviceDepth(void) const;

  std::vector<SPort> m_ports;
  CCriticalSection   m_mutex;
};
