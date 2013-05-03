/*
 *      Copyright (C) 2012-2013 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "GameManager.h"
#include "addons/AddonManager.h"
#include "Application.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "dialogs/GUIDialogYesNo.h"
#include "filesystem/Directory.h"
#include "GameFileLoader.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/WindowIDs.h"
#include "settings/Settings.h"
#include "threads/SingleLock.h"
#include "URL.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"

using namespace ADDON;
using namespace GAME_INFO;


/* TEMPORARY */
// Remove this struct when libretro has an API call to query the number of
// controller ports a game supports. If this code is still here in six months,
// Garrett will be very unhappy. I found a buffer overflow in SNES9x when
// trying to set controller ports 3-8, so this API call needs to happen.
/*
struct PortMapping
{
  GamePlatform platform;
  int          ports;
};

static const PortMapping ports[] =
{
  { PLATFORM_GAMEBOY,              1 },
  { PLATFORM_GAMEBOY_COLOR,        1 },
  { PLATFORM_GAMEBOY_ADVANCE,      1 },
  { PLATFORM_NEO_GEO_POCKET_COLOR, 1 },
  { PLATFORM_SEGA_MASTER_SYSTEM,   2 },
  { PLATFORM_SNES,                 2 },
};
*/

/* static */
CGameManager &CGameManager::Get()
{
  static CGameManager gameManagerInstance;
  return gameManagerInstance;
}

void CGameManager::RegisterAddons(const VECADDONS &addons)
{
  for (VECADDONS::const_iterator it = addons.begin(); it != addons.end(); it++)
    RegisterAddon(boost::dynamic_pointer_cast<CGameClient>(*it));
}

void CGameManager::RegisterAddon(GameClientPtr clientAddon, bool launchQueued /* = true */)
{
  // Sanity check
  if (!clientAddon)
    return;

  CLog::Log(LOGDEBUG, "CGameManager: registering add-on %s", clientAddon->ID().c_str());

  CSingleLock lock(m_critSection);

  // If we are already tracking the add-on, erase it so we can refresh the data
  for (std::vector<GameClientConfig>::iterator it = m_gameClients.begin(); it != m_gameClients.end(); it++)
  {
    if (clientAddon->ID() == it->id)
    {
      CLog::Log(LOGERROR, "CGameManager: Refreshing information for add-on %s!", clientAddon->ID().c_str());
      m_gameClients.erase(it);
      break;
    }
  }

  // Load the DLL
  if (!clientAddon->Init())
  {
    CLog::Log(LOGERROR, "CGameManager: failed to load DLL for %s, disabling in database", clientAddon->ID().c_str());
    CGUIDialogKaiToast::QueueNotification(clientAddon->Icon(), clientAddon->Name(), g_localizeStrings.Get(15023)); // Incompatible DLL
    CAddonDatabase database;
    if (database.Open())
      database.DisableAddon(clientAddon->ID());
    return;
  }

  m_gameClients.push_back(clientAddon->GetConfig());

  // Unload the DLL
  clientAddon->DeInit();

  // If a file was queued by RetroPlayer, test to see if we should launch the
  // newly installed game client
  if (launchQueued && !m_queuedFile.GetPath().empty())
  {
    // Test if the new client can launch the file
    CStdStringArray candidates;
    GetGameClientIDs(m_queuedFile, candidates);
    if (std::find(candidates.begin(), candidates.end(), clientAddon->ID()) != candidates.end())
    {
      LaunchFile(m_queuedFile, clientAddon->ID());
      // Don't ask the user twice
      m_queuedFile = CFileItem();
    }
  }
}

void CGameManager::UnregisterAddonByID(const CStdString &ID)
{
  CSingleLock lock(m_critSection);

  for (std::vector<GameClientConfig>::iterator it = m_gameClients.begin(); it != m_gameClients.end(); it++)
    if (ID == it->id)
      { m_gameClients.erase(it); return; }
  CLog::Log(LOGERROR, "CGameManager: can't unregister %s - not registered!", ID.c_str());
}

void CGameManager::RegisterRemoteAddons(const VECADDONS &addons, bool fromDatabase /* cosmetic */)
{
  CSingleLock lock(m_critSection);

  m_gameExtensions.clear();

  // First, populate the extensions list with our local extensions so that
  // IsGame() is tested against them as well
  for (std::vector<GameClientConfig>::iterator itLocal = m_gameClients.begin(); itLocal != m_gameClients.end(); itLocal++)
    m_gameExtensions.insert(itLocal->extensions.begin(), itLocal->extensions.end());

  for (VECADDONS::const_iterator itRemote = addons.begin(); itRemote != addons.end(); itRemote++)
  {
    AddonPtr remote = *itRemote;
    if (!remote->IsType(ADDON_GAMEDLL))
      continue;

    GameClientPtr gc = boost::dynamic_pointer_cast<CGameClient>(remote);
    // If it wasn't created polymorphically, do so now
    if (!gc)
      gc = GameClientPtr(new CGameClient(remote->Props()));

    bool bIsRemoteBroken = !gc->Props().broken.empty();
    bool bHasExtensions = !gc->GetConfig().extensions.empty();

    if (bHasExtensions && !bIsRemoteBroken)
    {
      // Extensions were specified in (unbroken) addon.xml
      m_gameExtensions.insert(gc->GetConfig().extensions.begin(), gc->GetConfig().extensions.end());
    }
    else
    {
      // No extensions listed in addon.xml. If installed, get the extensions from
      // the DLL. If the add-on is broken, also try to get extensions from the DLL.
      CLog::Log(LOGDEBUG, "CGameManager - No extensions for %s v%s in %s",
          gc->ID().c_str(), gc->Version().c_str(), fromDatabase ? "database" : "addon.xml");
    }
  }
  CLog::Log(LOGDEBUG, "CGameManager: tracking %d remote extensions", (int)(m_gameExtensions.size()));
}

bool CGameManager::IsGame(CStdString path)
{
  CSingleLock lock(m_critSection);

  // Reset the queued file. IsGame() is called often enough that leaving the
  // add-on browser should reset the file.
  m_queuedFile = CFileItem();

  // If RegisterRemoteAddons() hasn't been called yet, initialize
  // m_gameExtensions with addons from the database.
  if (m_gameExtensions.empty())
    LoadExtensionsFromDB();

  // Get the file extension (we want .zip if the file is a top-level zip directory)
  CStdString extension(URIUtils::GetExtension(CURL(path).GetFileNameWithoutPath()));
  extension.ToLower();
  if (extension.empty())
    return false;

  return m_gameExtensions.find(extension) != m_gameExtensions.end();
}

void CGameManager::LoadExtensionsFromDB()
{
    CLog::Log(LOGDEBUG, "CGameManager: Initializing remote extensions cache from database");
    VECADDONS addons;
    CAddonDatabase database;
    database.Open();
    database.GetAddons(addons);
    RegisterRemoteAddons(addons, true);
}

void CGameManager::GetExtensions(std::vector<CStdString> &exts)
{
  if (m_gameExtensions.empty())
    LoadExtensionsFromDB();
  exts.insert(exts.end(), m_gameExtensions.begin(), m_gameExtensions.end());
}

void CGameManager::QueueFile(const CFileItem &file)
{
  CSingleLock lock(m_critSection);
  m_queuedFile = file;
}

void CGameManager::LaunchFile(CFileItem file, const CStdString &strGameClient) const
{
  // This makes sure we aren't prompted again by PlayMedia()
  file.SetProperty("gameclient", strGameClient);

  CGUIDialogYesNo *pDialog = dynamic_cast<CGUIDialogYesNo*>(g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO));
  if (pDialog)
  {
    CStdString title(file.GetGameInfoTag()->GetTitle());
    if (title.empty())
      title = URIUtils::GetFileName(m_queuedFile.GetPath());

    pDialog->SetHeading(24025); // Manage emulators...
    pDialog->SetLine(0, 15024); // A compatible emulator was installed for:
    pDialog->SetLine(1, title);
    pDialog->SetLine(2, 20013); // Do you wish to launch the game?
    pDialog->DoModal();

    if (pDialog->IsConfirmed())
    {
      // Close the add-on info dialog, if open
      int iWindow = g_windowManager.GetTopMostModalDialogID(true);
      CGUIWindow *window = g_windowManager.GetWindow(iWindow);
      if (window)
        window->Close();

      g_application.PlayMedia(file);
    }
  }
}

void CGameManager::GetGameClientIDs(const CFileItem& file, CStdStringArray &candidates) const
{
  CSingleLock lock(m_critSection);

  CStdString gameclient = file.GetProperty("gameclient").asString();
  for (std::vector<GameClientConfig>::const_iterator it = m_gameClients.begin(); it != m_gameClients.end(); it++)
  {
    CLog::Log(LOGDEBUG, "GameManager: To open or not to open using %s, that is the question", it->id.c_str());
    if (CGameFileLoader::CanOpen(file, *it, true))
    {
      CLog::Log(LOGDEBUG, "GameManager: Adding client %s as a candidate", it->id.c_str());
      candidates.push_back(it->id);
    }
    if (!gameclient.empty() && it->id == gameclient)
      break; // If the game client isn't installed, it's not a valid candidate
  }
}
