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
#include "addons/AddonManager.h"
#include "games/savegames/SavestateDatabase.h"
#include "libretro/LibretroEnvironment.h"
#include "settings/Settings.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"

using namespace ADDON;
using namespace GAME_INFO;
using namespace GAMES;
using namespace std;

namespace GAMES
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

ILibretroCallbacksAV *CGameClient::m_callbacks = NULL;

CGameClient::CGameClient(const AddonProps &props) : CAddon(props)
{
  Initialize();

  InfoMap::const_iterator it;
  if ((it = props.extrainfo.find("platforms")) != props.extrainfo.end())
    SetPlatforms(it->second);
  if ((it = props.extrainfo.find("extensions")) != props.extrainfo.end())
    SetExtensions(it->second);
  if ((it = props.extrainfo.find("allowvfs")) != props.extrainfo.end())
    m_config.bAllowVFS = (it->second == "true");
  if ((it = props.extrainfo.find("blockextract")) != props.extrainfo.end())
    m_config.bRequireZip = (it->second == "true");
}

CGameClient::CGameClient(const cp_extension_t *ext) : CAddon(ext)
{
  Initialize();

  if (!ext)
    return;

  CStdString platforms = CAddonMgr::Get().GetExtValue(ext->configuration, "platforms");
  if (!platforms.IsEmpty())
  {
    Props().extrainfo.insert(make_pair("platforms", platforms));
    SetPlatforms(platforms);
  }

  CStdString extensions = CAddonMgr::Get().GetExtValue(ext->configuration, "extensions");
  if (!extensions.IsEmpty())
  {
    Props().extrainfo.insert(make_pair("extensions", extensions));
    SetExtensions(extensions);
  }

  CStdString allowvfs = CAddonMgr::Get().GetExtValue(ext->configuration, "allowvfs");
  if (!allowvfs.IsEmpty())
  {
    Props().extrainfo.insert(make_pair("allowvfs", allowvfs));
    m_config.bAllowVFS = (allowvfs == "true");
  }

  CStdString blockextract = CAddonMgr::Get().GetExtValue(ext->configuration, "blockextract");
  if (!blockextract.IsEmpty())
  {
    Props().extrainfo.insert(make_pair("blockextract", blockextract));
    m_config.bRequireZip = (blockextract == "true");
  }

  // If library attribute isn't present, look for a system-dependent one
  if (m_strLibName.IsEmpty())
  {
#if defined(TARGET_ANDROID)
    m_strLibName = CAddonMgr::Get().GetExtValue(ext->configuration, "@library_android");
#elif defined(_LINUX) && !defined(TARGET_DARWIN)
    m_strLibName = CAddonMgr::Get().GetExtValue(ext->configuration, "@library_linux");
#elif defined(_WIN32) && defined(HAS_SDL_OPENGL)
    m_strLibName = CAddonMgr::Get().GetExtValue(ext->configuration, "@library_wingl");
#elif defined(_WIN32) && defined(HAS_DX)
    m_strLibName = CAddonMgr::Get().GetExtValue(ext->configuration, "@library_windx");
#elif defined(TARGET_DARWIN)
    m_strLibName = CAddonMgr::Get().GetExtValue(ext->configuration, "@library_osx");
#endif
  }
}

void CGameClient::Initialize()
{
  m_config = GameClientConfig(ID());
  m_callbacks = NULL;
  m_bIsInited = false;
  m_bIsPlaying = false;
  m_frameRate = 0.0;
  m_sampleRate = 0.0;
  m_region = -1; // invalid
  m_serialSize = 0;
  m_bRewindEnabled = false;
}

bool CGameClient::LoadSettings(bool bForce /* = false */)
{
  if (m_settingsLoaded && !bForce)
    return true;

  TiXmlElement category("category");
  category.SetAttribute("label", "15025"); // Emulator setup
  m_addonXmlDoc.InsertEndChild(category);

  TiXmlElement systemdir("setting");
  systemdir.SetAttribute("id", "systemdirectory");
  systemdir.SetAttribute("label", "15026"); // External system directory
  systemdir.SetAttribute("type", "folder");
  systemdir.SetAttribute("default", "");
  m_addonXmlDoc.RootElement()->InsertEndChild(systemdir);

  // Whether system directory setting is visible in Game Settings (GUIWindowSettingsCategory)
  // Setting is made visible the first time the game client asks for it
  TiXmlElement gamesettings("setting");
  gamesettings.SetAttribute("id", "hassystemdirectory");
  gamesettings.SetAttribute("type", "bool");
  gamesettings.SetAttribute("visible", "false"); // don't show the setting in GUIDialogAddonSettings
  gamesettings.SetAttribute("default", "false");
  m_addonXmlDoc.RootElement()->InsertEndChild(gamesettings);

  SettingsFromXML(m_addonXmlDoc, true);
  LoadUserSettings();
  m_settingsLoaded = true;
  return true;
}

void CGameClient::SaveSettings()
{
  // Avoid creating unnecessary files. This will skip creating the user save
  // xml unless a value has deviated from a default, or a previous user save
  // xml was loaded. Once a user save xml is created all saves will then
  // succeed.
  //
  // The desired result here is that the user save xml only exists after
  // RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY is first called in
  // CLibretroEnvironment (or is modified by the user).
  if (HasUserSettings() || GetSetting("hassystemdirectory") == "true" || GetSetting("systemdirectory") != "")
    CAddon::SaveSettings();
}

CStdString CGameClient::GetString(uint32_t id)
{
  // No special string handling for game clients
  return g_localizeStrings.Get(id);
}

bool CGameClient::Init()
{
  DeInit();

  m_dll.SetFile(LibPath());
  m_dll.EnableDelayedUnload(false);
  if (!m_dll.Load())
  {
    CLog::Log(LOGERROR, "GameClient: Error loading DLL %s", LibPath().c_str());
    return false;
  }

  LIBRETRO::retro_system_info info = { };
  m_dll.retro_get_system_info(&info);
  m_clientName    = info.library_name ? info.library_name : "Unknown";
  m_clientVersion = info.library_version ? info.library_version : "v0.0";
  bool allowVFS   = !info.need_fullpath;
  bool requireZip = info.block_extract;

  set<string> extensions;
  toExtensionSet(info.valid_extensions ? info.valid_extensions : "", extensions);

  CLog::Log(LOGINFO, "GameClient: Loaded %s core at version %s", m_clientName.c_str(), m_clientVersion.c_str());

  // Verify the DLL's reported values match those in addon.xml
  // This is to catch any mistakes uploaded to add-on repos
  bool success = true;
  if (m_config.bAllowVFS != allowVFS)
  {
    CLog::Log(LOGERROR, "GameClient: <allowvfs> tag in addon.xml doesn't match DLL value (%s)",
        allowVFS ? "true" : "false");
    success = false;
  }
  if (m_config.bRequireZip != requireZip)
  {
    CLog::Log(LOGERROR, "GameClient: <blockextract> tag in addon.xml doesn't match DLL value (%s)",
        requireZip ? "true" : "false");
    success = false;
  }
  if (m_config.extensions != extensions) // != operator defined above
  {
    CLog::Log(LOGERROR, "GameClient: <extensions> tag in addon.xml doesn't match the set from DLL value (%s)",
        info.valid_extensions ? info.valid_extensions : "");
    success = false;
  }

  // Verify API versions
  if (m_dll.retro_api_version() != RETRO_API_VERSION)
  {
    CLog::Log(LOGERROR, "GameClient: API version error: XBMC is at version %d, %s is at version %d",
        RETRO_API_VERSION, m_clientName.c_str(), m_dll.retro_api_version());
    success = false;
  }

  if (!success)
  {
    DeInit();
    return false;
  }

  CLog::Log(LOGINFO, "GameClient: ------------------------------------");
  CLog::Log(LOGINFO, "GameClient: Loaded DLL for %s", ID().c_str());
  CLog::Log(LOGINFO, "GameClient: Client: %s at version %s", m_clientName.c_str(), m_clientVersion.c_str());
  CLog::Log(LOGINFO, "GameClient: Valid extensions: %s", info.valid_extensions ? info.valid_extensions : "-");
  CLog::Log(LOGINFO, "GameClient: Allow VFS: %s, require zip (block extract): %s", m_config.bAllowVFS ? "yes" : "no",
      m_config.bRequireZip ? "yes" : "no");
  CLog::Log(LOGINFO, "GameClient: ------------------------------------");

  return true;
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
      CLog::Log(LOGERROR, "GameClient: Error unloading DLL: %s", e.what());
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
  return CGameFileLoader::CanOpen(file, m_config);
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
    CLog::Log(LOGERROR, "GameClient: File has \"gameclient\" property set, but it doesn't match mine!");
    return false;
  }

  m_callbacks = callbacks;

  // Ensure the default values
  m_callbacks->SetPixelFormat(LIBRETRO::RETRO_PIXEL_FORMAT_0RGB1555);
  m_callbacks->SetKeyboardCallback(NULL);

  // Install the hooks. These are called by CLibretroEnvironment::EnvironmentCallback()
  CLibretroEnvironment::SetDLLCallbacks(this, GameClientPtr(new CGameClient(Props())));

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
    if (strategy[i]->CanLoad(m_config, file) && strategy[i]->GetGameInfo(info))
    {
      // Use the path of the discovered game file UNLESS CGameFileLoaderUseParentZip
      // was chosen. We don't want to CRC a .zip file (for e.g. save states) if we
      // can help it.
      if (URIUtils::GetExtension(info.path).Equals(".zip") && !URIUtils::GetExtension(file.GetPath()).Equals(".zip"))
        m_gamePath = file.GetPath();
      else
        m_gamePath = info.path;

      // If info.data is set, then info.path must be set back to NULL so that
      // the game client is forced to load from memory
      if (info.data)
        info.path = NULL;

      try
      {
        success = m_dll.retro_load_game(&info) && !CLibretroEnvironment::Abort();
      }
      catch (...)
      {
        success = false;
        CLog::Log(LOGERROR, "GameClient error: exception thrown in retro_load_game()");
      }

      if (success)
        CLog::Log(LOGINFO, "GameClient: Client successfully loaded game");
      else
      {
        delete[] reinterpret_cast<const uint8_t*>(info.data);
        info.data = NULL;

        // If the user bailed and went to the game settings screen, the abort bit was set
        if (CLibretroEnvironment::Abort())
        {
          CLog::Log(LOGDEBUG, "GameClient: successfully loaded game, but CLibretroEnvironment aborted");
          return false;
        }
        else
          CLog::Log(LOGINFO, "GameClient: Client failed to load game");
      }
    }
  }

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

  if (!success)
    return false;

  m_bIsPlaying = true;

  // Get information about system audio/video timings and geometry
  // Can be called only after retro_load_game()
  LIBRETRO::retro_system_av_info av_info = {};
  m_dll.retro_get_system_av_info(&av_info);

  unsigned int baseWidth  = av_info.geometry.base_width; // 256
  unsigned int baseHeight = av_info.geometry.base_height; // 224
  unsigned int maxWidth   = av_info.geometry.max_width; // 512
  unsigned int maxHeight  = av_info.geometry.max_height; // 448
  float aspectRatio       = av_info.geometry.aspect_ratio; // 0.0
  double fps              = av_info.timing.fps; // 60.098811862348406
  double sampleRate       = av_info.timing.sample_rate; // 32040.5 or 31997.222656

  CLog::Log(LOGINFO, "GameClient: ---------------------------------------");
  CLog::Log(LOGINFO, "GameClient: Opened file %s", m_gamePath.c_str());
  CLog::Log(LOGINFO, "GameClient: Base Width: %u", baseWidth);
  CLog::Log(LOGINFO, "GameClient: Base Height: %u", baseHeight);
  CLog::Log(LOGINFO, "GameClient: Max Width: %u", maxWidth);
  CLog::Log(LOGINFO, "GameClient: Max Height: %u", maxHeight);
  CLog::Log(LOGINFO, "GameClient: Aspect Ratio: %f", aspectRatio);
  CLog::Log(LOGINFO, "GameClient: FPS: %f", fps);
  CLog::Log(LOGINFO, "GameClient: Sample Rate: %f", sampleRate);
  CLog::Log(LOGINFO, "GameClient: ---------------------------------------");

  m_frameRate = fps;
  m_sampleRate = sampleRate;

  // Check if save states are supported, so savestates and rewind can be used.
  m_serialSize = m_dll.retro_serialize_size();
  m_bRewindEnabled = CSettings::Get().GetBool("gamesgeneral.enablerewind");

  if (!InitSaveState(info.data, info.size))
  {
    m_serialSize = 0;
    m_bRewindEnabled = false;
  }
  else if (!m_serialSize)
  {
    CLog::Log(LOGINFO, "GameClient: Game serialization not supported, continuing without save or rewind");
    m_bRewindEnabled = false;
  }
  else
  {
    // Load savestate if possible
    bool loadSuccess = false;

    if (!file.m_startSaveState.empty())
      loadSuccess = Load(file.m_startSaveState);
    else if (CSettings::Get().GetBool("gamesgeneral.savestates"))
      loadSuccess = AutoLoad();

    if (!loadSuccess && m_bRewindEnabled)
    {
      CLog::Log(LOGDEBUG, "GameClient: Failed to load last savestate, forcing rewind to off");
      m_bRewindEnabled = false;
    }

    // Set up rewind functionality
    if (m_bRewindEnabled)
    {
      m_serialState.Init(m_serialSize, (size_t)(CSettings::Get().GetInt("gamesgeneral.rewindtime") * m_frameRate));

      if (m_dll.retro_serialize(m_serialState.GetState(), m_serialState.GetFrameSize()))
        CLog::Log(LOGDEBUG, "GameClient: Rewind is enabled");
      else
      {
        m_bRewindEnabled = false;
        m_serialState.Reset();
        CLog::Log(LOGDEBUG, "GameClient: Unable to serialize state, proceeding without rewind");
      }
    }
  }

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

  // TODO: Use CGameInfoTagLoader::GetPlatformByName(file.GetInfoTag().GetPlatform()).ports
  // or CGameInfoTagLoader::GetPlatformByExtension(URIUtils::GetExtensions(file.GetPath())).ports
  // if GamePlatform != PLATFORM_UNKNOWN or PLATFORM_AMBIGUOUS.
  // Need an API call in libretro that lets us know the number of ports
  SetDevice(0, RETRO_DEVICE_JOYPAD);

  // Hopefully the game client assumed its own copy of the game data
  //delete[] reinterpret_cast<const uint8_t*>(info.data); // TODO
  info.data = NULL;

  return true;
}

void CGameClient::CloseFile()
{
  CSingleLock lock(m_critSection);

  if (m_dll.IsLoaded() && m_bIsPlaying)
  {
    if (CSettings::Get().GetBool("gamesgeneral.savestates"))
      AutoSave();
    m_dll.retro_unload_game();
    m_bIsPlaying = false;
  }
  m_callbacks = NULL;
  m_gamePath.clear();
  m_saveState.Reset();
  CLibretroEnvironment::ResetCallbacks();
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

    m_saveState.SetPlaytimeFrames(m_saveState.GetPlaytimeFrames() + 1);
    m_saveState.SetPlaytimeWallClock(m_saveState.GetPlaytimeWallClock() + 1.0 / m_frameRate);

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

bool CGameClient::InitSaveState(const void *gameBuffer /* = NULL */, size_t length /* = 0 */)
{
  // Reset the database ID. This makes sure adding a new record doesn't erase an old one
  m_saveState.SetDatabaseId(-1);
  m_saveState.SetGameClient(ID());
  m_saveState.SetGamePath(m_gamePath);

  if (m_saveState.GetGameCRC().empty())
  {
    // Check the database for game CRC first
    CSavestateDatabase db;
    CStdString strCrc;
    if (db.Open() && db.GetCRC(m_gamePath, strCrc))
    {
      m_saveState.SetGameCRC(strCrc);
    }
    else
    {
      // Path and/or CRC not in database, calculate the game CRC now
      CLog::Log(LOGDEBUG, "GameClient: %s not in database, calculating CRC", URIUtils::GetFileName(m_gamePath).c_str());
      if (gameBuffer)
      {
        // If the length is too great, fall back to CRCing the file path
        if (length > MAX_SAVESTATE_CRC_LENGTH)
          m_saveState.SetGameCRCFromFile(m_gamePath, true);
        else
          m_saveState.SetGameCRCFromFile(reinterpret_cast<const char*>(gameBuffer), length);
      }
      else
        m_saveState.SetGameCRCFromFile(m_gamePath);

      // If we got a CRC, make the database aware of it
      if ((db.IsOpen() || db.Open()) && !m_saveState.GetGameCRC().empty())
      {
        CLog::Log(LOGDEBUG, "GameClient: Updating database with CRC %s", m_saveState.GetGameCRC().c_str());
        db.UpdateCRC(m_gamePath, m_saveState.GetGameCRC());
      }
    }
  }

  if (m_saveState.GetGameCRC().empty())
  {
    CLog::Log(LOGERROR, "GameClient: Failed to calculate CRC for %s", URIUtils::GetFileName(m_gamePath).c_str());
    return false;
  }
  return true;
}

bool CGameClient::AutoLoad()
{
  CSingleLock lock(m_critSection);

  if (!m_bIsPlaying)
    return false; // libretro DLL would probably crash
  CLog::Log(LOGINFO, "GameClient: Auto-loading last save state");
  if (!InitSaveState())
    return false;
  m_saveState.SetSaveTypeAuto();
  return Load();
}

bool CGameClient::Load(unsigned int slot)
{
  CSingleLock lock(m_critSection);

  if (!m_bIsPlaying)
    return false; // libretro DLL would probably crash
  CLog::Log(LOGINFO, "GameClient: Loading save state from slot %u", slot);
  if (!InitSaveState())
    return false;
  m_saveState.SetSaveTypeSlot(slot);
  return Load();
}

bool CGameClient::Load(const CStdString &saveStatePath)
{
  CSingleLock lock(m_critSection);

  if (!m_bIsPlaying)
    return false; // libretro DLL would probably crash
  CLog::Log(LOGINFO, "GameClient: Loading save state %s", saveStatePath.c_str());
  m_saveState.SetPath(saveStatePath);
  return Load();
}

bool CGameClient::Load()
{
  // Load the savestate into a copy, and assign the copy into our member
  // variable if the deserialization succeeds
  CSavestate savestate(m_saveState);
  vector<uint8_t> data;

  CSavestateDatabase db;
  if (db.Open() && db.Load(savestate, data))
  {
    if (!m_dll.retro_unserialize(data.data(), data.size()))
    {
      CLog::Log(LOGERROR, "GameClient: Libretro core failed to de-serialize data!");
      return false;
    }
    // Reset rewind buffer if rewinding is enabled
    if (m_bRewindEnabled && m_serialState.IsInited())
    {
      m_serialState.Init(data.size(), (size_t)(CSettings::Get().GetInt("gamesgeneral.rewindtime") * m_frameRate));
      memcpy(m_serialState.GetState(), data.data(), data.size());
    }
    m_saveState = savestate;
  }
  else
  {
    CLog::Log(LOGDEBUG, "GameClient: Failed to read save state or load from database");
  }
  // Return true even if Read() failed, because the next call the Save() will probably succeed
  return true;
}

bool CGameClient::AutoSave()
{
  CSingleLock lock(m_critSection);

  if (!m_bIsPlaying)
    return false;
  CLog::Log(LOGINFO, "GameClient: Auto-save");
  if (!InitSaveState())
    return false;
  m_saveState.SetSaveTypeAuto();
  return Save();
}

bool CGameClient::Save(unsigned int slot)
{
  CSingleLock lock(m_critSection);

  if (!m_bIsPlaying)
    return false;
  CLog::Log(LOGINFO, "GameClient: Saving state to slot %u", slot);
  if (!InitSaveState())
    return false;

  // Avoid duplicate labels. If saving to "Slot 2", and a manual save is
  // labeled "Slot 2", delete the manual label.
  m_saveState.SetSaveTypeSlot(slot); // Generate temporary slot label
  m_saveState.SetSaveTypeLabel(m_saveState.GetLabel());
  CSavestateDatabase db;
  if (db.Open())
    db.DeleteSaveState(m_saveState.GetPath(), false);

  m_saveState.SetSaveTypeSlot(slot);
  return Save();
}

bool CGameClient::Save(const CStdString &label)
{
  CSingleLock lock(m_critSection);

  if (!m_bIsPlaying)
    return false;
  CLog::Log(LOGINFO, "GameClient: Saving state with label %s", label.c_str());
  if (!InitSaveState())
    return false;
  m_saveState.SetSaveTypeLabel(label);
  return Save();
}

bool CGameClient::Save()
{
  // Prefer serialized states to avoid any game client serialization procedures
  if (m_bRewindEnabled && m_serialState.GetFrameSize())
  {
    m_savestateBuffer.resize(m_serialState.GetFrameSize());
    memcpy(m_savestateBuffer.data(), m_serialState.GetState(), m_serialState.GetFrameSize());
  }
  else
  {
    if (!m_serialSize)
      return false;
    m_savestateBuffer.resize(m_serialSize);
    if (!m_dll.retro_serialize(m_savestateBuffer.data(), m_savestateBuffer.size()))
      return false;
  }

  CSavestateDatabase db;
  return db.Open() && db.Save(m_saveState, m_savestateBuffer);
}

unsigned int CGameClient::RewindFrames(unsigned int frames)
{
  CSingleLock lock(m_critSection);

  unsigned int rewound = 0;
  if (m_bIsPlaying && m_bRewindEnabled)
  {
    rewound = m_serialState.RewindFrames(frames);
    if (rewound && m_dll.retro_unserialize(m_serialState.GetState(), m_serialState.GetFrameSize()))
    {
      // We calculate these separately because they can actually diverge, as
      // the framerate is possibly variable and can depend on the chosen audio
      // samplerate (I'm not sure how likely this is however)
      uint64_t frames = m_saveState.GetPlaytimeFrames();
      m_saveState.SetPlaytimeFrames(frames > rewound ? frames - rewound : 0);

      double wallclock = m_saveState.GetPlaytimeWallClock();
      m_saveState.SetPlaytimeWallClock(wallclock > rewound / m_frameRate ? wallclock - rewound / m_frameRate : 0.0);
    }
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

    InitSaveState();
    m_saveState.SetPlaytimeFrames(0);
    m_saveState.SetPlaytimeWallClock(0.0);

    if (m_bRewindEnabled)
    {
      m_serialState.Init(m_serialState.GetFrameSize(), m_serialState.GetMaxFrames());
      if (!m_dll.retro_serialize(m_serialState.GetState(), m_serialState.GetFrameSize()))
      {
        m_bRewindEnabled = false;
        CLog::Log(LOGINFO, "GameClient::Reset - Unable to serialize state, proceeding without rewind");
      }
    }
  }
}

void CGameClient::SetFrameRate(double framerate)
{
  m_frameRate = framerate;
  if (m_bRewindEnabled)
    m_serialState.SetMaxFrames((size_t)(CSettings::Get().GetInt("gamesgeneral.rewindtime") * m_frameRate));
}

void CGameClient::SetExtensions(const string &strExtensionList)
{
  // If no extensions are provided, don't erase the ones we are already tracking
  if (strExtensionList.empty())
    return;

  toExtensionSet(strExtensionList, m_config.extensions);
}

void CGameClient::SetPlatforms(const string &strPlatformList)
{
  // If no platforms are provided, don't erase the ones we are already tracking
  if (strPlatformList.empty())
    return;

  m_config.platforms.clear();
  vector<string> platforms = StringUtils::Split(strPlatformList, "|");
  for (vector<string>::iterator it = platforms.begin(); it != platforms.end(); it++)
  {
    StringUtils::Trim(*it);
    GamePlatform id = CGameInfoTagLoader::GetPlatformByName(*it).id;
    if (id != PLATFORM_UNKNOWN)
      m_config.platforms.push_back(id);
  }
}

bool CGameClient::IsExtensionValid(const string &ext) const
{
  return CGameFileLoader::IsExtensionValid(ext, m_config.extensions);
}

void CGameClient::SetPixelFormat(LIBRETRO::retro_pixel_format format)
{
  if (m_callbacks)
    m_callbacks->SetPixelFormat(format);
}

void CGameClient::SetKeyboardCallback(LIBRETRO::retro_keyboard_event_t callback)
{
  if (m_callbacks)
    m_callbacks->SetKeyboardCallback(callback);
}

/* static */
void CGameClient::VideoFrame(const void *data, unsigned width, unsigned height, size_t pitch)
{
  if (m_callbacks)
    m_callbacks->VideoFrame(data, width, height, pitch);
}

/* static */
void CGameClient::AudioSample(int16_t left, int16_t right)
{
  if (m_callbacks)
    m_callbacks->AudioSample(left, right);
}

/* static */
size_t CGameClient::AudioSampleBatch(const int16_t *data, size_t frames)
{
  if (m_callbacks)
    return m_callbacks->AudioSampleBatch(data, frames);
  return frames;
}

/* static */
int16_t CGameClient::GetInputState(unsigned port, unsigned device, unsigned index, unsigned id)
{
  if (m_callbacks)
    return m_callbacks->GetInputState(port, device, index, id);
  return 0;
}
