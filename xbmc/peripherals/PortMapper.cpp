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

#include "PortMapper.h"
#include "input/PortManager.h"
#include "peripherals/devices/Peripheral.h"
#include "peripherals/Peripherals.h"

using namespace PERIPHERALS;

CPortMapper::CPortMapper(void)
{
  CPortManager::Get().RegisterObserver(this);
}

CPortMapper::~CPortMapper(void)
{
  CPortManager::Get().UnregisterObserver(this);
}

void CPortMapper::Notify(const Observable &obs, const ObservableMessage msg)
{
  switch (msg)
  {
    case ObservableMessagePeripheralsChanged:
    case ObservableMessagePortsChanged:
      ProcessPeripherals();
      break;
    default:
      break;
  }
}

void CPortMapper::ProcessPeripherals(void)
{
  std::vector<CPeripheral*> devices;
  g_peripherals.GetPeripheralsWithFeature(devices, FEATURE_JOYSTICK);

  std::vector<CPeripheral*> keyboards;
  g_peripherals.GetPeripheralsWithFeature(keyboards, FEATURE_KEYBOARD);
  devices.insert(devices.end(), keyboards.begin(), keyboards.end());

  std::map<CPeripheral*, IJoystickInputHandler*> newPortMap;
  CPortManager::Get().MapDevices(devices, newPortMap);

  ProcessChanges(m_portMap, newPortMap);
  m_portMap.swap(newPortMap);
}

void CPortMapper::ProcessChanges(const std::map<CPeripheral*, IJoystickInputHandler*>& oldPortMap,
                                 const std::map<CPeripheral*, IJoystickInputHandler*>& newPortMap)
{
  for (std::map<CPeripheral*, IJoystickInputHandler*>::const_iterator itNew = newPortMap.begin(); itNew != newPortMap.end(); ++itNew)
  {
    std::map<CPeripheral*, IJoystickInputHandler*>::const_iterator itOld = oldPortMap.find(itNew->first);

    IJoystickInputHandler* oldHandler = itOld != oldPortMap.end() ? itOld->second : NULL;
    IJoystickInputHandler* newHandler = itNew->second;

    if (oldHandler != newHandler)
    {
      CPeripheral* device = itNew->first;

      // Unregister old handler
      if (oldHandler != NULL)
        device->UnregisterJoystickInputHandler(oldHandler);

      // Register new handler
      if (newHandler != NULL)
        device->RegisterJoystickInputHandler(newHandler);
    }
  }
}