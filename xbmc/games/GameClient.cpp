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

#include "GameClient.h"
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
  // Helper functions
  void toExtensionSet(const CStdString strExtensionList, set<string> &extensions)
  {
    extensions.clear();
    vector<string> vecExtensions = StringUtils::Split(strExtensionList, "|");
    for (vector<string>::iterator it = vecExtensions.begin(); it != vecExtensions.end(); it++)
    {
      string &ext = *it;
      if (ext.empty())
        continue;

      StringUtils::ToLower(ext);

      // Make sure extension starts with "."
      if (ext[0] != '.')
        ext.insert(0, ".");

      extensions.insert(ext);
    }
  }

  // Returns true if lhs and rhs are equal sets
  // TODO: Use functions from std library
  bool operator==(const set<string> &lhs, const set<string> &rhs)
  {
    if (lhs.size() != rhs.size())
      return false;
    for (set<string>::const_iterator itl = lhs.begin(), itr = rhs.begin(); itl != lhs.end(); itl++, itr++)
    {
      if ((*itl) != (*itr))
        return false;
    }
    return true;
  }

  bool operator!=(const set<string> &lhs, const set<string> &rhs)
  {
    return !(lhs == rhs);
  }
} // namespace GAMES
*/

CGameClient::CGameClient(const AddonProps &props)
  : CAddonDll<DllGameClient, GameClient, game_client_properties>(props),
    m_apiVersion("0.0.0")
{
  ResetProperties();

  InfoMap::const_iterator it;
  if ((it = props.extrainfo.find("platforms")) != props.extrainfo.end())
    SetPlatforms(it->second);
  if ((it = props.extrainfo.find("extensions")) != props.extrainfo.end())
    SetExtensions(it->second);
  if ((it = props.extrainfo.find("allowvfs")) != props.extrainfo.end())
    m_bAllowVFS = (it->second == "true" || it->second == "yes");
  if ((it = props.extrainfo.find("requirearchive")) != props.extrainfo.end())
    m_bRequireArchive = (it->second == "true" || it->second == "yes");
}

CGameClient::CGameClient(const cp_extension_t *ext)
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

    string strAllowVFS = CAddonMgr::Get().GetExtValue(ext->configuration, "allowvfs");
    if (!strAllowVFS.empty())
    {
      Props().extrainfo.insert(make_pair("allowvfs", strAllowVFS));
      m_bAllowVFS = (strAllowVFS == "true" || strAllowVFS == "yes");
    }

    string strRequireArchive = CAddonMgr::Get().GetExtValue(ext->configuration, "requirearchive");
    if (!strRequireArchive.empty())
    {
      Props().extrainfo.insert(make_pair("requirearchive", strRequireArchive));
      m_bRequireArchive = (strRequireArchive == "true" || strRequireArchive == "yes");
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
  m_bAllowVFS = false;
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
  if (!m_bReadyToUse)
    return;
  m_bReadyToUse = false;

  // Reset 'ready to use' to false
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
  bool   bAllowVFS;
  bool   bRequireArchive;
  
  try { strClientName = m_pStruct->GetClientName(); }
  catch (...) { LogException("GetClientName()"); return false; }

  try { strClientVersion = m_pStruct->GetClientVersion(); }
  catch (...) { LogException("GetClientVersion()"); return false; }

  try { strValidExtensions = m_pStruct->GetValidExtensions(); }
  catch (...) { LogException("GetValidExtensions()"); return false; }

  try { bAllowVFS = m_pStruct->AllowVFS(); }
  catch (...) { LogException("AllowVFS()"); return false; }

  try { bRequireArchive = m_pStruct->RequireArchive(); }
  catch (...) { LogException("RequireArchive()"); return false; }

  // These properties are declared in addon.xml. Make sure they match the values
  // reported by the game client.
  if (m_bAllowVFS != bAllowVFS)
  {
    CLog::Log(LOGERROR, "GAME: <allowvfs> tag in addon.xml doesn't match DLL value (%s)",
        bAllowVFS ? "true" : "false");
    return false;
  }
  if (m_bRequireArchive != bRequireArchive)
  {
    CLog::Log(LOGERROR, "GAME: <requirearchive> tag in addon.xml doesn't match DLL value (%s)",
        bRequireArchive ? "true" : "false");
    return false;
  }
  if (m_strValidExtensions != strValidExtensions) // != operator defined above
  {
    CLog::Log(LOGERROR, "GAME: <extensions> tag in addon.xml doesn't match the set from DLL (%s)",
        strValidExtensions..c_str());
    return false;
  }

  // Update client name and version
  m_strClientName    = strClientName;
  m_strClientVersion = strClientVersion;

  CLog::Log(LOGINFO, "GAME: ------------------------------------");
  CLog::Log(LOGINFO, "GAME: Loaded DLL for %s", ID().c_str());
  CLog::Log(LOGINFO, "GAME: Client: %s at version %s", m_strClientName.c_str(), m_strClientVersion.c_str());
  CLog::Log(LOGINFO, "GAME: Valid extensions: %s", strValidExtensions.c_str());
  CLog::Log(LOGINFO, "GAME: Allow VFS: %s, require archive: %s",
      m_bAllowVFS ? "yes" : "no",
      m_bRequireArchive ? "yes" : "no");
  CLog::Log(LOGINFO, "GAME: ------------------------------------");

  return true;
}

bool CGameClient::LogError(const GAME_ERROR error, const char *strMethod) const
{
  if (error != GAME_ERROR_NO_ERROR)
  {
    CLog::Log(LOGERROR, "GAME - %s - addon '%s' returned an error: %s",
        strMethod, m_strClientName.c_str(), ToString(error));
    return false;
  }
  return true;
}

void CGameClient::LogException(const char *strFunctionName) const
{
  CLog::Log(LOGERROR, "GAME: exception caught while trying to call '%s' on add-on '%s'. Please contact the developer of this add-on: %s", strFunctionName, GetFriendlyName().c_str(), Author().c_str());
}

void CGameClient::DeInit()
{
  if (m_dll.IsLoaded())
  {
    CloseFile();

    if (m_bIsInited)
    {
      m_dll.retro_deinit();
      m_bIsInited = false;
    }
    try
    {
      m_dll.Unload();
    }
    catch (exception &e)
    {
      CLog::Log(LOGERROR, "GAME: Error unloading DLL: %s", e.what());
    }
  }
}

/* static */
void CGameClient::GetStrategy(CGameFileLoaderUseHD &hd, CGameFileLoaderUseParentZip &outerzip,
    CGameFileLoaderUseVFS &vfs, CGameFileLoaderEnterZip &innerzip, CGameFileLoader *strategies[4])
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

bool CGameClient::CanOpen(const CFileItem &file) const
{
  return CGameFileLoader::CanOpen(*this, file);
}

bool CGameClient::OpenFile(const CFileItem& file, ILibretroCallbacksAV *callbacks)
{
  CSingleLock lock(m_critSection);

  // Can't open a file without first initializing the DLL...
  if (!m_dll.IsLoaded())
    Init();

  CloseFile();

  if (file.HasProperty("gameclient") && file.GetProperty("gameclient").asString() != ID())
  {
    CLog::Log(LOGERROR, "GAME: File has \"gameclient\" property set, but it doesn't match mine!");
    return false;
  }

  m_callbacks = callbacks;

  // Ensure the default values
  m_callbacks->SetPixelFormat(LIBRETRO::RETRO_PIXEL_FORMAT_0RGB1555);
  m_callbacks->SetKeyboardCallback(NULL);

  // Install the hooks. These are called by CLibretroEnvironment::EnvironmentCallback()
  CLibretroEnvironment::SetDLLCallbacks(this, boost::dynamic_pointer_cast<CGameClient>(Clone()));

  // Because we call m_dll.retro_init() here instead of in Init(), keep track
  // of this. Note that if we return false later, m_bIsInited will still be
  // true because the DLL is still loaded and inited
  if (!m_bIsInited)
  {
    // Set up our callback to retrieve environment variables
    // retro_set_environment() must be called before retro_init()
    m_dll.retro_set_environment(CLibretroEnvironment::EnvironmentCallback);
    m_dll.retro_init();
    m_bIsInited = true;
  }

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

  // Install callbacks (static wrappers for ILibretroCallbacksAV)
  m_dll.retro_set_video_refresh(VideoFrame);
  m_dll.retro_set_audio_sample(AudioSample);
  m_dll.retro_set_audio_sample_batch(AudioSampleBatch);
  m_dll.retro_set_input_state(GetInputState);
  m_dll.retro_set_input_poll(NoopPoop);

  // TODO: Need an API call in libretro that lets us know the number of ports
  SetDevice(0, RETRO_DEVICE_JOYPAD);

  return true;
}

bool CGameClient::OpenInternal(const CFileItem& file)
{
  LIBRETRO::retro_game_info info;

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

    try
    {
      success = m_dll.retro_load_game(&info) && !CLibretroEnvironment::Abort();
    }
    catch (...)
    {
      success = false;
      CLog::Log(LOGERROR, "GAME: Exception thrown in retro_load_game()");
    }

    if (success)
    {
      CLog::Log(LOGINFO, "GAME: Client successfully loaded game");
      return true;
    }

    // If the user bailed and went to the game settings screen, the abort flag was set
    if (CLibretroEnvironment::Abort())
    {
      CLog::Log(LOGDEBUG, "GAME: Successfully loaded game, but CLibretroEnvironment aborted");
      break;
    }
    CLog::Log(LOGINFO, "GAME: Client failed to load game");
  }

  return false;

  /*
  bool ret, useMultipleRoms = false;
  if (!useMultipleRoms)
  {
    ret = m_dll.retro_load_game(&info);
  }
  else
  {
    // Special game types used when multiple roms are required
    unsigned int romType = RETRO_GAME_TYPE_BSX;
    //unsigned int romType = RETRO_GAME_TYPE_BSX_SLOTTED;
    //unsigned int romType = RETRO_GAME_TYPE_SUFAMI_TURBO;
    //unsigned int romType = RETRO_GAME_TYPE_SUPER_GAME_BOY;
    ret = m_dll.retro_load_game_special(romType, &info, 1);
  }
  */
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

void CGameClient::CloseFile()
{
  CSingleLock lock(m_critSection);

  if (m_dll.IsLoaded() && m_bIsPlaying)
  {
    m_dll.retro_unload_game();
    m_bIsPlaying = false;
  }

  m_callbacks = NULL;
  CLibretroEnvironment::ResetCallbacks();
  m_gameFile.Reset();
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

void CGameClient::SetPlatforms(const string &strPlatformList)
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

bool CGameClient::IsExtensionValid(const string &ext) const
{
  return CGameFileLoader::IsExtensionValid(ext, m_extensions);
}
