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
#include "addons/AddonDatabase.h"
#include "addons/AddonInstaller.h"
#include "Application.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "filesystem/Directory.h"
#include "profiles/ProfilesManager.h"
#include "threads/SingleLock.h"
#include "URL.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"

using namespace ADDON;
using namespace GAME;
using namespace XFILE;
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


/* static */
CGameManager& CGameManager::Get()
{
  static CGameManager gameManagerInstance;
  return gameManagerInstance;
}

void CGameManager::Start()
{
  CAddonMgr::Get().RegisterAddonMgrCallback(ADDON_GAMEDLL, this);
  CAddonMgr::Get().RegisterObserver(this);
  CAddonInstaller::Get().RegisterObserver(this);
  CAddonDatabase::RegisterAddonDatabaseCallback(ADDON_GAMEDLL, this);

  // TODO: Run these off-thread
  // CAddonMgr::Init() is called before CGameManager::Start(), so we won't
  // receive the first ObservableMessageAddons message
  UpdateAddons();
  UpdateRemoteAddons();
}

void CGameManager::Stop()
{
  CAddonMgr::Get().UnregisterAddonMgrCallback(ADDON_GAMEDLL);
  CAddonMgr::Get().UnregisterObserver(this);
  // TODO: Why does this crash?
  //CAddonInstaller::Get().UnregisterObserver(this);
  CAddonDatabase::UnregisterAddonDatabaseCallback(ADDON_GAMEDLL);
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
      mappedClient->Create();
    else
      mappedClient->Destroy();

    return bRestart ? mappedClient->ReadyToUse() : true;
  }

  return false;
}

bool CGameManager::UpdateAddons()
{
  VECADDONS gameClients;
  if (CAddonMgr::Get().GetAddons(ADDON_GAMEDLL, gameClients))
  {
    for (VECADDONS::const_iterator it = gameClients.begin(); it != gameClients.end(); it++)
      RegisterAddon(boost::dynamic_pointer_cast<CGameClient>(*it));
  }
  return true;
}

void CGameManager::UpdateRemoteAddons()
{
  CAddonDatabase database;
  if (database.Open())
  {
    VECADDONS addons;

    // Get add-ons in remote repositories
    database.GetAddons(addons, ADDON_GAMEDLL);
    database.Close();

    // Get enabled add-ons installed locally
    CAddonMgr::Get().GetAddons(ADDON_GAMEDLL, addons, true);

    // Get disabled add-ons installed locally
    CAddonMgr::Get().GetAddons(ADDON_GAMEDLL, addons, false);

    CSingleLock lock(m_critSection);

    for (VECADDONS::const_iterator it = addons.begin(); it != addons.end(); ++it)
    {
      const AddonPtr& addon = *it;
      if (!addon->IsType(ADDON_GAMEDLL))
        continue;

      GameClientPtr gc = boost::dynamic_pointer_cast<CGameClient>(addon);
      if (!gc)
        continue;

      bool bIsBroken = !gc->Props().broken.empty();
      if (!bIsBroken && !gc->GetExtensions().empty())
        m_gameExtensions.insert(gc->GetExtensions().begin(), gc->GetExtensions().end());
    }
    CLog::Log(LOGDEBUG, "GameManager: tracking %d remote extensions", (int)(m_gameExtensions.size()));
  }
}

bool CGameManager::RegisterAddon(const GameClientPtr& client)
{
  if (!client)
    return false;

  // This special game client is a wrapper for libretro cores. It shouldn't be
  // registered as a real game client.
  if (client->ID() == LIBRETRO_WRAPPER_LIBRARY)
    return false;

  CAddonDatabase database;
  if (!database.Open())
    return false;

  // This logic is from PVRClients.cpp. In addition to the enabled status, it
  // also checks that the game client is installed and configured. If the client
  // has been installed but is not configured yet, it will be disabled in the
  // database.
  if (!client->Enabled() || database.IsAddonDisabled(client->ID()))
    return false;

  CSingleLock lock(m_critSection);

  if (!m_gameClients.empty())
  {
    GameClientMap::const_iterator it = m_gameClients.find(client->ID());
    if (it != m_gameClients.end())
      return true; // Already registered
  }

  if (!client->Create())
  {
    lock.Leave();

    CLog::Log(LOGERROR, "GameManager: failed to load DLL for %s, disabling in database", client->ID().c_str());
    CGUIDialogKaiToast::QueueNotification(client->Icon(), client->Name(), g_localizeStrings.Get(15023)); // Error loading DLL

    // Removes the game client from m_gameClients via CAddonDatabase callback
    CAddonDatabase database;
    if (database.Open())
      database.DisableAddon(client->ID());
    return false;
  }

  client->Destroy();

  m_gameClients[client->ID()] = client;
  CLog::Log(LOGDEBUG, "GameManager: Registered add-on %s", client->ID().c_str());

  // If a file was queued by RetroPlayer, try to launch the newly installed game client
  m_fileLauncher.Launch(client);

  return true;
}

void CGameManager::UnregisterAddonByID(const string& strClientId)
{
  CSingleLock lock(m_critSection);

  GameClientMap::iterator it = m_gameClients.find(strClientId);
  if (it != m_gameClients.end())
  {
    it->second->Destroy();
    m_gameClients.erase(it);
  }
  else
  {
    CLog::Log(LOGERROR, "GameManager: can't unregister %s - not registered!", strClientId.c_str());
  }
}

bool CGameManager::GetClient(const string& strClientId, GameClientPtr& addon) const
{
  CSingleLock lock(m_critSection);

  GameClientMap::const_iterator itr = m_gameClients.find(strClientId);
  if (itr != m_gameClients.end())
  {
    addon = itr->second;
    return true;
  }
  return false;
}

bool CGameManager::GetConnectedClient(const string& strClientId, GameClientPtr& addon) const
{
  return GetClient(strClientId, addon) && addon->ReadyToUse();
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
  GameClientMap::const_iterator itr = m_gameClients.find(addon->ID());
  if (itr != m_gameClients.end())
    return itr->second->ReadyToUse();
  return false;
}

void CGameManager::GetGameClientIDs(const CFileItem& file, vector<string>& candidates) const
{
  CSingleLock lock(m_critSection);

  string requestedClient = file.GetProperty("gameclient").asString();

  for (GameClientMap::const_iterator it = m_gameClients.begin(); it != m_gameClients.end(); it++)
  {
    if (!requestedClient.empty() && requestedClient != it->first)
      continue;

    CLog::Log(LOGDEBUG, "GameManager: To open or not to open using %s, that is the question", it->second->ID().c_str());
    // TODO
    /*
    if (CGameFileLoader::CanOpen(*it->second, file))
    {
      CLog::Log(LOGDEBUG, "GameManager: Adding client %s as a candidate", it->second->ID().c_str());
      candidates.push_back(it->second->ID());
    }
    */

    // If the requested client isn't installed, there are no valid candidates
    if (!requestedClient.empty())
      break;
  }
}

void CGameManager::GetExtensions(vector<string> &exts) const
{
  exts.insert(exts.end(), m_gameExtensions.begin(), m_gameExtensions.end());
}

bool CGameManager::IsGame(const std::string &path) const
{
  CSingleLock lock(m_critSection);

  // Get the file extension (must use a CURL, if the string is top-level zip
  // directory it might not end in .zip)
  string extension(URIUtils::GetExtension(CURL(path).GetFileNameWithoutPath()));
  StringUtils::ToLower(extension);
  if (extension.empty())
    return false;

  return m_gameExtensions.find(extension) != m_gameExtensions.end();
}

void CGameManager::Notify(const Observable& obs, const ObservableMessage msg)
{
  if (msg == ObservableMessageAddons)
    UpdateAddons();
  else if (msg == ObservableMessageRemoteAddons)
    UpdateRemoteAddons();
}

void CGameManager::AddonEnabled(AddonPtr addon, bool bDisabled)
{
  if (!addon)
    return;

  if (addon->Type() == ADDON_GAMEDLL)
    RegisterAddon(boost::dynamic_pointer_cast<CGameClient>(addon));
}

void CGameManager::AddonDisabled(AddonPtr addon)
{
  if (!addon)
    return;

  if (addon->Type() == ADDON_GAMEDLL)
    UnregisterAddonByID(addon->ID());
}
