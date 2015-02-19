/*
 *      Copyright (C) 2012-2014 Team XBMC
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

#include "GUIWindowGamePeripherals.h"
#include "GUIDialogControllerInput.h"
#include "addons/AddonManager.h"
#include "games/addons/GamePeripheral.h"
#include "guilib/GUIGamePeripheral.h"
#include "guilib/GUIFocusPlane.h"
#include "guilib/GUIMessage.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/WindowIDs.h"
#include "input/joysticks/JoystickTypes.h"
#include "input/Key.h"
#include "peripherals/devices/PeripheralJoystick.h"
#include "peripherals/Peripherals.h"
#include "utils/log.h"

#include <assert.h>
#include <set>
#include <string>

using namespace ADDON;
using namespace GAME;
using namespace PERIPHERALS;

#define CONTROL_PERIPHERAL_LIST     1
#define CONTROL_FOCUS_PLANE         2
#define CONTROL_GAME_PERIPHERAL     3

// --- CGUIJoystickDriverHandler -----------------------------------------------

CGUIJoystickDriverHandler::CGUIJoystickDriverHandler(CGUIWindowGamePeripherals* window, CPeripheral* device)
  : m_window(window), m_device(device)
{
  assert(m_window);
  assert(m_device);

  m_device->RegisterJoystickDriverHandler(this);
}

CGUIJoystickDriverHandler::~CGUIJoystickDriverHandler(void)
{
  m_device->UnregisterJoystickDriverHandler(this);
}

void CGUIJoystickDriverHandler::OnButtonMotion(unsigned int buttonIndex, bool bPressed)
{
  m_window->OnButtonMotion(m_device, buttonIndex, bPressed);
}

void CGUIJoystickDriverHandler::OnHatMotion(unsigned int hatIndex, HatDirection direction)
{
  m_window->OnHatMotion(m_device, hatIndex, direction);
}

void CGUIJoystickDriverHandler::OnAxisMotion(unsigned int axisIndex, float position)
{
  m_window->OnAxisMotion(m_device, axisIndex, position);
}

void CGUIJoystickDriverHandler::ProcessAxisMotions(void)
{
  m_window->ProcessAxisMotions(m_device);
}

// --- CGUIWindowGamePeripherals -----------------------------------------------

CGUIWindowGamePeripherals::CGUIWindowGamePeripherals(void) :
  CGUIWindow(WINDOW_GAME_PERIPHERALS, "GamePeripherals.xml"),
  m_selectedItem(-1)
{
}

bool CGUIWindowGamePeripherals::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();

      if (iControl == CONTROL_PERIPHERAL_LIST)
      {
        int iItem = GetSelectedItem();
        return OnClick(iItem);
      }
      break;
    }
  }

  const bool bHandled = CGUIWindow::OnMessage(message);

  OnSelect(GetSelectedItem());

  return bHandled;
}

bool CGUIWindowGamePeripherals::OnAction(const CAction& action)
{
  if (CGUIWindow::OnAction(action))
  {
    OnSelect(GetSelectedItem());
    return true;
  }
  return false;
}

void CGUIWindowGamePeripherals::OnInitWindow()
{
  CGUIWindow::OnInitWindow();

  std::vector<CPeripheral*> devices = ScanPeripherals();
  for (std::vector<CPeripheral*>::iterator it = devices.begin(); it != devices.end(); ++it)
    m_driverHandlers.push_back(new CGUIJoystickDriverHandler(this, *it));

  m_items.Clear();

  VECADDONS peripherals;
  CAddonMgr::Get().GetAddons(ADDON_GAME_PERIPHERAL, peripherals);
  for (VECADDONS::const_iterator it = peripherals.begin(); it != peripherals.end(); ++it)
  {
    if (!LoadPeripheral(*it))
      CLog::Log(LOGERROR, "Failed to load peripheral %s", (*it)->ID().c_str());
  }

  for (GamePeripheralVector::iterator it = m_peripherals.begin(); it != m_peripherals.end(); ++it)
  {
    CFileItemPtr item(new CFileItem((*it)->Label()));
    m_items.Add(item);
  }

  CGUIMessage msg(GUI_MSG_LABEL_BIND, GetID(), CONTROL_PERIPHERAL_LIST, 0, 0, &m_items);
  g_windowManager.SendMessage(msg);
}

void CGUIWindowGamePeripherals::OnDeinitWindow(int nextWindowID)
{
  for (std::vector<CGUIJoystickDriverHandler*>::iterator it = m_driverHandlers.begin(); it != m_driverHandlers.end(); ++it)
    delete *it;
  m_driverHandlers.clear();

  CGUIWindow::OnDeinitWindow(nextWindowID);
}

void CGUIWindowGamePeripherals::OnButtonMotion(PERIPHERALS::CPeripheral* device, unsigned int buttonIndex, bool bPressed)
{
  // TODO
}

void CGUIWindowGamePeripherals::OnHatMotion(PERIPHERALS::CPeripheral* device, unsigned int hatIndex, HatDirection direction)
{
  // TODO
}

void CGUIWindowGamePeripherals::OnAxisMotion(PERIPHERALS::CPeripheral* device, unsigned int axisIndex, float position)
{
  // TODO
}

void CGUIWindowGamePeripherals::ProcessAxisMotions(PERIPHERALS::CPeripheral* device)
{
  // TODO
}

GamePeripheralPtr CGUIWindowGamePeripherals::GetPeripheral(const AddonPtr& addon) const
{
  for (GamePeripheralVector::const_iterator it = m_peripherals.begin(); it != m_peripherals.end(); ++it)
  {
    if ((*it)->Addon()->ID() == addon->ID())
      return *it;
  }
  return CGamePeripheral::EmptyPtr;
}

GamePeripheralPtr CGUIWindowGamePeripherals::LoadPeripheral(const AddonPtr& addon)
{
  const GamePeripheralPtr& peripheral = GetPeripheral(addon);
  if (!peripheral)
  {
    CGUIControl* control = GetControl(CONTROL_GAME_PERIPHERAL);
    GamePeripheralPtr newPeripheral = GamePeripheralPtr(new CGamePeripheral(addon, control));
    if (newPeripheral->Load())
    {
      m_peripherals.push_back(newPeripheral);
      return newPeripheral;
    }
  }
  return peripheral;
}

std::vector<CPeripheral*> CGUIWindowGamePeripherals::ScanPeripherals(void)
{
  std::vector<CPeripheral*> peripherals;

  g_peripherals.GetPeripheralsWithFeature(peripherals, FEATURE_JOYSTICK);
  g_peripherals.GetPeripheralsWithFeature(peripherals, FEATURE_KEYBOARD);

  return peripherals;
}

bool CGUIWindowGamePeripherals::OnClick(int iItem)
{
  if (0 <= iItem && iItem < (int)m_peripherals.size())
  {
    CGUIDialogControllerInput* pMenu = dynamic_cast<CGUIDialogControllerInput*>(g_windowManager.GetWindow(WINDOW_DIALOG_CONTROLLER_INPUT));
    if (pMenu)
      pMenu->DoModal(m_peripherals[iItem], dynamic_cast<CGUIFocusPlane*>(GetControl(CONTROL_FOCUS_PLANE)));
    return true;
  }
  return false;
}

bool CGUIWindowGamePeripherals::OnSelect(int iItem)
{
  if (0 <= iItem && iItem < (int)m_peripherals.size() && iItem != m_selectedItem)
  {
    m_selectedItem = iItem;

    CGUIGamePeripheral* pPeripheral = dynamic_cast<CGUIGamePeripheral*>(GetControl(CONTROL_GAME_PERIPHERAL));
    if (pPeripheral)
    {
      pPeripheral->ActivatePeripheral(m_peripherals[iItem]);
      return true;
    }
  }
  return false;
}

int CGUIWindowGamePeripherals::GetSelectedItem(void)
{
  if (!m_items.IsEmpty())
  {
    CGUIMessage msg(GUI_MSG_ITEM_SELECTED, GetID(), CONTROL_PERIPHERAL_LIST);
    if (CGUIWindow::OnMessage(msg))
      return (int)msg.GetParam1();
  }
  return -1;
}
