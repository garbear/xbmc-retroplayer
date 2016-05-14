/*
 *      Copyright (C) 2016 Team Kodi
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

#include "GUIDialogSelectGameClient.h"
#include "addons/AddonManager.h"
#include "addons/GUIWindowAddonBrowser.h"
#include "dialogs/GUIDialogContextMenu.h"
#include "games/addons/GameClient.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/WindowIDs.h"
#include "utils/log.h"

using namespace GAME;

bool CGUIDialogSelectGameClient::ShowAndGetGameClient(const GameClientVector& candidates, const GameClientVector& installable, GameClientPtr& gameClient)
{
  using namespace ADDON;

  CLog::Log(LOGDEBUG, "Select game client dialog: Found %u candidates", candidates.size());
  for (const auto& gameClient : candidates)
    CLog::Log(LOGDEBUG, "Adding %s as a candidate", gameClient->ID().c_str());

  if (!installable.empty())
  {
    CLog::Log(LOGDEBUG, "Select game client dialog: Found %u installable clients", installable.size());
    for (const auto& gameClient : installable)
      CLog::Log(LOGDEBUG, "Adding %s as an installable client", gameClient->ID().c_str());
  }

  CContextButtons choiceButtons;

  // Add emulators
  int i = 0;
  for (const GameClientPtr& gameClient : candidates)
    choiceButtons.Add(i++, gameClient->Name());

  // Add button to install emulators
  const int iInstallEmulator = i++;
  if (!installable.empty())
    choiceButtons.Add(iInstallEmulator, 35253); // "Install emulator"

  // Add button to manage emulators
  const int iAddonMgr = i++;
  choiceButtons.Add(iAddonMgr, 35254); // "Manage emulators"

  // Do modal
  int result = CGUIDialogContextMenu::ShowAndGetChoice(choiceButtons);

  if (0 <= result && result < static_cast<int>(candidates.size()))
  {
    gameClient = candidates[result];
  }
  else if (result == iInstallEmulator)
  {
    gameClient = InstallGameClient();
  }
  else if (result == iAddonMgr)
  {
    ActivateAddonMgr();
  }
  else
  {
    CLog::Log(LOGDEBUG, "Select game client dialog: User cancelled game client selection");
  }

  return gameClient.get() != nullptr;
}

GameClientPtr CGUIDialogSelectGameClient::InstallGameClient()
{
  using namespace ADDON;

  GameClientPtr gameClient;

  std::string chosenClientId;
  if (CGUIWindowAddonBrowser::SelectAddonID(ADDON_GAMEDLL, chosenClientId, false, true, false, true, false) >= 0 && !chosenClientId.empty())
  {
    CLog::Log(LOGDEBUG, "Select game client dialog: User installed %s", chosenClientId.c_str());
    AddonPtr addon;
    if (CAddonMgr::GetInstance().GetAddon(chosenClientId, addon, ADDON_GAMEDLL))
      gameClient = std::dynamic_pointer_cast<CGameClient>(addon);

    if (!gameClient)
      CLog::Log(LOGERROR, "Select game client dialog: Failed to get addon %s", chosenClientId.c_str());
  }

  return gameClient;
}

void CGUIDialogSelectGameClient::ActivateAddonMgr()
{
  CLog::Log(LOGDEBUG, "User chose to go to the add-on manager");
  std::vector<std::string> params;
  params.push_back("addons://user/category.emulators");
  g_windowManager.ActivateWindow(WINDOW_ADDON_BROWSER, params);
}
