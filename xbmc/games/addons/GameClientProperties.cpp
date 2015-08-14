/*
 *      Copyright (C) 2012-2015 Team XBMC
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

#include "GameClientProperties.h"
#include "GameClient.h"
#include "addons/IAddon.h"
#include "addons/AddonManager.h"
#include "filesystem/SpecialProtocol.h"
#include "settings/Settings.h"

#include <cstring>

using namespace ADDON;
using namespace GAME;

#define GAME_CLIENT_SAVE_DIRECTORY    "save"
#define GAME_CLIENT_SYSTEM_DIRECTORY  "system"

CGameClientProperties::CGameClientProperties(const CGameClient* parent, game_client_properties*& props)
  : m_parent(parent),
    m_properties()
{
  // Allow the caller to access the property structure directly
  props = &m_properties;
}

void CGameClientProperties::ReleaseResources(void)
{
  for (std::vector<char*>::const_iterator it = m_proxyDllPaths.begin(); it != m_proxyDllPaths.end(); ++it)
    delete[] *it;
  m_proxyDllPaths.clear();
}

void CGameClientProperties::InitializeProperties(void)
{
  ReleaseResources();

  m_properties.game_client_dll_path = GetLibraryPath();
  m_properties.proxy_dll_paths      = GetProxyDllPaths();
  m_properties.proxy_dll_count      = GetProxyDllCount();
  m_properties.netplay_server       = GetNetplayServer();
  m_properties.netplay_server_port  = GetNetplayServerPort();
  m_properties.system_directory     = GetSystemDirectory();
  m_properties.content_directory    = GetContentDirectory();
  m_properties.save_directory       = GetSaveDirectory();
}

const char* CGameClientProperties::GetLibraryPath(void)
{
  if (m_strLibraryPath.empty())
  {
    // Get the parent add-on's real path
    std::string strLibPath = dynamic_cast<const CGameClient*>(m_parent)->CAddon::LibPath();
    m_strLibraryPath = CSpecialProtocol::TranslatePath(strLibPath);
  }
  return m_strLibraryPath.c_str();
}

const char* const* CGameClientProperties::GetProxyDllPaths(void)
{
  // Add all game client dependencies
  // TODO: Compare helper version with required dependency
  const ADDONDEPS& dependencies = m_parent->GetDeps();
  for (ADDONDEPS::const_iterator it = dependencies.begin(); it != dependencies.end(); ++it)
    AddProxyDll(it->first);

  return !m_proxyDllPaths.empty() ? m_proxyDllPaths.data() : NULL;
}

const char* CGameClientProperties::GetNetplayServer(void)
{
  return "127.0.0.1"; // TODO
}

unsigned int CGameClientProperties::GetNetplayServerPort(void)
{
  return 35890; // TODO
}

const char* CGameClientProperties::GetSystemDirectory(void)
{
  if (m_strSystemDirectory.empty())
    m_strSystemDirectory = CSpecialProtocol::TranslatePath(URIUtils::AddFileToFolder(m_parent->Profile(), GAME_CLIENT_SYSTEM_DIRECTORY));
  return m_strSystemDirectory.c_str();
}

const char* CGameClientProperties::GetContentDirectory(void)
{
  if (m_strContentDirectory.empty())
    m_strContentDirectory = CSpecialProtocol::TranslatePath(URIUtils::GetParentPath(GetLibraryPath()));
  return m_strContentDirectory.c_str();
}

const char* CGameClientProperties::GetSaveDirectory(void)
{
  if (m_strSaveDirectory.empty())
    m_strSaveDirectory = CSpecialProtocol::TranslatePath(URIUtils::AddFileToFolder(m_parent->Profile(), GAME_CLIENT_SAVE_DIRECTORY));
  return m_strSaveDirectory.c_str();
}

bool CGameClientProperties::AddProxyDll(const std::string& strAddonId)
{
  // Ignore add-on if it isn't a game client
  AddonPtr addon;
  if (CAddonMgr::Get().GetAddon(strAddonId, addon, ADDON_GAMEDLL) && addon)
  {
    // Get the add-on's real path
    std::string strLibPath = dynamic_cast<const CGameClient*>(addon.get())->CAddon::LibPath();

    // Ignore add-on if it is already added
    if (!HasProxyDll(strLibPath))
    {
      char* libPath = new char[strLibPath.length() + 1];
      std::strcpy(libPath, strLibPath.c_str());
      m_proxyDllPaths.push_back(libPath);
      return true;
    }
  }
  return false;
}

bool CGameClientProperties::HasProxyDll(const std::string& strLibPath) const
{
  for (std::vector<char*>::const_iterator it = m_proxyDllPaths.begin(); it != m_proxyDllPaths.end(); ++it)
  {
    if (strLibPath == *it)
      return true;
  }
  return false;
}
