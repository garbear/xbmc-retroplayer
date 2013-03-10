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

#include "GameClient.h"
#include "addons/AddonManager.h"
#include "Application.h"
#include "filesystem/File.h"
#include "filesystem/Directory.h"
#include "settings/GUISettings.h"
#include "threads/SingleLock.h"
#include "URL.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

#include <limits>

using namespace ADDON;
using namespace GAME_INFO;
using namespace XFILE;


bool CGameClient::IRetroStrategy::GetGameInfo(retro_game_info &info) const
{
  info.path = NULL;
  info.data = NULL;
  info.size = 0;
  info.meta = NULL;

  if (!m_useVfs)
  {
    info.path = m_path.c_str();
    CLog::Log(LOGINFO, "GameClient: Strategy is valid, client is loading file %s", info.path);
  }
  else
  {
    unsigned char *data;
    int64_t length;

    // Load the file from the vfs
    CFile vfsFile;
    if (!vfsFile.Open(m_path))
    {
      CLog::Log(LOGERROR, "GameClient::CStrategyUseVFS: XBMC cannot open file");
      return false; // XBMC can't load it, don't expect the game client to
    }

    length = vfsFile.GetLength();

    // Check for file size overflow (libretro accepts files <= size_t max)
    if (length <= 0 || length >= std::numeric_limits<size_t>::max())
    {
      CLog::Log(LOGERROR, "GameClient: Invalid file size: %"PRId64" bytes", length);
      return false;
    }

    data = new unsigned char[(size_t)length];

    // Verify the allocation and read in the data
    if (!(data && vfsFile.Read(data, length) == length))
    {
      CLog::Log(LOGERROR, "GameClient: XBMC failed to read game data");
      delete[] data;
      return false;
    }

    info.data = data;
    info.size = (size_t)length;
    CLog::Log(LOGINFO, "GameClient: Strategy is valid, client is loading file from VFS (filesize: %lu KB)", info.size);
  }
  return true;
}

bool CGameClient::CStrategyUseHD::CanLoad(const GameClientConfig &gc, const CFileItem& file)
{
  CLog::Log(LOGINFO, "GameClient::CStrategyUseHD: Testing if we can load game from hard drive");

  // Make sure the file is local
  if (!file.GetAsUrl().GetProtocol().empty())
  {
    CLog::Log(LOGINFO, "GameClient::CStrategyUseHD: File is not local (or is inside an archive)");
    return false;
  }

  // Make sure the extension is valid
  if (!IsExtensionValid(URIUtils::GetExtension(file.GetPath()), gc.extensions))
  {
    CLog::Log(LOGINFO, "GameClient::CStrategyUseHD: Extension %s is not valid", URIUtils::GetExtension(file.GetPath()).c_str());
    return false;
  }

  m_path = file.GetPath();
  m_useVfs = false;
  return true;
}

bool CGameClient::CStrategyUseVFS::CanLoad(const GameClientConfig &gc, const CFileItem& file)
{
  CLog::Log(LOGINFO, "GameClient::CStrategyUseVFS: Testing if we can load game from VFS");

  // Obvious check
  if (!gc.bAllowVFS)
  {
    CLog::Log(LOGINFO, "GameClient::CStrategyUseVFS: Game client does not allow VFS");
    return false;
  }

  // Make sure the extension is valid
  CStdString ext = URIUtils::GetExtension(file.GetPath());
  if (!IsExtensionValid(ext, gc.extensions))
  {
    CLog::Log(LOGINFO, "GameClient::CStrategyUseVFS: Extension %s is not valid", ext.c_str());
    return false;
  }

  m_path = file.GetPath();
  m_useVfs = true;
  return true;
}

bool CGameClient::CStrategyUseParentZip::CanLoad(const GameClientConfig &gc, const CFileItem& file)
{
  CLog::Log(LOGINFO, "GameClient::CStrategyUseParentZip: Testing if the game is in a zip");

  // Can't use parent zip if file isn't a child file of a .zip folder
  if (!URIUtils::IsInZIP(file.GetPath()))
  {
    CLog::Log(LOGINFO, "GameClient::CStrategyUseParentZip: Game is not in a zip file");
    return false;
  }

  if (!IsExtensionValid(".zip", gc.extensions))
  {
    CLog::Log(LOGINFO, "GameClient::CStrategyUseParentZip: This game client does not support zip files");
    return false;
  }

  // Make sure we're in the root folder of the zip (no parent folder)
  CURL parentURL(URIUtils::GetParentPath(file.GetPath()));
  if (!parentURL.GetFileName().empty())
  {
    CLog::Log(LOGINFO, "GameClient::CStrategyUseParentZip: Game is not in the root folder of the zip");
    return false;
  }

  // Make sure the container zip is on the local hard disk (or not inside another zip)
  if (!CURL(parentURL.GetHostName()).GetProtocol().empty())
  {
    CLog::Log(LOGINFO, "GameClient::CStrategyUseParentZip: Zip file is not on the local hard disk");
    return false;
  }

  // Make sure the extension is valid
  if (!IsExtensionValid(URIUtils::GetExtension(file.GetPath()), gc.extensions))
  {
    CLog::Log(LOGINFO, "GameClient::CStrategyUseParentZip: Extension %s is not valid", URIUtils::GetExtension(file.GetPath()).c_str());
    return false;
  }

  // Found our file
  m_path = parentURL.GetHostName();
  m_useVfs = false;
  return true;
}

bool CGameClient::CStrategyEnterZip::CanLoad(const GameClientConfig &gc, const CFileItem& file)
{
  CLog::Log(LOGINFO, "GameClient::CStrategyEnterZip: Testing if the file is a zip containing a game");

  // Must be a zip file, clearly
  if (!URIUtils::GetExtension(file.GetPath()).Equals(".zip"))
  {
    CLog::Log(LOGINFO, "GameClient::CStrategyEnterZip: File is not a zip");
    return false;
  }

  // Must support loading from the vfs
  if (!gc.bAllowVFS)
  {
    CLog::Log(LOGINFO, "GameClient::CStrategyEnterZip: Game client does not allow VFS");
    return false;
  }

  // Look for an internal file. This will screen against valid extensions.
  CStdString internalFile;
  if (!GetEffectiveRomPath(file.GetPath(), gc.extensions, internalFile))
  {
    CLog::Log(LOGINFO, "GameClient::CStrategyEnterZip: Zip does not contain a file with a valid extension");
    return false;
  }

  m_path = internalFile;
  m_useVfs = true;
  return true;
}


CGameClient::DataReceiver::SetPixelFormat_t       CGameClient::_SetPixelFormat      = NULL;
CGameClient::DataReceiver::SetKeyboardCallback_t  CGameClient::_SetKeyboardCallback = NULL;

/* static */
bool CGameClient::GetEffectiveRomPath(const CStdString &zipPath, const std::set<CStdString> &validExts, CStdString &effectivePath)
{
  // Default case: effective zip file is the zip file itself
  effectivePath = zipPath;

  // If it's not a zip file, we can't open and explore...
  if (!URIUtils::GetExtension(zipPath).Equals(".zip"))
    return false;

  // Enumerate the zip directory, looking for valid extensions
  CStdString strUrl;
  URIUtils::CreateArchivePath(strUrl, "zip", zipPath, "");

  CStdString strValidExts;
  for (std::set<CStdString>::const_iterator it = validExts.begin(); it != validExts.end(); it++)
    strValidExts += *it + "|";

  CFileItemList itemList;
  if (CDirectory::GetDirectory(strUrl, itemList, strValidExts, DIR_FLAG_READ_CACHE | DIR_FLAG_NO_FILE_INFO) && itemList.Size())
  {
    // Use the first file discovered
    effectivePath = itemList[0]->GetPath();
    return true;
  }
  return false;
}

/* static */
bool CGameClient::IsExtensionValid(const CStdString &ext, const std::set<CStdString> &setExts)
{
  if (setExts.empty())
    return true; // Be optimistic :)
  if (ext.empty())
    return false;
  CStdString ext2(ext);
  ext2.ToLower();
  if (ext2.at(0) != '.')
    ext2 = "." + ext2;
  return setExts.find(ext2) != setExts.end();
}

CGameClient::CGameClient(const AddonProps &props) : CAddon(props)
{
  Initialize();

  InfoMap::const_iterator it;
  if ((it = props.extrainfo.find("platforms")) != props.extrainfo.end())
    SetPlatforms(it->second);
  if ((it = props.extrainfo.find("extensions")) != props.extrainfo.end())
    SetExtensions(it->second);
}

CGameClient::CGameClient(const cp_extension_t *ext) : CAddon(ext)
{
  Initialize();

  if (!ext)
    return;

  CStdString platforms;
  platforms = CAddonMgr::Get().GetExtValue(ext->configuration, "platforms");
  if (!platforms.IsEmpty())
  {
    Props().extrainfo.insert(make_pair("platforms", platforms));
    SetPlatforms(platforms);
  }

  CStdString extensions;
  extensions = CAddonMgr::Get().GetExtValue(ext->configuration, "extensions");
  if (!extensions.IsEmpty())
  {
    Props().extrainfo.insert(make_pair("extensions", extensions));
    SetExtensions(extensions);
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
  m_config = GameClientConfig();
  m_config.id = ID();
  m_bIsInited = false;
  m_bIsPlaying = false;
  m_frameRate = 0.0;
  m_sampleRate = 0.0;
  m_region = -1; // invalid
  m_rewindSupported = false;
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

  retro_system_info info = { };
  m_dll.retro_get_system_info(&info);
  m_clientName         = info.library_name ? info.library_name : "Unknown";
  m_clientVersion      = info.library_version ? info.library_version : "v0.0";
  m_config.bAllowVFS   = !info.need_fullpath;
  m_config.bRequireZip = info.block_extract;
  SetExtensions(info.valid_extensions ? info.valid_extensions : "");
  CLog::Log(LOGINFO, "GameClient: Loaded %s core at version %s", m_clientName.c_str(), m_clientVersion.c_str());

  // Verify API versions
  if (m_dll.retro_api_version() != RETRO_API_VERSION)
  {
    CLog::Log(LOGERROR, "GameClient: API version error: XBMC is at version %d, %s is at version %d", RETRO_API_VERSION, m_clientName.c_str(), m_dll.retro_api_version());
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
    if (m_bIsPlaying)
    {
      m_dll.retro_unload_game();
      m_bIsPlaying = false;
    }
    if (m_bIsInited)
    {
      m_dll.retro_deinit();
      m_bIsInited = false;
    }
    try
    {
      m_dll.Unload();
    }
    catch (std::exception &e)
    {
      CLog::Log(LOGERROR, "GameClient: Error unloading DLL: %s", e.what());
    }
  }
}

/* static */
void CGameClient::GetStrategy(CStrategyUseHD &hd, CStrategyUseParentZip &outerzip,
      CStrategyUseVFS &vfs, CStrategyEnterZip &innerzip, IRetroStrategy *strategies[4])
{
  if (!g_guiSettings.GetBool("gamesdebug.prefervfs"))
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

/* static */
bool CGameClient::CanOpen(const CFileItem &file, const GameClientConfig &config, bool useStrategies /* = false */)
{
  // Check gameclient
  if (!file.GetProperty("gameclient").empty() && file.GetProperty("gameclient").asString() != config.id)
    return false;

  if (config.extensions.empty() && config.platforms.empty())
    return true; // Client provided us with *no* useful information. Be optimistic.

  // Check platform
  if (!config.platforms.empty() && file.GetGameInfoTag())
  {
    GamePlatform id = CGameInfoTagLoader::GetPlatformByName(file.GetGameInfoTag()->GetPlatform()).id;
    if (id != PLATFORM_UNKNOWN)
      if (std::find(config.platforms.begin(), config.platforms.end(), id) == config.platforms.end())
        return false;
  }

  // If we don't do thorough screening, at least check inside the zip for valid files
  if (!useStrategies)
  {
    // Get the file extension (we want .zip if the file is a top-level zip directory)
    CStdString ext(URIUtils::GetExtension(file.GetAsUrl().GetFileNameWithoutPath()));
    if (ext == ".zip")
    {
      // If .zip is not valid, see if there is a file inside that is
      CStdString path2;
      return IsExtensionValid(ext, config.extensions) || GetEffectiveRomPath(file.GetPath(), config.extensions, path2);
    }
    // If the game client lists extensions, check those as well
    return config.extensions.empty() || config.extensions.find(ext) != config.extensions.end();
  }
  else
  {
    CStrategyUseHD        hd;
    CStrategyUseParentZip outerzip;
    CStrategyUseVFS       vfs;
    CStrategyEnterZip     innerzip;

    IRetroStrategy *strategy[4] = { &hd, &outerzip, &vfs, &innerzip };

    for (unsigned int i = 0; i < sizeof(strategy) / sizeof(strategy[0]); i++)
      if (strategy[i]->CanLoad(config, file))
        return true;
    return false;
  }
}

bool CGameClient::OpenFile(const CFileItem& file, const DataReceiver &callbacks)
{
  // Can't open a file without first initializing the DLL...
  if (!m_dll.IsLoaded())
    Init();

  if (!file.GetProperty("gameclient").empty() && file.GetProperty("gameclient").asString() != ID())
  {
    CLog::Log(LOGERROR, "GameClient: File has \"gameclient\" property set, but it doesn't match mine!");
    return false;
  }

  // Ensure the default values
  callbacks.SetPixelFormat(RETRO_PIXEL_FORMAT_0RGB1555);
  callbacks.SetKeyboardCallback(NULL);

  // Install the hooks. These are called by EnvironmentCallback().
  _SetPixelFormat      = callbacks.SetPixelFormat;
  _SetKeyboardCallback = callbacks.SetKeyboardCallback;

  // Because we call m_dll.retro_init() here instead of in Init(), keep track
  // of this. Note that if we return false later, m_bIsInited will still be
  // true because the DLL is still loaded and inited
  if (!m_bIsInited)
  {
    // Set up our callback to retrieve environment variables
    // retro_set_environment() must be called before retro_init()
    m_dll.retro_set_environment(EnvironmentCallback);
    m_dll.retro_init();
    m_bIsInited = true;
  }

  retro_game_info info;

  CStrategyUseHD        hd;
  CStrategyUseParentZip outerzip;
  CStrategyUseVFS       vfs;
  CStrategyEnterZip     innerzip;

  IRetroStrategy *strategy[4];
  GetStrategy(hd, outerzip, vfs, innerzip, strategy);

  bool success = false;
  for (unsigned int i = 0; i < sizeof(strategy) / sizeof(strategy[0]); i++)
  {
    if (strategy[i]->CanLoad(m_config, file) && strategy[i]->GetGameInfo(info))
    {
      if (m_dll.retro_load_game(&info))
      {
        CLog::Log(LOGINFO, "GameClient: Client successfully loaded game");
        success = true;
        break;
      }
      else
        CLog::Log(LOGINFO, "GameClient: Client failed to load game");
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
  retro_system_av_info av_info = {};
  m_dll.retro_get_system_av_info(&av_info);

  unsigned int baseWidth  = av_info.geometry.base_width; // 256
  unsigned int baseHeight = av_info.geometry.base_height; // 224
  unsigned int maxWidth   = av_info.geometry.max_width; // 512
  unsigned int maxHeight  = av_info.geometry.max_height; // 448
  float aspectRatio       = av_info.geometry.aspect_ratio; // 0.0
  double fps              = av_info.timing.fps; // 60.098811862348406
  double sampleRate       = av_info.timing.sample_rate; // 32040.5 or 31997.222656

  CLog::Log(LOGINFO, "GameClient: ---------------------------------------");
  CLog::Log(LOGINFO, "GameClient: Opened file %s", file.GetPath().c_str());
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

  // Check if save states are supported, so rewind can be used.
  size_t state_size = m_dll.retro_serialize_size();
  m_rewindSupported = state_size && g_guiSettings.GetBool("games.enablerewind");
  if (m_rewindSupported)
  {
    m_serialState.Init(state_size, (size_t)(g_guiSettings.GetInt("games.rewindtime") * m_frameRate));
    if (!m_dll.retro_serialize(m_serialState.GetState(), m_serialState.GetFrameSize()))
    {
      m_rewindSupported = false;
      CLog::Log(LOGINFO, "GameClient: Unable to serialize state, proceeding without rewind");
    }
    else
    {
      CLog::Log(LOGINFO, "GameClient: Rewind is enabled");

      // Load save and auto state
    }
  }
  else
  {
    CLog::Log(LOGINFO, "GameClient: Rewind support is not enabled");
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

  // Install callbacks
  m_dll.retro_set_video_refresh(callbacks.VideoFrame);
  m_dll.retro_set_audio_sample(callbacks.AudioSample);
  m_dll.retro_set_audio_sample_batch(callbacks.AudioSampleBatch);
  m_dll.retro_set_input_state(callbacks.GetInputState);
  m_dll.retro_set_input_poll(NoopPoop);

  // TODO: Use CGameInfoTagLoader::GetPlatformByName(file.GetInfoTag().GetPlatform()).ports
  // or CGameInfoTagLoader::GetPlatformByExtension(URIUtils::GetExtensions(file.GetPath())).ports
  // if GamePlatform != PLATFORM_UNKNOWN or PLATFORM_AMBIGUOUS.
  // Need an API call in libretro that lets us know the number of ports
  SetDevice(0, RETRO_DEVICE_JOYPAD);

  return true;
}

void CGameClient::CloseFile()
{
  CSingleLock lock(m_critSection);

  if (m_dll.IsLoaded() && m_bIsPlaying)
  {
    m_dll.retro_unload_game();
    m_bIsPlaying = false;
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

void CGameClient::RunFrame()
{
  if (m_bIsPlaying)
  {
    // RunFrame() and RewindFrames() can be run in different threads, must lock
    CSingleLock lock(m_critSection);

    m_dll.retro_run();

    // Append a new state delta to the rewind buffer
    if (m_rewindSupported)
    {
      if (m_dll.retro_serialize(m_serialState.GetNextState(), m_serialState.GetFrameSize()))
        m_serialState.AdvanceFrame();
      else
      {
        CLog::Log(LOGERROR, "GameClient core claimed it could serialize, but failed.");
        m_rewindSupported = false;
      }
    }
  }
}

unsigned int CGameClient::RewindFrames(unsigned int frames)
{
  unsigned int rewound = 0;
  if (m_bIsPlaying && m_rewindSupported)
  {
    CSingleLock lock(m_critSection);

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

    if (m_rewindSupported)
    {
      m_serialState.Init(m_serialState.GetFrameSize(), m_serialState.GetMaxFrames());
      if (!m_dll.retro_serialize(m_serialState.GetState(), m_serialState.GetFrameSize()))
      {
        m_rewindSupported = false;
        CLog::Log(LOGINFO, "GameClient::Reset - Unable to serialize state, proceeding without rewind");
      }
    }
  }
}

void CGameClient::SetExtensions(const CStdString &strExtensionList)
{
  // If no extensions are provided, don't erase the ones we are already tracking
  if (strExtensionList.empty())
    return;

  m_config.extensions.clear();
  CStdStringArray extensions;
  StringUtils::SplitString(strExtensionList, "|", extensions);
  for (CStdStringArray::const_iterator it = extensions.begin(); it != extensions.end(); it++)
  {
    CStdString ext(*it);
    if (ext.empty())
      continue;

    ext.ToLower();
    if (ext.at(0) != '.')
      ext = "." + ext;

    // Zip crashes some emulators, allow users to disable zips for problematic emulators
    if (ext == ".zip" && !g_guiSettings.GetBool("gamesdebug.allowzip"))
      continue;

    m_config.extensions.insert(ext);
  }
}

void CGameClient::SetPlatforms(const CStdString &strPlatformList)
{
  // If no platforms are provided, don't erase the ones we are already tracking
  if (strPlatformList.empty())
    return;

  m_config.platforms.clear();
  CStdStringArray platforms;
  StringUtils::SplitString(strPlatformList, "|", platforms);
  for (CStdStringArray::iterator it = platforms.begin(); it != platforms.end(); it++)
  {
    it->Trim();
    GamePlatform id = CGameInfoTagLoader::GetPlatformByName(*it).id;
    if (id != PLATFORM_UNKNOWN)
      m_config.platforms.push_back(id);
  }
}

/* static */
bool CGameClient::EnvironmentCallback(unsigned int cmd, void *data)
{
  // Note: SHUTDOWN doesn't use data and GET_SYSTEM_DIRECTORY uses data as a return path
  if (!(cmd == RETRO_ENVIRONMENT_SHUTDOWN || cmd == RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY) && !data)
  {
    CLog::Log(LOGERROR, "GameClient environment query ID=%d: no data! naughty core?", cmd);
    return false;
  }

  switch (cmd)
  {
  case RETRO_ENVIRONMENT_GET_OVERSCAN:
    {
      // Whether or not the game client should use overscan (true) or crop away overscan (false)
      *reinterpret_cast<bool*>(data) = false;
      CLog::Log(LOGINFO, "GameClient environment query ID=%d: %s", RETRO_ENVIRONMENT_GET_OVERSCAN,
          *reinterpret_cast<bool*>(data) ? "use overscan" : "crop away overscan");
      break;
    }
  case RETRO_ENVIRONMENT_GET_CAN_DUPE:
    {
      // Boolean value whether or not we support frame duping, passing NULL to video frame callback
      *reinterpret_cast<bool*>(data) = true;
      CLog::Log(LOGINFO, "GameClient environment query ID=%d: frame duping is %s",
          RETRO_ENVIRONMENT_GET_CAN_DUPE, *reinterpret_cast<bool*>(data) ? "enabled" : "disabled");
      break;
    }
  case RETRO_ENVIRONMENT_GET_VARIABLE:
    {
      // Interface to acquire user-defined information from environment that cannot feasibly be
      // supported in a multi-system way. Mostly used for obscure, specific features that the
      // user can tap into when necessary.
      retro_variable *var = reinterpret_cast<retro_variable*>(data);
      if (var->key && var->value)
      {
        // For example...
        if (strncmp("too_sexy_for", var->key, 12) == 0)
        {
          var->value = "my_shirt";
          CLog::Log(LOGINFO, "GameClient environment query ID=%d: variable %s set to %s",
              RETRO_ENVIRONMENT_GET_VARIABLE, var->key, var->value);
        }
        else
        {
          var->value = NULL;
          CLog::Log(LOGERROR, "GameClient environment query ID=%d: undefined variable %s",
              RETRO_ENVIRONMENT_GET_VARIABLE, var->key);
        }
      }
      else
      {
        if (var->value)
          var->value = NULL;
        CLog::Log(LOGERROR, "GameClient environment query ID=%d: no variable given",
            RETRO_ENVIRONMENT_GET_VARIABLE);
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_VARIABLES:
    {
      // Allows an implementation to signal the environment which variables it might want to check
      // for later using GET_VARIABLE. 'data' points to an array of retro_variable structs terminated
      // by a { NULL, NULL } element. retro_variable::value should contain a human readable description
      // of the key.
      const retro_variable *vars = reinterpret_cast<const retro_variable*>(data);
      if (!vars->key)
      {
        CLog::Log(LOGERROR, "GameClient environment query ID=%d: no variables given",
            RETRO_ENVIRONMENT_SET_VARIABLES);
      }
      else
      {
        while (vars && vars->key)
        {
          if (vars->value)
            CLog::Log(LOGINFO, "GameClient environment query ID=%d: notified of var %s (%s)",
              RETRO_ENVIRONMENT_SET_VARIABLES, vars->key, vars->value);
          else
            CLog::Log(LOGWARNING, "GameClient environment query ID=%d: var %s has no description",
              RETRO_ENVIRONMENT_SET_VARIABLES, vars->key);
          vars++;
        }
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_MESSAGE:
    {
      // Sets a message to be displayed. Generally not for trivial messages.
      const retro_message *msg = reinterpret_cast<const retro_message*>(data);
      if (msg->msg && msg->frames)
        CLog::Log(LOGINFO, "GameClient environment query ID=%d: display msg \"%s\" for %d frames",
            RETRO_ENVIRONMENT_SET_MESSAGE, msg->msg, msg->frames);
      break;
    }
  case RETRO_ENVIRONMENT_SET_ROTATION:
    {
      // Sets screen rotation of graphics. Valid values are 0, 1, 2, 3, which rotates screen
      // by 0, 90, 180, 270 degrees counter-clockwise respectively.
      unsigned int rotation = *reinterpret_cast<const unsigned int*>(data);
      if (0 <= rotation && rotation <= 3)
        CLog::Log(LOGINFO, "GameClient environment query ID=%d: set screen rotation to %d degrees",
            RETRO_ENVIRONMENT_SET_ROTATION, rotation * 90);
      else
        CLog::Log(LOGERROR, "GameClient environment query ID=%d: invalid rotation %d",
            RETRO_ENVIRONMENT_SET_ROTATION, rotation);
      break;
    }
  case RETRO_ENVIRONMENT_SHUTDOWN:
    // Game has been shut down. Should only be used if game has a specific way to shutdown
    // the game from a menu item or similar.
    CLog::Log(LOGINFO, "GameClient environment query ID=%d: game signaled shutdown event", RETRO_ENVIRONMENT_SHUTDOWN);

    if (g_application.IsPlayingGame())
      g_application.StopPlaying();

    break;
  case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
    {
      // Generally how computationally intense this core is, to gauge how capable XBMC's host system
      // will be for running the core. It can also be called on a game-specific basis. The levels
      // are "floating", but roughly defined as:
      // 0: Low-powered embedded devices such as Raspberry Pi
      // 1: Phones, tablets, 6th generation consoles, such as Wii/Xbox 1, etc
      // 2: 7th generation consoles, such as PS3/360, with sub-par CPUs.
      // 3: Modern desktop/laptops with reasonably powerful CPUs.
      // 4: High-end desktops with very powerful CPUs.
      unsigned int performanceLevel = *reinterpret_cast<const unsigned int*>(data);
      if (0 <= performanceLevel && performanceLevel <= 3)
        CLog::Log(LOGINFO, "GameClient environment query ID=%d: performance hint: %d",
            RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, performanceLevel);
      else if (performanceLevel == 4)
        CLog::Log(LOGINFO, "GameClient environment query ID=%d: performance hint: I hope you have a badass computer...",
            RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL);
      else
        CLog::Log(LOGERROR, "GameClient environment query ID=%d: invalid performance hint: %d",
            RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, performanceLevel);
      break;
    }
  case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    {
      // Returns a directory for storing system specific ROMs such as BIOSes, configuration data,
      // etc. The returned value can be NULL, in which case it's up to the implementation to find
      // a suitable directory.
      *reinterpret_cast<const char**>(data) = NULL;
      CLog::Log(LOGINFO, "GameClient environment query ID=%d: no system directory given to core",
          RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY);
      break;
    }
  case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
    {
      // Get the internal pixel format used by the core. The default pixel format is
      // RETRO_PIXEL_FORMAT_0RGB1555 (see libretro.h). Returning false lets the core
      // know that XBMC does not support the pixel format.
      retro_pixel_format pix_fmt = *reinterpret_cast<const retro_pixel_format*>(data);
      // Validate the format
      switch (pix_fmt)
      {
      case RETRO_PIXEL_FORMAT_0RGB1555: // 5 bit color, high bit must be zero
      case RETRO_PIXEL_FORMAT_XRGB8888: // 8 bit color, high byte is ignored
      case RETRO_PIXEL_FORMAT_RGB565:   // 5/6/5 bit color
        CLog::Log(LOGINFO, "GameClient environment query ID=%d: set pixel format: %d",
            RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, pix_fmt);
        _SetPixelFormat(pix_fmt);
        break;
      default:
        CLog::Log(LOGERROR, "GameClient environment query ID=%d: invalid pixel format: %d",
            RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, pix_fmt);
        return false;
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
    {
      // Describes the internal input bind through a human-readable string.
      // This string can be used to better let a user configure input. The array
      // is terminated by retro_input_descriptor::description being set to NULL.
      const retro_input_descriptor *descriptor = reinterpret_cast<const retro_input_descriptor*>(data);

      if (!descriptor->description)
      {
        CLog::Log(LOGERROR, "GameClient environment query ID=%d: no descriptors given",
            RETRO_ENVIRONMENT_SET_VARIABLES);
      }
      else
      {
        while (descriptor && descriptor->description)
        {
          CLog::Log(LOGINFO, "GameClient environment query ID=%d: notified of input %s (port=%d, device=%d, index=%d, id=%d)",
            RETRO_ENVIRONMENT_SET_VARIABLES, descriptor->description,
              descriptor->port, descriptor->device, descriptor->index, descriptor->id);
          descriptor++;
        }
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK:
    {
      // Sets a callback function, called by XBMC, used to notify core about
      // keyboard events.
      // down is set if the key is being pressed, or false if it is being released.
      // keycode is the RETROK value of the char.
      // character is the text character of the pressed key. (UTF-32).
      // key_modifiers is a set of RETROKMOD values or'ed together.
      const retro_keyboard_callback *callback_struct = reinterpret_cast<const retro_keyboard_callback*>(data);
      if (callback_struct->callback)
      {
        CLog::Log(LOGINFO, "GameClient environment query ID=%d: set keyboard callback", RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK);
        _SetKeyboardCallback(callback_struct->callback);
      }
      break;
    }
  default:
    CLog::Log(LOGERROR, "GameClient environment query: invalid query: %u", cmd);
    return false;
  }
  return true;
}
