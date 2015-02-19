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

#include "ControllerManager.h"
#include "addons/AddonManager.h"
#include "games/addons/GameController.h"
#include "utils/log.h"

using namespace ADDON;
using namespace GAME;

// --- AddonIdEqual ------------------------------------------------------------

namespace GAME
{
  struct AddonIdEqual
  {
    AddonIdEqual(const std::string& strId) : m_strId(strId) { }

    bool operator()(const AddonPtr& addon) const { return addon && addon->ID() == m_strId; }

  private:
    const std::string m_strId;
  };
}

// --- CControllerManager ------------------------------------------------------------

CControllerManager& CControllerManager::Get()
{
  static CControllerManager instance;
  return instance;
}

void CControllerManager::Start(void)
{
  CAddonMgr::Get().RegisterObserver(this);
  UpdateAddons();
}

void CControllerManager::Stop(void)
{
  CAddonMgr::Get().UnregisterObserver(this);
  m_controllers.clear();
}

bool CControllerManager::GetController(const std::string& strControllerId, GameControllerPtr& addon) const
{
  CSingleLock lock(m_critSection);

  ControllerMap::const_iterator it = m_controllers.find(strControllerId);
  if (it != m_controllers.end())
  {
    addon = it->second;
    return true;
  }

  return false;
}

void CControllerManager::UpdateAddons(void)
{
  VECADDONS controllers;
  if (CAddonMgr::Get().GetAddons(ADDON_GAME_CONTROLLER, controllers, true))
  {
    CSingleLock lock(m_critSection);

    for (VECADDONS::const_iterator it = controllers.begin(); it != controllers.end(); it++)
    {
      const GameControllerPtr& controller = std::dynamic_pointer_cast<CGameController>(*it);

      if (!controller)
        continue;

      if (m_controllers.find(controller->ID()) != m_controllers.end())
        continue; // Already registered

      if (!controller->LoadLayout())
      {
        CLog::Log(LOGERROR, "Failed to load controller %s", controller->ID().c_str());
        continue;
      }

      if (controller->Layout().FeatureCount() == 0)
      {
        CLog::Log(LOGERROR, "Controller %s: no features!", controller->ID().c_str());
        continue;
      }

      m_controllers[controller->ID()] = controller;
    }

    // Check if add-ons have been removed
    for (ControllerMap::const_iterator it = m_controllers.begin(); it != m_controllers.end(); ++it)
    {
      bool bFound = std::count_if(controllers.begin(), controllers.end(), AddonIdEqual(it->first)) > 0;

      if (!bFound)
      {
        m_controllers.erase(it);
        it = m_controllers.begin();
      }
    }
  }
}

void CControllerManager::Notify(const Observable& obs, const ObservableMessage msg)
{
  if (msg == ObservableMessageAddons)
    UpdateAddons();
}
