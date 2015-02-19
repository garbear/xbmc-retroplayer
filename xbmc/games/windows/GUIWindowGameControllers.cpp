/*
 *      Copyright (C) 2014-2015 Team XBMC
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

#include "GUIWindowGameControllers.h"
#include "GUIDialogControllerInput.h"
#include "addons/AddonManager.h"
#include "dialogs/GUIDialogOK.h"
#include "games/addons/GameController.h"
#include "guilib/GUIGameController.h"
#include "guilib/GUIFocusPlane.h"
#include "guilib/GUIMessage.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/WindowIDs.h"
#include "input/joysticks/JoystickTypes.h"
#include "input/Key.h"
#include "peripherals/bus/PeripheralBusAddon.h"
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
#define CONTROL_GAME_CONTROLLER     3

// --- CGUIJoystickInputHandler ------------------------------------------------

CGUIJoystickInputHandler::CGUIJoystickInputHandler(CGUIWindowGameControllers* window, CPeripheral* device, const std::string& strDeviceId)
  : m_window(window),
    m_device(device),
    m_strDeviceId(strDeviceId)
{
  assert(m_window);
  assert(m_device);

  m_device->RegisterJoystickInputHandler(this);
}

CGUIJoystickInputHandler::~CGUIJoystickInputHandler(void)
{
  m_device->UnregisterJoystickInputHandler(this); // TODO: segfaults if device is deleted
}

bool CGUIJoystickInputHandler::OnButtonPress(unsigned int featureIndex, bool bPressed)
{
  return m_window->OnButtonPress(m_device, featureIndex, bPressed);
}

bool CGUIJoystickInputHandler::OnButtonMotion(unsigned int featureIndex, float magnitude)
{
  return m_window->OnButtonMotion(m_device, featureIndex, magnitude);
}

bool CGUIJoystickInputHandler::OnAnalogStickMotion(unsigned int featureIndex, float x, float y)
{
  return m_window->OnAnalogStickMotion(m_device, featureIndex, x, y);
}

bool CGUIJoystickInputHandler::OnAccelerometerMotion(unsigned int featureIndex, float x, float y, float z)
{
  return m_window->OnAccelerometerMotion(m_device, featureIndex, x, y, z);
}

// --- CGUIWindowGameControllers -----------------------------------------------

CGUIWindowGameControllers::CGUIWindowGameControllers(void) :
  CGUIWindow(WINDOW_GAME_CONTROLLERS, "GameControllers.xml"),
  m_selectedItem(-1)
{
}

bool CGUIWindowGameControllers::OnMessage(CGUIMessage& message)
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

bool CGUIWindowGameControllers::OnAction(const CAction& action)
{
  if (CGUIWindow::OnAction(action))
  {
    OnSelect(GetSelectedItem());
    return true;
  }
  return false;
}

void CGUIWindowGameControllers::OnInitWindow()
{
  CGUIWindow::OnInitWindow();

  /* TODO
  std::vector<CPeripheral*> devices = ScanPeripherals();
  for (std::vector<CPeripheral*>::iterator it = devices.begin(); it != devices.end(); ++it)
    m_inputHandlers.push_back(new CGUIJoystickInputHandler(this, *it, "game.controller.default")); // TODO
  */

  m_items.Clear();

  VECADDONS controllers;
  CAddonMgr::Get().GetAddons(ADDON_GAME_CONTROLLER, controllers);
  for (VECADDONS::const_iterator it = controllers.begin(); it != controllers.end(); ++it)
  {
    if (!LoadController(std::dynamic_pointer_cast<CGameController>(*it)))
      CLog::Log(LOGERROR, "Failed to load controller %s", (*it)->ID().c_str());
  }

  for (GameControllerVector::iterator it = m_controllers.begin(); it != m_controllers.end(); ++it)
  {
    CFileItemPtr item(new CFileItem((*it)->Label()));
    m_items.Add(item);
  }

  CGUIMessage msg(GUI_MSG_LABEL_BIND, GetID(), CONTROL_PERIPHERAL_LIST, 0, 0, &m_items);
  g_windowManager.SendMessage(msg);

  // Check for button mapping support
  CPeripheralBus* bus = g_peripherals.GetBusByType(PERIPHERAL_BUS_ADDON);
  if (bus)
  {
    if (static_cast<CPeripheralBusAddon*>(bus)->GetAddonCount() == 0)
    {
      // TODO: Move the XML implementation of button map storage from add-on to
      // Kodi while keeping support for add-on button-mapping

      // "Joystick support not found"
      // "Controller configuration is disabled. Install the proper joystick support add-on."
      CGUIDialogOK::ShowAndGetInput(35011, 35012);
    }
  }
}

void CGUIWindowGameControllers::OnDeinitWindow(int nextWindowID)
{
  for (std::vector<CGUIJoystickInputHandler*>::iterator it = m_inputHandlers.begin(); it != m_inputHandlers.end(); ++it)
    delete *it;
  m_inputHandlers.clear();

  CGUIWindow::OnDeinitWindow(nextWindowID);
}

bool CGUIWindowGameControllers::OnButtonPress(PERIPHERALS::CPeripheral* device, unsigned int featureIndex, bool bPressed)
{
  return false; // TODO
}

bool CGUIWindowGameControllers::OnButtonMotion(PERIPHERALS::CPeripheral* device, unsigned int featureIndex, float magnitude)
{
  return false; // TODO
}

bool CGUIWindowGameControllers::OnAnalogStickMotion(PERIPHERALS::CPeripheral* device, unsigned int featureIndex, float x, float y)
{
  return false; // TODO
}

bool CGUIWindowGameControllers::OnAccelerometerMotion(PERIPHERALS::CPeripheral* device, unsigned int featureIndex, float x, float y, float z)
{
  return false; // TODO
}

bool CGUIWindowGameControllers::LoadController(const GameControllerPtr& controller)
{
  if (controller)
  {
    bool bFound = false;

    for (GameControllerVector::const_iterator it = m_controllers.begin(); it != m_controllers.end(); ++it)
    {
      if ((*it)->ID() == controller->ID())
      {
        bFound = true;
        break;
      }
    }

    if (!bFound)
      m_controllers.push_back(controller);

    return true;
  }

  return false;
}

std::vector<CPeripheral*> CGUIWindowGameControllers::ScanPeripherals(void)
{
  std::vector<CPeripheral*> peripherals;

  g_peripherals.GetPeripheralsWithFeature(peripherals, FEATURE_JOYSTICK);
  g_peripherals.GetPeripheralsWithFeature(peripherals, FEATURE_KEYBOARD);

  return peripherals;
}

bool CGUIWindowGameControllers::OnClick(int iItem)
{
  if (0 <= iItem && iItem < (int)m_controllers.size())
  {
    CGUIDialogControllerInput* pMenu = dynamic_cast<CGUIDialogControllerInput*>(g_windowManager.GetWindow(WINDOW_DIALOG_CONTROLLER_INPUT));
    if (pMenu)
      pMenu->DoModal(m_controllers[iItem], dynamic_cast<CGUIFocusPlane*>(GetControl(CONTROL_FOCUS_PLANE)));
    return true;
  }
  return false;
}

bool CGUIWindowGameControllers::OnSelect(int iItem)
{
  if (0 <= iItem && iItem < (int)m_controllers.size() && iItem != m_selectedItem)
  {
    m_selectedItem = iItem;

    CGUIGameController* pController = dynamic_cast<CGUIGameController*>(GetControl(CONTROL_GAME_CONTROLLER));
    if (pController)
    {
      pController->ActivateController(m_controllers[iItem]);
      return true;
    }
  }
  return false;
}

int CGUIWindowGameControllers::GetSelectedItem(void)
{
  if (!m_items.IsEmpty())
  {
    CGUIMessage msg(GUI_MSG_ITEM_SELECTED, GetID(), CONTROL_PERIPHERAL_LIST);
    if (CGUIWindow::OnMessage(msg))
      return (int)msg.GetParam1();
  }
  return -1;
}
