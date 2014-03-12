/*
 *      Copyright (C) 2012-2014 Team XBMC
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

#include "GameClient.h"
#include "FileItem.h"
//#include "addons/AddonManager.h"
//#include "settings/Settings.h"
//#include "threads/SingleLock.h"
//#include "utils/log.h"
//#include "utils/StringUtils.h"
//#include "utils/URIUtils.h"
//#include "utils/Variant.h"

using namespace ADDON;
using namespace GAME;
using namespace std;

#define GAME_CLIENT_NAME_UNKNOWN     "Unknown"
#define GAME_CLIENT_VERSION_UNKNOWN  "v0.0.0"

/*
namespace GAME
{
  // Helper function
  void toExtensionSet(const CStdString strExtensionList, set<string> &extensions)
  {
    extensions.clear();
    vector<string> vecExtensions = StringUtils::Split(strExtensionList, "|");
    for (vector<string>::iterator it = vecExtensions.begin(); it != vecExtensions.end(); it++)
    {
      string& ext = *it;
      if (ext.empty())
        continue;

      StringUtils::ToLower(ext);

      // Make sure extension starts with "."
      if (ext[0] != '.')
        ext.insert(0, ".");

      extensions.insert(ext);
    }
  }
} // namespace GAMES
*/

CGameClient::CGameClient(const AddonProps& props)
  : CAddonDll<DllGameClient, GameClient, game_client_properties>(props),
    m_apiVersion("0.0.0")
{
  ResetProperties();

  InfoMap::const_iterator it;
  if ((it = props.extrainfo.find("platforms")) != props.extrainfo.end())
    SetPlatforms(it->second);
  if ((it = props.extrainfo.find("extensions")) != props.extrainfo.end())
    SetExtensions(it->second);
  if ((it = props.extrainfo.find("supports_vfs")) != props.extrainfo.end())
    m_bSupportsVFS = (it->second == "true" || it->second == "yes");
}

CGameClient::CGameClient(const cp_extension_t* ext)
  : CAddonDll<DllGameClient, GameClient, game_client_properties>(ext),
    m_apiVersion("0.0.0")
{
  ResetProperties();

  if (ext)
  {
    string strPlatforms = CAddonMgr::Get().GetExtValue(ext->configuration, "platforms");
    if (!strPlatforms.empty())
    {
      Props().extrainfo.insert(make_pair("platforms", strPlatforms));
      SetPlatforms(strPlatforms);
    }

    string strExtensions = CAddonMgr::Get().GetExtValue(ext->configuration, "extensions");
    if (!strExtensions.empty())
    {
      Props().extrainfo.insert(make_pair("extensions", strExtensions));
      SetExtensions(strExtensions);
    }

    string strSupportsVFS = CAddonMgr::Get().GetExtValue(ext->configuration, "allowvfs");
    if (!strSupportsVFS.empty())
    {
      Props().extrainfo.insert(make_pair("supports_vfs", strSupportsVFS));
      m_bSupportsVFS = (strSupportsVFS == "true" || strSupportsVFS == "yes");
    }
  }
}

void CGameClient::ResetProperties()
{
  m_apiVersion = AddonVersion("0.0.0");
  m_bReadyToUse = false;
  m_strClientName.clear();
  m_strClientVersion.clear();

  /*
  m_bSupportsVFS = false;
  m_bRequireZip = false;
  m_bIsPlaying = false;
  m_bIsInited = false;
  m_frameRate = 0.0;
  m_frameRateCorrection = 1.0;
  m_sampleRate = 0.0;
  m_region = -1; // invalid
  m_serialSize = 0;
  m_bRewindEnabled = false;
  */
}

ADDON_STATUS CGameClient::Create(void)
{
  ADDON_STATUS status = ADDON_STATUS_UNKNOWN;

  // Ensure that a previous instance is destroyed
  Destroy();

  // Reset all properties to defaults
  ResetProperties();

  // Initialise the add-on
  bool bReadyToUse = false;
  CLog::Log(LOGDEBUG, "GAME - %s - creating game add-on instance '%s'", __FUNCTION__, Name().c_str());
  try
  {
    status = CAddonDll<DllGameClient, GameClient, game_client_properties>::Create();
    if (status == ADDON_STATUS_OK)
      bReadyToUse = GetAddonProperties();
  }
  catch (...) { LogException(__FUNCTION__); }

  m_bReadyToUse = bReadyToUse;

  return status;
}

void CGameClient::Destroy(void)
{
  // Reset 'ready to use' to false
  if (!m_bReadyToUse)
    return;
  m_bReadyToUse = false;

  CLog::Log(LOGDEBUG, "GAME: %s - destroying game add-on '%s'", __FUNCTION__, m_strClientName.c_str());

  // Destroy the add-on
  try { CAddonDll<DllGameClient, GameClient, game_client_properties>::Destroy(); }
  catch (...) { LogException(__FUNCTION__); }

  // Reset all properties to defaults
  ResetProperties();
}

bool CGameClient::GetAddonProperties(void)
{
  string strClientName;
  string strClientVersion;
  string strValidExtensions;
  bool   bSupportsVFS;
  bool   bRequireArchive;
  
  try { strClientName = m_pStruct->GetClientName(); }
  catch (...) { LogException("GetClientName()"); return false; }

  try { strClientVersion = m_pStruct->GetClientVersion(); }
  catch (...) { LogException("GetClientVersion()"); return false; }

  try { strValidExtensions = m_pStruct->GetValidExtensions(); }
  catch (...) { LogException("GetValidExtensions()"); return false; }

  try { bSupportsVFS = m_pStruct->SupportsVFS(); }
  catch (...) { LogException("SupportsVFS()"); return false; }

  // These properties are declared in addon.xml. Make sure they match the values
  // reported by the game client. This is primarily to avoid errors when adding
  // addon.xml files to libretro cores.
  if (m_bSupportsVFS != bSupportsVFS)
  {
    CLog::Log(LOGERROR, "GAME: <supports_vfs> tag in addon.xml doesn't match DLL value (%s)", bSupportsVFS ? "true" : "false");
    return false;
  }
  if (m_strValidExtensions != strValidExtensions) // != operator defined above
  {
    CLog::Log(LOGERROR, "GAME: <extensions> tag in addon.xml doesn't match the set from DLL (%s)", strValidExtensions.c_str());
    return false;
  }

  // Update client name and version
  m_strClientName    = strClientName;
  m_strClientVersion = strClientVersion;

  CLog::Log(LOGINFO, "GAME: ------------------------------------");
  CLog::Log(LOGINFO, "GAME: Loaded DLL for %s", ID().c_str());
  CLog::Log(LOGINFO, "GAME: Client: %s at version %s", m_strClientName.c_str(), m_strClientVersion.c_str());
  CLog::Log(LOGINFO, "GAME: Valid extensions: %s", strValidExtensions.c_str());
  CLog::Log(LOGINFO, "GAME: Supports VFS: %s", m_bSupportsVFS ? "yes" : "no");
  CLog::Log(LOGINFO, "GAME: ------------------------------------");

  return true;
}

bool CGameClient::OpenFile(const CFileItem& file)
{
  CSingleLock lock(m_critSection);

  if (!ReadyToUse())
    return false;

  if (file.HasProperty("gameclient") && file.GetProperty("gameclient").asString() != ID())
  {
    CLog::Log(LOGERROR, "GAME: File has \"gameclient\" property set, but it doesn't match mine!");
    return false;
  }
  
  CloseFile();

  if (!OpenInternal(file))
    return false;

  m_bIsPlaying = true;

  LoadGameInfo();

  InitSerialization();

  // Query the game region
  switch (m_dll.retro_get_region())
  {
  case RETRO_REGION_NTSC:
    m_region = RETRO_REGION_NTSC;
    break;
  case RETRO_REGION_PAL:
    m_region = RETRO_REGION_PAL;
    break;
  default:
    break;
  }

  // TODO: Need an API call in libretro that lets us know the number of ports
  SetDevice(0, RETRO_DEVICE_JOYPAD);

  return true;
}

bool CGameClient::OpenInternal(const CFileItem& file)
{
  game_info info;

  CGameFileLoaderUseHD        hd;
  CGameFileLoaderUseParentZip outerzip;
  CGameFileLoaderUseVFS       vfs;
  CGameFileLoaderEnterZip     innerzip;

  CGameFileLoader *strategy[4];
  GetStrategy(hd, outerzip, vfs, innerzip, strategy);

  bool success = false;
  for (unsigned int i = 0; i < sizeof(strategy) / sizeof(strategy[0]) && !success; i++)
  {
    if (!strategy[i]->CanLoad(*this, file, m_gameFile))
      continue;
    m_gameFile.ToInfo(info);

    try { success = m_pStruct->LoadGame(&info); }
    catch (...) { LogException("LoadGame()"); }

    if (success)
    {
      CLog::Log(LOGINFO, "GAME: Client successfully loaded game");
      return true;
    }
  }

  CLog::Log(LOGINFO, "GAME: Client failed to load game");
  return false;
}

void CGameClient::CloseFile()
{
  CSingleLock lock(m_critSection);

  if (m_dll.IsLoaded() && m_bIsPlaying)
  {
    m_dll.retro_unload_game();
    m_bIsPlaying = false;
  }

  m_gameFile.Reset();
}

bool CGameClient::LoadGameInfo()
{
  // Get information about system audio/video timings and geometry
  // Can be called only after retro_load_game()
  game_system_av_info av_info = { };
  
  try
  {
    GAME_ERROR error = m_pStruct->GetSystemAVInfo(&av_info);
    if (error != GAME_ERROR_NO_ERROR)
      return false;
  }
  catch (...) { LogException("GetSystemAVInfo()"); return false; }

  CLog::Log(LOGINFO, "GAME: ---------------------------------------");
  CLog::Log(LOGINFO, "GAME: Opened file %s",   m_gameFile.Path().c_str());
  CLog::Log(LOGINFO, "GAME: Base Width:   %u", av_info.geometry.base_width);
  CLog::Log(LOGINFO, "GAME: Base Height:  %u", av_info.geometry.base_height);
  CLog::Log(LOGINFO, "GAME: Max Width:    %u", av_info.geometry.max_width);
  CLog::Log(LOGINFO, "GAME: Max Height:   %u", av_info.geometry.max_height);
  CLog::Log(LOGINFO, "GAME: Aspect Ratio: %f", av_info.geometry.aspect_ratio);
  CLog::Log(LOGINFO, "GAME: FPS:          %f", av_info.timing.fps);
  CLog::Log(LOGINFO, "GAME: Sample Rate:  %f", av_info.timing.sample_rate);
  CLog::Log(LOGINFO, "GAME: ---------------------------------------");

  m_frameRate = av_info.timing.fps;
  m_sampleRate = av_info.timing.sample_rate;
}

bool CGameClient::LogError(const GAME_ERROR error, const char* strMethod) const
{
  if (error != GAME_ERROR_NO_ERROR)
  {
    CLog::Log(LOGERROR, "GAME - %s - addon '%s' returned an error: %s",
        strMethod, m_strClientName.c_str(), ToString(error));
    return false;
  }
  return true;
}

void CGameClient::LogException(const char* strFunctionName) const
{
  CLog::Log(LOGERROR, "GAME: exception caught while trying to call '%s' on add-on '%s'. Please contact the developer of this add-on: %s", strFunctionName, GetFriendlyName().c_str(), Author().c_str());
}

/* static */
void CGameClient::GetStrategy(CGameFileLoaderUseHD& hd, CGameFileLoaderUseParentZip& outerzip,
    CGameFileLoaderUseVFS& vfs, CGameFileLoaderEnterZip& innerzip, CGameFileLoader* strategies[4])
{
  if (!CSettings::Get().GetBool("gamesdebug.prefervfs"))
  {
    // Passing file names comes first
    strategies[0] = &hd;
    strategies[1] = &outerzip;
    strategies[2] = &vfs;
    strategies[3] = &innerzip;
  }
  else
  {
    // Loading through VFS comes first
    strategies[0] = &vfs;
    strategies[1] = &innerzip;
    strategies[2] = &hd;
    strategies[3] = &outerzip;
  }
}

bool CGameClient::CanOpen(const CFileItem& file) const
{
  return CGameFileLoader::CanOpen(*this, file);
}

void CGameClient::InitSerialization()
{
  // Check if serialization is supported so savestates and rewind can be used
  m_serialSize = m_dll.retro_serialize_size();
  m_bRewindEnabled = CSettings::Get().GetBool("gamesgeneral.enablerewind");

  if (!m_serialSize)
  {
    CLog::Log(LOGINFO, "GAME: Game serialization not supported, continuing without save or rewind");
    m_bRewindEnabled = false;
  }

  // Set up rewind functionality
  if (m_bRewindEnabled)
  {
    m_serialState.Init(m_serialSize, (size_t)(CSettings::Get().GetInt("gamesgeneral.rewindtime") * GetFrameRate()));

    if (m_dll.retro_serialize(m_serialState.GetState(), m_serialState.GetFrameSize()))
      CLog::Log(LOGDEBUG, "GAME: Rewind is enabled");
    else
    {
      m_serialSize = 0;
      m_bRewindEnabled = false;
      m_serialState.Reset();
      CLog::Log(LOGDEBUG, "GAME: Unable to serialize state, proceeding without rewind");
    }
  }
}

void CGameClient::SetDevice(unsigned int port, unsigned int device)
{
  if (m_bIsPlaying)
  {
    // Validate port (TODO: Check if port is less that players that individual game client supports)
    if (port < GAMECLIENT_MAX_PLAYERS)
    {
      // Validate device
      if (device <= RETRO_DEVICE_ANALOG ||
          device == RETRO_DEVICE_JOYPAD_MULTITAP ||
          device == RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE ||
          device == RETRO_DEVICE_LIGHTGUN_JUSTIFIER ||
          device == RETRO_DEVICE_LIGHTGUN_JUSTIFIERS)
      {
        m_dll.retro_set_controller_port_device(port, device);
      }
    }
  }
}

bool CGameClient::RunFrame()
{
  CSingleLock lock(m_critSection);

  if (m_bIsPlaying)
  {
    try
    {
      m_dll.retro_run();
    }
    catch (...)
    {
      CLog::Log(LOGERROR, "GameClient error: exception thrown in retro_run()");
      return false;
    }

    // Append a new state delta to the rewind buffer
    if (m_bRewindEnabled)
    {
      if (m_dll.retro_serialize(m_serialState.GetNextState(), m_serialState.GetFrameSize()))
        m_serialState.AdvanceFrame();
      else
      {
        CLog::Log(LOGERROR, "GameClient core claimed it could serialize, but failed.");
        m_bRewindEnabled = false;
      }
    }
  }

  return true;
}

unsigned int CGameClient::RewindFrames(unsigned int frames)
{
  CSingleLock lock(m_critSection);

  unsigned int rewound = 0;
  if (m_bIsPlaying && m_bRewindEnabled)
  {
    rewound = m_serialState.RewindFrames(frames);
    if (rewound)
      m_dll.retro_unserialize(m_serialState.GetState(), m_serialState.GetFrameSize());
  }
  return rewound;
}

void CGameClient::Reset()
{
  if (m_bIsPlaying)
  {
    // TODO: Reset all controller ports to their same value. bSNES since v073r01
    // resets controllers to JOYPAD after a reset, so guard against this.
    m_dll.retro_reset();

    if (m_bRewindEnabled)
    {
      m_serialState.Init(m_serialState.GetFrameSize(), m_serialState.GetMaxFrames());
      if (!m_dll.retro_serialize(m_serialState.GetState(), m_serialState.GetFrameSize()))
      {
        m_bRewindEnabled = false;
        CLog::Log(LOGINFO, "GAME::Reset - Unable to serialize state, proceeding without rewind");
      }
    }
  }
}

void CGameClient::SetFrameRateCorrection(double correctionFactor)
{
  if (correctionFactor != 0.0)
    m_frameRateCorrection = correctionFactor;
  if (m_bRewindEnabled)
    m_serialState.SetMaxFrames((size_t)(CSettings::Get().GetInt("gamesgeneral.rewindtime") * GetFrameRate()));
}

void CGameClient::SetExtensions(const string &strExtensionList)
{
  // If no extensions are provided, don't erase the ones we are already tracking
  if (strExtensionList.empty())
    return;

  toExtensionSet(strExtensionList, m_extensions);
}

void CGameClient::SetPlatforms(const string& strPlatformList)
{
  // If no platforms are provided, don't erase the ones we are already tracking
  if (strPlatformList.empty())
    return;

  m_platforms.clear();
  vector<string> platforms = StringUtils::Split(strPlatformList, "|");
  for (vector<string>::iterator it = platforms.begin(); it != platforms.end(); it++)
  {
    StringUtils::Trim(*it);
    GamePlatform id = CGameInfoTagLoader::GetPlatformInfoByName(*it).id;
    if (id != PLATFORM_UNKNOWN)
      m_platforms.push_back(id);
  }
}

bool CGameClient::IsExtensionValid(const string& ext) const
{
  return CGameFileLoader::IsExtensionValid(ext, m_extensions);
}
