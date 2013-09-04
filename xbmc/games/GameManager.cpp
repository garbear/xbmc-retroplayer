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
//#include "settings/Settings.h"
#include "threads/SingleLock.h"
#include "URL.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"

using namespace ADDON;
using namespace GAME_INFO;
using namespace GAMES;
using namespace std;


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


CGameManager::CGameManager() : CThread("game client updater")
{
}

/* static */
CGameManager &CGameManager::Get()
{
  static CGameManager _instance;
  return _instance;
}

void CGameManager::Start(void)
{
  Stop();
  m_addonDb.Open();
  Create();
  SetPriority(-1);
}

void CGameManager::Stop(void)
{
  StopThread();
  CSingleLock lock(m_critSection);
  for (map<string, GameClientPtr>::const_iterator itr = m_gameClients.begin(); itr != m_gameClients.end(); itr++)
    itr->second->DeInit();
  m_addonDb.Close();
}

void CGameManager::Process(void)
{
  CAddonMgr::Get().RegisterAddonMgrCallback(ADDON_GAMEDLL, this);
  CAddonMgr::Get().RegisterObserver(this);
  CAddonDatabase::RegisterAddonDatabaseCallback(ADDON_GAMEDLL, this);
  UpdateAddons();

  while (!m_bStop)
  {
    UpdateAndInitialiseClients();
    // Wait for StopThread() to be called (no need for our own CEvent)
    AbortableWait(CEvent());
  }

  CAddonMgr::Get().UnregisterAddonMgrCallback(ADDON_GAMEDLL);
  CAddonMgr::Get().UnregisterObserver(this);
  CAddonDatabase::UnregisterAddonDatabaseCallback(ADDON_GAMEDLL);
}

void CGameManager::RegisterAddons(const VECADDONS &addons)
{
  for (VECADDONS::const_iterator it = addons.begin(); it != addons.end(); it++)
    RegisterAddon(boost::dynamic_pointer_cast<CGameClient>(*it));
}

bool CGameManager::RegisterAddon(const GameClientPtr &clientAddon)
{
  if (!clientAddon)
    return false;

  CLog::Log(LOGDEBUG, "CGameManager: registering add-on %s", clientAddon->ID().c_str());

  CSingleLock lock(m_critSection);

  // Add the client to m_gameClients
  GameClientPtr &client = m_gameClients[clientAddon->ID()];
  if (client)
    CLog::Log(LOGERROR, "CGameManager: Refreshing information for add-on %s!", clientAddon->ID().c_str());
  client = clientAddon;

  if (!client->Init())
  {
    CLog::Log(LOGERROR, "CGameManager: failed to load DLL for %s, disabling in database", client->ID().c_str());
    CGUIDialogKaiToast::QueueNotification(client->Icon(), client->Name(), g_localizeStrings.Get(15023)); // Error loading DLL
    // Removes the game client from m_gameClients
    m_addonDb.DisableAddon(client->ID());
    return false;
  }
  else
  {
    // Unload the loaded and inited DLL
    client->DeInit();

    // If a file was queued by RetroPlayer, test to see if we should launch the
    // newly installed game client
    if (m_queuedFile)
    {
      // Test if the new client can launch the file
      vector<string> candidates;
      GetGameClientIDs(*m_queuedFile, candidates);
      if (find(candidates.begin(), candidates.end(), clientAddon->ID()) != candidates.end())
      {
        LaunchFile(*m_queuedFile, clientAddon->ID());
        // Don't ask the user twice
        UnqueueFile();
      }
    }
    return true;
  }
}

void CGameManager::LaunchFile(CFileItem file, const CStdString &strGameClient) const
{
  // This makes sure we aren't prompted again by PlayMedia()
  file.SetProperty("gameclient", strGameClient);

  CGUIDialogYesNo *pDialog = dynamic_cast<CGUIDialogYesNo*>(g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO));
  if (pDialog)
  {
    CStdString title(file.GetGameInfoTag()->GetTitle());
    if (title.empty() && m_queuedFile)
      title = URIUtils::GetFileName(m_queuedFile->GetPath());

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

void CGameManager::UnregisterAddonByID(const string &strId)
{
  CSingleLock lock(m_critSection);

  GameClientPtr client = m_gameClients[strId];
  if (client)
  {
    if (client->IsInitialized())
      client->DeInit();
  }
  else
  {
    CLog::Log(LOGERROR, "CGameManager: can't unregister %s - not registered!", strId.c_str());
  }
  m_gameClients.erase(m_gameClients.find(strId));
}

void CGameManager::RegisterRemoteAddons(const VECADDONS &addons)
{
  CSingleLock lock(m_critSection);

  m_gameExtensions.clear();

  // First, populate the extensions list with our local extensions so that
  // IsGame() is tested against them as well
  for (map<string, GameClientPtr>::iterator itLocal = m_gameClients.begin(); itLocal != m_gameClients.end(); itLocal++)
  {
    const set<string> &extensions = itLocal->second->GetConfig().extensions;
    m_gameExtensions.insert(extensions.begin(), extensions.end());
  }

  for (VECADDONS::const_iterator itRemote = addons.begin(); itRemote != addons.end(); itRemote++)
  {
    const AddonPtr &remote = *itRemote;
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
      const set<string> &extensions = gc->GetConfig().extensions;
      m_gameExtensions.insert(extensions.begin(), extensions.end());
    }
    else
    {
      // No extensions listed in addon.xml. If installed, get the extensions from
      // the DLL. If the add-on is broken, also try to get extensions from the DLL.
      CLog::Log(LOGDEBUG, "CGameManager - No extensions for %s v%s", gc->ID().c_str(), gc->Version().c_str());
    }
  }
  CLog::Log(LOGDEBUG, "CGameManager: tracking %d remote extensions", (int)(m_gameExtensions.size()));
}

void CGameManager::AddonEnabled(AddonPtr addon, bool bDisabled)
{
  if (!addon)
    return;

  // If the enabled addon is a game client, register it
  RegisterAddon(boost::dynamic_pointer_cast<CGameClient>(addon));
}

void CGameManager::AddonDisabled(AddonPtr addon)
{
  // If the disabled addon is a service, stop it
  if (!addon)
    return;

  // If the disabled addon isn't a game client, UnregisterAddon() will still do the right thing
  UnregisterAddonByID(addon->ID());
}

void CGameManager::Notify(const Observable &obs, const ObservableMessage msg)
{
  if (msg == ObservableMessageAddons)
    UpdateAddons();
}

void CGameManager::QueueFile(const CFileItem &file)
{
  CSingleLock lock(m_critSection);
  m_queuedFile = CFileItemPtr(new CFileItem(file));
}

void CGameManager::UnqueueFile()
{
  CSingleLock lock(m_critSection);
  m_queuedFile = CFileItemPtr();
}

bool CGameManager::GetClient(const string &strClientId, GameClientPtr &addon) const
{
  CSingleLock lock(m_critSection);

  map<string, GameClientPtr>::const_iterator itr = m_gameClients.find(strClientId);
  if (itr != m_gameClients.end())
  {
    addon = itr->second;
    return true;
  }
  return false;
}

bool CGameManager::GetConnectedClient(const string &strClientId, GameClientPtr &addon) const
{
  return GetClient(strClientId, addon) && addon->IsInitialized();
}

bool CGameManager::IsConnectedClient(const string &strClientId) const
{
  GameClientPtr client;
  return GetConnectedClient(strClientId, client);
}

bool CGameManager::IsConnectedClient(const AddonPtr addon) const
{
  // See if we are tracking the client
  CSingleLock lock(m_critSection);
  map<string, GameClientPtr>::const_iterator itr = m_gameClients.find(addon->ID());
  if (itr != m_gameClients.end())
    return itr->second->IsInitialized();
  return false;
}

void CGameManager::GetGameClientIDs(const CFileItem& file, vector<string> &candidates) const
{
  CSingleLock lock(m_critSection);

  CStdString requestedClient = file.GetProperty("gameclient").asString();

  // If a start save state was specified, validate the candidate against the
  // save state's game client
  if (!file.m_startSaveState.empty())
  {
    CSavestate savestate;
    savestate.SetPath(file.m_startSaveState);
    if (!savestate.GetGameClient().empty())
    {
      if (requestedClient.empty())
        requestedClient = savestate.GetGameClient(); // Use new game client as filter below
      else if (requestedClient != savestate.GetGameClient())
        return; // New game client doesn't match, no valid candidates
    }
  }

  for (map<string, GameClientPtr>::const_iterator it = m_gameClients.begin(); it != m_gameClients.end(); it++)
  {
    if (!requestedClient.empty() && requestedClient != it->first)
      continue;

    const GameClientConfig &config = it->second->GetConfig();

    CLog::Log(LOGDEBUG, "GameManager: To open or not to open using %s, that is the question", config.id.c_str());
    if (CGameFileLoader::CanOpen(file, config))
    {
      CLog::Log(LOGDEBUG, "GameManager: Adding client %s as a candidate", config.id.c_str());
      candidates.push_back(config.id);
    }

    if (!requestedClient.empty())
      break; // If the requested client isn't installed, it's not a valid candidate
  }
}

void CGameManager::GetExtensions(vector<string> &exts)
{
  if (m_gameExtensions.empty())
    LoadExtensionsFromDB();
  exts.insert(exts.end(), m_gameExtensions.begin(), m_gameExtensions.end());
}

bool CGameManager::IsGame(CStdString path)
{
  CSingleLock lock(m_critSection);

  // Reset the queued file. IsGame() is called often enough that leaving the
  // add-on browser should reset the file.
  UnqueueFile();

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
    m_addonDb.GetAddons(addons);
    RegisterRemoteAddons(addons);
}

bool CGameManager::StopClient(AddonPtr client, bool bRestart)
{
  // This lock is to ensure that ReCreate() or Destroy() are not started from
  // multiple threads.
  CSingleLock lock(m_critSection);

  GameClientPtr mappedClient;
  if (GetClient(client->ID(), mappedClient))
  {
    CLog::Log(LOGDEBUG, "%s - %s add-on '%s'", __FUNCTION__, bRestart ? "restarting" : "stopping", mappedClient->Name().c_str());
    if (bRestart)
      mappedClient->Init();
    else
      mappedClient->DeInit();

    return bRestart ? mappedClient->IsInitialized() : true;
  }

  return false;
}

bool CGameManager::UpdateAndInitialiseClients()
{
  VECADDONS addons;
  {
    if (m_gameClients.empty())
      return false;

    CSingleLock lock(m_critSection);
    addons.reserve(m_gameClients.size());
    for (map<string, GameClientPtr>::const_iterator it = m_gameClients.begin(); it != m_gameClients.end(); ++it)
      addons.push_back(it->second);
  }

  for (VECADDONS::const_iterator it = addons.begin(); it != addons.end(); it++)
  {
    const AddonPtr& addon = *it;

    // It's possible for PVR add-ons to be enabled but not enabled in the
    // database when they're installed but not configured yet.
    bool bEnabled = addon->Enabled() && !m_addonDb.IsAddonDisabled(addon->ID());

    if (!bEnabled)
    {
      StopClient(addon, false);
      CSingleLock lock(m_critSection);
      map<string, GameClientPtr>::iterator addonPtr = m_gameClients.find(addon->ID());
      if (addonPtr != m_gameClients.end())
        m_gameClients.erase(addonPtr);
    }
  }

  return true;
}
