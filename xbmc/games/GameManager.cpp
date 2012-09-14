/*
 *      Copyright (C) 2012 Garrett Brown
 *      Copyright (C) 2012 Team XBMC
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
#include "utils/log.h"
#include "utils/URIUtils.h"

using namespace ADDON;
using namespace GAME_INFO;

typedef struct
{
  GameSystemType system;
  const char *shortName;
  const char *longName;
  const char *extensions;
} SystemMapping;

static const SystemMapping systemInfo[] =
{
  {SYSTEM_GameBoy,        "GB",   "GameBoy",         "gb"},
  {SYSTEM_GameBoyColor,   "GBC",  "GameBoy Color",   "gbc|cgb|sgb"},
  {SYSTEM_GameBoyAdvance, "GBA",  "GameBoy Advance", "gba|agb|elf|mb"}, // bin
  {SYSTEM_SuperNintendo,  "SNES", "Super Nintendo",  "smc|sfc|fig|gd3|gd7|dx2|bsx|swc"}
};

/* TEMPORARY */
// Remove this struct when libretro has an API call to query the number of
// controller ports a system supports. If this code is still here in six months,
// Garrett will be very unhappy. I found a buffer overflow in SNES9x when
// trying to set controller ports 3-8, so this API call needs to happen.
typedef struct { GameSystemType system; int ports; } PortMapping;
static const PortMapping ports[] =
{
  {SYSTEM_GameBoy,        1},
  {SYSTEM_GameBoyColor,   1},
  {SYSTEM_GameBoyAdvance, 1},
  {SYSTEM_SuperNintendo,  2}
};

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

void CGameManager::RegisterAddon(const GameClientPtr &clientAddon)
{
  // Sanity check
  if (!clientAddon)
    return;

  // Check to see if we are already tracking the add-on
  for (std::vector<GameClientObject>::const_iterator it = m_gameClients.begin(); it != m_gameClients.end(); it++)
  {
    if (clientAddon->ID() == it->id)
    {
      CLog::Log(LOGERROR, "CGameManager: already tracking add-on %s!", it->id.c_str());
      return;
    }
  }

  // Load the DLL
  if (!clientAddon->Init())
    return;

  GameClientObject clientObject;
  clientObject.id            = clientAddon->ID();
  clientObject.extensions    = clientAddon->GetExtensions(); // Store systems as a '|' delimited string
  CStdStringArray strSystems = clientAddon->GetSystems();

  // Store system type(s) as a vector. Translate strings to enum types here
  for (CStdStringArray::iterator it = strSystems.begin(); it != strSystems.end(); it++)
  {
    it->Trim();
    // Get the code for the system type from the systems structure
    GameSystemType system = SYSTEM_UNKNOWN;
    for (size_t i = 0; i < sizeof(systemInfo) / sizeof(SystemMapping); i++)
    {
      if (it->Equals(systemInfo[i].shortName) || it->Equals(systemInfo[i].longName))
        clientObject.systems.push_back(systemInfo[i].system);
      // Only push back SYSTEM_OTHER once (or should it only be pushed back if clientObject.systems.size() == 0?)
      else if (std::find(clientObject.systems.begin(), clientObject.systems.end(), SYSTEM_OTHER) != clientObject.systems.end())
        clientObject.systems.push_back(SYSTEM_OTHER);
    }
  }

  // Currently we only store these three fields, so we're all done here
  m_gameClients.push_back(clientObject);

  // Unload the DLL
  clientAddon->DeInit();
}

void CGameManager::UnregisterAddon(const CStdString &ID)
{
  for (std::vector<GameClientObject>::iterator it = m_gameClients.begin(); it != m_gameClients.end(); it++)
    if (ID == it->id)
      { m_gameClients.erase(it); return; }
  CLog::Log(LOGERROR, "CGameManager: can't unregister %s - not registered!", ID.c_str());
}

GameClientPtr CGameManager::GetGameClient(const CStdString &strFile)
{
  CStdString strExtension;
  URIUtils::GetExtension(strFile, strExtension);
  strExtension.ToLower();
  strExtension.TrimLeft('.');
  
  std::vector<CStdString> ids; // Container to hold discovered clients
  for (std::vector<GameClientObject>::const_iterator it = m_gameClients.begin();
      it != m_gameClients.end(); it++)
  {
    // Check to see if the game client supports the extension
    if (ContainsExtesion(it->extensions, strExtension))
      ids.push_back(it->id);
    // Fix for clients that don't specify extensions: match on the system type
    // specified in the game client's addon.xml
    else if (it->extensions.length() == 0)
    {
      GameSystemType type = GetSystemType(strFile);
      if (type != SYSTEM_UNKNOWN && it->systems.size())
        if (std::find(it->systems.begin(), it->systems.end(), type) != it->systems.end())
          ids.push_back(it->id);
    }
  }

  // If we still don't have any candidates, search on system type
  GameSystemType system;
  if (ids.size() == 0 && (system = GetSystemType(strFile)) != SYSTEM_UNKNOWN)
  {
    for (std::vector<GameClientObject>::const_iterator it = m_gameClients.begin();
        it != m_gameClients.end(); it++)
    {
      if (std::find(it->systems.begin(), it->systems.end(), system) != it->systems.end())
      {
        ids.push_back(it->id);
        break;
      }
    }
  }

  AddonPtr addon;
  if (ids.size() == 1)
  {
    CAddonMgr::Get().GetAddon(ids[0], addon, ADDON_GAMEDLL);
    return boost::dynamic_pointer_cast<CGameClient>(addon);
  }
  else if (ids.size() > 1)
  {
    CStdString candidates = ids[0];
    for (std::vector<CStdString>::const_iterator it = ids.begin() + 1; it != ids.end(); it++)
      candidates += ", " + *it;
    CLog::Log(LOGINFO, "GameManager: Multiple game clients found, using %s", ids[0].c_str());
    CLog::Log(LOGINFO, "GameManager: Candidates were %s", candidates.c_str());
    CAddonMgr::Get().GetAddon(ids[0], addon, ADDON_GAMEDLL);
    return boost::dynamic_pointer_cast<CGameClient>(addon);
  }
  return GameClientPtr();
}

GameSystemType CGameManager::GetSystemType(const CStdString &strFile)
{
  CStdString strExtension;
  URIUtils::GetExtension(strFile, strExtension);
  strExtension.ToLower();
  strExtension.TrimLeft('.');

  // Search our structure
  for (size_t i = 0; i < sizeof(systemInfo) / sizeof(SystemMapping); i++)
    if (ContainsExtesion(systemInfo[i].extensions, strExtension))
      return systemInfo[i].system;
  
  // If unfound, search installed add-ons to see if any support the extension
  for (std::vector<GameClientObject>::const_iterator it = m_gameClients.begin();
      it != m_gameClients.end(); it++)
  {
    if (ContainsExtesion(it->extensions, strExtension))
    {
      // If the game client supports unknown extensions, but system type from
      // its <system> tag IS known, then return that system type
      if (it->systems.size())
        return it->systems[0];
      else
        return SYSTEM_OTHER;
    }
  }
  return SYSTEM_UNKNOWN;
}

/* static */
unsigned int CGameManager::GetPlayers(GameSystemType system)
{
  for (size_t i = 0; i < sizeof(ports) / sizeof(PortMapping); i++)
    if (ports[i].system == system)
      return ports[i].ports;
  return 0; // Unknown
}

/* static */
bool CGameManager::ContainsExtesion(const CStdString &strExtensionList, const CStdString &strExt)
{
  const char *extList = strExtensionList.c_str();
  size_t length = strExt.length();

  // Look to see if the extension is present in the extensions list
  // Favor speed over algorithmic elegance: An extension is found when it
  // matches and the next character is '|' or '\0' (avoid partial matches
  // like gb -> gba)
  while (*extList != '\0')
  {
    if (strncmp(strExt.c_str(), extList, length) == 0 &&
        (extList[length] == '\0' || extList[length] == '|'))
    {
      return true;
    }
    do
    {
      // Need to advance to next '|', but avoid '\0'
      // Post increment, so if we hit a '|' we end up 1 char past it
      if (*extList++ == '|')
        break;
    } while (*extList != '\0'); // Failing this slides out of the outer while loop as well
  }
  return false;
}
