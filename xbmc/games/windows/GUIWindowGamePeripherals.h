/*
 *      Copyright (C) 2014 Team XBMC
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

#include "addons/Addon.h"
#include "games/GameTypes.h"
#include "guilib/GUIWindow.h"
#include "input/joysticks/IJoystickDriverHandler.h"
#include "FileItem.h"

#include <map>

namespace PERIPHERALS
{
  class CPeripheral;
}

class CGUIWindowGamePeripherals;

class CGUIJoystickDriverHandler : public IJoystickDriverHandler
{
public:
  CGUIJoystickDriverHandler(CGUIWindowGamePeripherals* window, PERIPHERALS::CPeripheral* device);

  virtual ~CGUIJoystickDriverHandler(void);

  // Implementation of IJoystickDriverHandler
  virtual void OnButtonMotion(unsigned int buttonIndex, bool bPressed);
  virtual void OnHatMotion(unsigned int hatIndex, HatDirection direction);
  virtual void OnAxisMotion(unsigned int axisIndex, float position);
  virtual void ProcessAxisMotions(void);

private:
  CGUIWindowGamePeripherals* const m_window;
  PERIPHERALS::CPeripheral* const  m_device;
};

class CGUIWindowGamePeripherals : public CGUIWindow
{
public:
  CGUIWindowGamePeripherals(void);
  virtual ~CGUIWindowGamePeripherals(void) { }

  // implementation of CGUIControl
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction& action);

  // implementation of CGUIWindow
  virtual void OnDeinitWindow(int nextWindowID);

  void OnButtonMotion(PERIPHERALS::CPeripheral* device, unsigned int buttonIndex, bool bPressed);
  void OnHatMotion(PERIPHERALS::CPeripheral* device, unsigned int hatIndex, HatDirection direction);
  void OnAxisMotion(PERIPHERALS::CPeripheral* device, unsigned int axisIndex, float position);
  void ProcessAxisMotions(PERIPHERALS::CPeripheral* device);

protected:
  GAME::GamePeripheralPtr GetPeripheral(const ADDON::AddonPtr& addon) const;
  GAME::GamePeripheralPtr LoadPeripheral(const ADDON::AddonPtr& addon);

  // implementation of CGUIWindow
  virtual void OnInitWindow(void);

private:
  std::vector<PERIPHERALS::CPeripheral*> ScanPeripherals(void);

  bool OnClick(int iItem);
  bool OnSelect(int iItem);

  int GetSelectedItem(int iControl);

  std::vector<CGUIJoystickDriverHandler*> m_driverHandlers;
  GAME::GamePeripheralVector m_peripherals;
  CFileItemList              m_items;
  int                        m_selectedItem;
};
