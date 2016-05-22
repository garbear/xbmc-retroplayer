/*
 *      Copyright (C) 2014-2016 Team Kodi
 *      http://kodi.tv
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIControllerList.h"

#include <algorithm>
#include <assert.h>
#include <iterator>

#include "GUIControllerDefines.h"
#include "GUIControllerWindow.h"
#include "GUIFeatureList.h"
#include "addons/AddonManager.h"
#include "dialogs/GUIDialogYesNo.h"
#include "games/controllers/Controller.h"
#include "games/controllers/guicontrols/GUIControllerButton.h"
#include "games/controllers/guicontrols/GUIGameController.h"
#include "guilib/GUIButtonControl.h"
#include "guilib/GUIControlGroupList.h"
#include "guilib/GUIWindow.h"
#include "guilib/WindowIDs.h"
#include "input/joysticks/DefaultJoystick.h" // for DEFAULT_CONTROLLER_ID
#include "messaging/ApplicationMessenger.h"
#include "peripherals/Peripherals.h"

using namespace ADDON;
using namespace GAME;

CGUIControllerList::CGUIControllerList(CGUIWindow* window, IFeatureList* featureList) :
  m_guiWindow(window),
  m_featureList(featureList),
  m_controllerList(nullptr),
  m_controllerButton(nullptr),
  m_focusedController(-1) // Initially unfocused
{
  assert(m_featureList != nullptr);
}

bool CGUIControllerList::Initialize(void)
{
  m_controllerList = dynamic_cast<CGUIControlGroupList*>(m_guiWindow->GetControl(CONTROL_CONTROLLER_LIST));
  m_controllerButton = dynamic_cast<CGUIButtonControl*>(m_guiWindow->GetControl(CONTROL_CONTROLLER_BUTTON_TEMPLATE));

  if (m_controllerButton)
    m_controllerButton->SetVisible(false);

  Refresh();

  CAddonMgr::GetInstance().RegisterObserver(this);

  return m_controllerList != nullptr &&
         m_controllerButton != nullptr;
}

void CGUIControllerList::Deinitialize(void)
{
  CAddonMgr::GetInstance().UnregisterObserver(this);

  CleanupButtons();

  m_controllerList = nullptr;
  m_controllerButton = nullptr;
}

void CGUIControllerList::Refresh(void)
{
  CleanupButtons();

  if (!RefreshControllers())
    return;

  if (m_controllerList)
  {
    unsigned int buttonId = 0;
    for (ControllerVector::const_iterator it = m_controllers.begin(); it != m_controllers.end(); ++it)
    {
      const ControllerPtr& controller = *it;

      CGUIButtonControl* pButton = new CGUIControllerButton(*m_controllerButton, controller->Label(), buttonId++);
      m_controllerList->AddControl(pButton);

      // Just in case
      if (buttonId >= MAX_CONTROLLER_COUNT)
        break;
    }
  }
}

void CGUIControllerList::OnFocus(unsigned int controllerIndex)
{
  if (controllerIndex < m_controllers.size())
  {
    m_focusedController = controllerIndex;

    const ControllerPtr& controller = m_controllers[controllerIndex];
    m_featureList->Load(controller);

    // TODO: Activate controller for all game controller controls
    CGUIGameController* pController = dynamic_cast<CGUIGameController*>(m_guiWindow->GetControl(CONTROL_GAME_CONTROLLER));
    if (pController)
      pController->ActivateController(controller);
  }
}

void CGUIControllerList::OnSelect(unsigned int controllerIndex)
{
  m_featureList->OnSelect(0);
}

void CGUIControllerList::ResetController(void)
{
  if (0 <= m_focusedController && m_focusedController < (int)m_controllers.size())
  {
    const std::string strControllerId = m_controllers[m_focusedController]->ID();

    // TODO: Choose peripheral
    // For now, ask the user if they would like to reset all peripherals
    // "Reset controller profile"
    // "Would you like to reset this controller profile for all devices?"
    if (!CGUIDialogYesNo::ShowAndGetInput(35060, 35061))
      return;

    PERIPHERALS::g_peripherals.ResetButtonMaps(strControllerId);
  }
}

void CGUIControllerList::Notify(const Observable& obs, const ObservableMessage msg)
{
  using namespace KODI::MESSAGING;

  if (msg == ObservableMessageAddons)
  {
    CGUIMessage msg(GUI_MSG_REFRESH_LIST, WINDOW_INVALID, CONTROL_CONTROLLER_LIST);
    CApplicationMessenger::GetInstance().SendGUIMessage(msg);
  }
}

bool CGUIControllerList::RefreshControllers(void)
{
  auto GetControllerID = [](const ControllerPtr& addon) { return addon->ID(); };
  auto GetAddonID = [](const AddonPtr& addon) { return addon->ID(); };

  std::set<std::string> currentIds;
  std::set<std::string> newIds;

  std::set<std::string> added;
  std::set<std::string> removed;

  // Get current add-ons
  std::transform(m_controllers.begin(), m_controllers.end(), std::inserter(currentIds, currentIds.end()), GetControllerID);

  // Get new add-ons
  VECADDONS newAddons;
  CAddonMgr::GetInstance().GetAddons(newAddons, ADDON_GAME_CONTROLLER);
  std::transform(newAddons.begin(), newAddons.end(), std::inserter(newIds, newIds.end()), GetAddonID);

  // Get differences
  std::set_difference(newIds.begin(), newIds.end(), currentIds.begin(), currentIds.end(), std::inserter(added, added.end()));
  std::set_difference(currentIds.begin(), currentIds.end(), newIds.begin(), newIds.end(), std::inserter(removed, removed.end()));

  // Register new controllers
  for (const std::string& addonId : added)
  {
    auto GetAddon = [addonId](const AddonPtr& addon) { return addon->ID() == addonId; };

    VECADDONS::iterator it = std::find_if(newAddons.begin(), newAddons.end(), GetAddon);
    if (it != newAddons.end())
    {
      ControllerPtr newController = std::dynamic_pointer_cast<CController>(*it);
      if (newController && newController->LoadLayout())
        m_controllers.push_back(newController);
    }
  }

  // Erase removed controllers
  for (const std::string& addonId : removed)
  {
    auto IsController = [addonId](const ControllerPtr& controller) { return controller->ID() == addonId; };
    m_controllers.erase(std::remove_if(m_controllers.begin(), m_controllers.end(), IsController), m_controllers.end());
  }

  // Sort add-ons, with default controller first
  const bool bChanged = !added.empty() || !removed.empty();
  if (bChanged)
  {
    std::sort(m_controllers.begin(), m_controllers.end(),
      [](const ControllerPtr& i, const ControllerPtr& j)
      {
        if (i->ID() == DEFAULT_CONTROLLER_ID && j->ID() != DEFAULT_CONTROLLER_ID) return true;
        if (i->ID() != DEFAULT_CONTROLLER_ID && j->ID() == DEFAULT_CONTROLLER_ID) return false;

        return i->Name() < j->Name();
      });
  }

  return bChanged;
}

void CGUIControllerList::CleanupButtons(void)
{
  if (m_controllerList)
    m_controllerList->ClearAll();
}
