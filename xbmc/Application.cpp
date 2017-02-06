/*
 *      Copyright (C) 2005-2015 Team XBMC
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
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "network/Network.h"
#include "threads/SystemClock.h"
#include "system.h"
#include "Application.h"
#include "events/EventLog.h"
#include "events/NotificationEvent.h"
#include "interfaces/builtins/Builtins.h"
#include "utils/JobManager.h"
#include "utils/Variant.h"
#include "utils/Splash.h"
#include "LangInfo.h"
#include "utils/Screenshot.h"
#include "Util.h"
#include "URL.h"
#include "guilib/TextureManager.h"
#include "cores/IPlayer.h"
#include "cores/VideoPlayer/DVDFileInfo.h"
#include "cores/AudioEngine/AEFactory.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSP.h"
#include "cores/AudioEngine/Utils/AEUtil.h"
#include "cores/playercorefactory/PlayerCoreFactory.h"
#include "PlayListPlayer.h"
#include "Autorun.h"
#include "video/Bookmark.h"
#include "video/VideoLibraryQueue.h"
#include "guilib/GUIControlProfiler.h"
#include "utils/LangCodeExpander.h"
#include "GUIInfoManager.h"
#include "playlists/PlayListFactory.h"
#include "guilib/GUIFontManager.h"
#include "guilib/GUIColorManager.h"
#include "guilib/StereoscopicsManager.h"
#include "addons/BinaryAddonCache.h"
#include "addons/LanguageResource.h"
#include "addons/Skin.h"
#include "interfaces/generic/ScriptInvocationManager.h"
#ifdef HAS_PYTHON
#include "interfaces/python/XBPython.h"
#endif
#include "input/ButtonTranslator.h"
#include "guilib/GUIAudioManager.h"
#include "GUIPassword.h"
#include "input/InertialScrollingHandler.h"
#include "messaging/ThreadMessage.h"
#include "messaging/ApplicationMessenger.h"
#include "messaging/helpers/DialogHelper.h"
#include "SectionLoader.h"
#include "cores/DllLoader/DllLoaderContainer.h"
#include "GUIUserMessages.h"
#include "filesystem/Directory.h"
#include "filesystem/DirectoryCache.h"
#include "filesystem/StackDirectory.h"
#include "filesystem/SpecialProtocol.h"
#include "filesystem/DllLibCurl.h"
#include "filesystem/PluginDirectory.h"
#include "utils/SystemInfo.h"
#include "utils/TimeUtils.h"
#include "GUILargeTextureManager.h"
#include "TextureCache.h"
#include "playlists/SmartPlayList.h"
#ifdef HAS_FILESYSTEM_RAR
#include "filesystem/RarManager.h"
#endif
#include "playlists/PlayList.h"
#include "profiles/ProfilesManager.h"
#include "windowing/WindowingFactory.h"
#include "powermanagement/PowerManager.h"
#include "powermanagement/DPMSSupport.h"
#include "settings/Settings.h"
#include "settings/AdvancedSettings.h"
#include "settings/DisplaySettings.h"
#include "settings/MediaSettings.h"
#include "settings/SkinSettings.h"
#include "guilib/LocalizeStrings.h"
#include "utils/CPUInfo.h"
#include "utils/SeekHandler.h"

#include "input/KeyboardLayoutManager.h"

#if HAVE_SDL_VERSION == 1
#include <SDL/SDL.h>
#elif HAVE_SDL_VERSION == 2
#include <SDL2/SDL.h>
#endif

#ifdef HAS_UPNP
#include "network/upnp/UPnP.h"
#include "filesystem/UPnPDirectory.h"
#endif
#if defined(TARGET_POSIX) && defined(HAS_FILESYSTEM_SMB)
#include "filesystem/SMBDirectory.h"
#endif
#ifdef HAS_FILESYSTEM_NFS
#include "filesystem/NFSFile.h"
#endif
#ifdef HAS_FILESYSTEM_SFTP
#include "filesystem/SFTPFile.h"
#endif
#include "PartyModeManager.h"
#include "network/ZeroconfBrowser.h"
#ifndef TARGET_POSIX
#include "threads/platform/win/Win32Exception.h"
#endif
#ifdef HAS_EVENT_SERVER
#include "network/EventServer.h"
#endif
#ifdef HAS_DBUS
#include <dbus/dbus.h>
#endif
#ifdef HAS_JSONRPC
#include "interfaces/json-rpc/JSONRPC.h"
#endif
#include "interfaces/AnnouncementManager.h"
#include "peripherals/Peripherals.h"
#include "peripherals/devices/PeripheralImon.h"
#include "music/infoscanner/MusicInfoScanner.h"

// Windows includes
#include "guilib/GUIWindowManager.h"
#include "video/dialogs/GUIDialogVideoInfo.h"
#include "windows/GUIWindowScreensaver.h"
#include "video/VideoInfoScanner.h"
#include "video/PlayerController.h"

// Dialog includes
#include "video/dialogs/GUIDialogVideoBookmarks.h"
#include "dialogs/GUIDialogOK.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "dialogs/GUIDialogSubMenu.h"
#include "dialogs/GUIDialogButtonMenu.h"
#include "dialogs/GUIDialogSimpleMenu.h"
#include "addons/GUIDialogAddonSettings.h"

// PVR related include Files
#include "pvr/PVRManager.h"

#include "epg/EpgContainer.h"

#include "video/dialogs/GUIDialogFullScreenInfo.h"
#include "guilib/GUIControlFactory.h"
#include "dialogs/GUIDialogCache.h"
#include "dialogs/GUIDialogPlayEject.h"
#include "utils/URIUtils.h"
#include "utils/XMLUtils.h"
#include "addons/AddonInstaller.h"
#include "addons/AddonManager.h"
#include "addons/RepositoryUpdater.h"
#include "music/tags/MusicInfoTag.h"
#include "music/tags/MusicInfoTagLoaderFactory.h"
#include "CompileInfo.h"

#ifdef HAS_PERFORMANCE_SAMPLE
#include "utils/PerformanceSample.h"
#else
#define MEASURE_FUNCTION
#endif

#ifdef TARGET_WINDOWS
#include "win32util.h"
#endif

#ifdef TARGET_DARWIN_OSX
#include "platform/darwin/osx/CocoaInterface.h"
#include "platform/darwin/osx/XBMCHelper.h"
#endif
#ifdef TARGET_DARWIN
#include "platform/darwin/DarwinUtils.h"
#endif

#ifdef HAS_DVD_DRIVE
#include <cdio/logging.h>
#endif

#include "storage/MediaManager.h"
#include "utils/JobManager.h"
#include "utils/SaveFileStateJob.h"
#include "utils/AlarmClock.h"
#include "utils/StringUtils.h"
#include "DatabaseManager.h"
#include "input/InputManager.h"

#ifdef TARGET_POSIX
#include "XHandle.h"
#include "XTimeUtils.h"
#endif

#if defined(TARGET_ANDROID)
#include "platform/android/activity/XBMCApp.h"
#include "platform/android/activity/AndroidFeatures.h"
#include "platform/android/jni/Build.h"
#endif

#ifdef TARGET_WINDOWS
#include "utils/Environment.h"
#endif

#if defined(HAS_LIBAMCODEC)
#include "utils/AMLUtils.h"
#endif

#include "cores/FFmpeg.h"
#include "utils/CharsetConverter.h"
#include "pictures/GUIWindowSlideShow.h"
#include "windows/GUIWindowLoginScreen.h"

using namespace ADDON;
using namespace XFILE;
#ifdef HAS_DVD_DRIVE
using namespace MEDIA_DETECT;
#endif
using namespace PLAYLIST;
using namespace VIDEO;
using namespace MUSIC_INFO;
#ifdef HAS_EVENT_SERVER
using namespace EVENTSERVER;
#endif
#ifdef HAS_JSONRPC
using namespace JSONRPC;
#endif
using namespace ANNOUNCEMENT;
using namespace PVR;
using namespace EPG;
using namespace PERIPHERALS;
using namespace KODI::MESSAGING;
using namespace ActiveAE;

using namespace XbmcThreads;

using KODI::MESSAGING::HELPERS::DialogResponse;

#define MAX_FFWD_SPEED 5

//extern IDirectSoundRenderer* m_pAudioDecoder;
CApplication::CApplication(void)
  : m_pPlayer(new CApplicationPlayer)
  , m_iScreenSaveLock(0)
  , m_bPlaybackStarting(false)
  , m_ePlayState(PLAY_STATE_NONE)
  , m_confirmSkinChange(true)
  , m_ignoreSkinSettingChanges(false)
  , m_saveSkinOnUnloading(true)
  , m_autoExecScriptExecuted(false)
  , m_bScreenSave(false)
  , m_bInhibitIdleShutdown(false)
  , m_dpms(nullptr)
  , m_dpmsIsActive(false)
  , m_dpmsIsManual(false)
  , m_itemCurrentFile(new CFileItem)
  , m_stackFileItemToUpdate(new CFileItem)
  , m_threadID(0)
  , m_bInitializing(true)
  , m_bPlatformDirectories(true)
  , m_progressTrackingVideoResumeBookmark(*new CBookmark)
  , m_progressTrackingItem(new CFileItem)
  , m_progressTrackingPlayCountUpdate(false)
  , m_currentStackPosition(0)
  , m_nextPlaylistItem(-1)
  , m_lastRenderTime(0)
  , m_skipGuiRender(false)
  , m_bStandalone(false)
  , m_bEnableLegacyRes(false)
  , m_bTestMode(false)
  , m_bSystemScreenSaverEnable(false)
  , m_musicInfoScanner(new CMusicInfoScanner)
  , m_muted(false)
  , m_volumeLevel(VOLUME_MAXIMUM)
  , m_pInertialScrollingHandler(new CInertialScrollingHandler())
  , m_network(nullptr)
  , m_fallbackLanguageLoaded(false)
  , m_WaitingExternalCalls(0)
  , m_ProcessedExternalCalls(0)
{
  TiXmlBase::SetCondenseWhiteSpace(false);

#ifdef HAS_GLX
  XInitThreads();
#endif

  /* for now always keep this around */
  m_currentStack = new CFileItemList;

#ifdef HAS_DVD_DRIVE
  m_Autorun = new CAutorun();
#endif
}

CApplication::~CApplication(void)
{
  delete m_musicInfoScanner;
  delete &m_progressTrackingVideoResumeBookmark;
#ifdef HAS_DVD_DRIVE
  delete m_Autorun;
#endif
  delete m_currentStack;

  delete m_dpms;
  delete m_pInertialScrollingHandler;
  delete m_pPlayer;

  m_actionListeners.clear();
}

bool CApplication::OnEvent(XBMC_Event& newEvent)
{
  switch(newEvent.type)
  {
    case XBMC_QUIT:
      if (!g_application.m_bStop)
        CApplicationMessenger::GetInstance().PostMsg(TMSG_QUIT);
      break;
    case XBMC_VIDEORESIZE:
      if (g_windowManager.Initialized())
      {
        g_Windowing.SetWindowResolution(newEvent.resize.w, newEvent.resize.h);
        if (!g_advancedSettings.m_fullScreen)
        {
          g_graphicsContext.SetVideoResolution(RES_WINDOW, true);
          CSettings::GetInstance().SetInt(CSettings::SETTING_WINDOW_WIDTH, newEvent.resize.w);
          CSettings::GetInstance().SetInt(CSettings::SETTING_WINDOW_HEIGHT, newEvent.resize.h);
          CSettings::GetInstance().Save();
        }
      }
      break;
    case XBMC_VIDEOMOVE:
#ifdef TARGET_WINDOWS
      if (g_advancedSettings.m_fullScreen)
      {
        // when fullscreen, remain fullscreen and resize to the dimensions of the new screen
        RESOLUTION newRes = (RESOLUTION) g_Windowing.DesktopResolution(g_Windowing.GetCurrentScreen());
        CDisplaySettings::GetInstance().SetCurrentResolution(newRes, true);
        g_graphicsContext.SetVideoResolution(g_graphicsContext.GetVideoResolution(), true);
      }
      else
#endif
      {
        g_Windowing.OnMove(newEvent.move.x, newEvent.move.y);
      }
      break;
    case XBMC_USEREVENT:
      CApplicationMessenger::GetInstance().PostMsg(static_cast<uint32_t>(newEvent.user.code));
      break;
    case XBMC_APPCOMMAND:
      return g_application.OnAppCommand(newEvent.appcommand.action);
    case XBMC_SETFOCUS:
      // Reset the screensaver
      g_application.ResetScreenSaver();
      g_application.WakeUpScreenSaverAndDPMS();
      // Send a mouse motion event with no dx,dy for getting the current guiitem selected
      g_application.OnAction(CAction(ACTION_MOUSE_MOVE, 0, static_cast<float>(newEvent.focus.x), static_cast<float>(newEvent.focus.y), 0, 0));
      break;
    default:
      return CInputManager::GetInstance().OnEvent(newEvent);
  }
  return true;
}

extern "C" void __stdcall init_emu_environ();
extern "C" void __stdcall update_emu_environ();
extern "C" void __stdcall cleanup_emu_environ();

//
// Utility function used to copy files from the application bundle
// over to the user data directory in Application Support/Kodi.
//
static void CopyUserDataIfNeeded(const std::string &strPath, const std::string &file, const std::string &destname = "")
{
  std::string destPath;
  if (destname == "")
    destPath = URIUtils::AddFileToFolder(strPath, file);
  else
    destPath = URIUtils::AddFileToFolder(strPath, destname);
  
  if (!CFile::Exists(destPath))
  {
    // need to copy it across
    std::string srcPath = URIUtils::AddFileToFolder("special://xbmc/userdata/", file);
    CFile::Copy(srcPath, destPath);
  }
}

void CApplication::Preflight()
{
#ifdef HAS_DBUS
  // call 'dbus_threads_init_default' before any other dbus calls in order to
  // avoid race conditions with other threads using dbus connections
  dbus_threads_init_default();
#endif

  // run any platform preflight scripts.
#if defined(TARGET_DARWIN_OSX)
  std::string install_path;

  install_path = CUtil::GetHomePath();
  setenv("KODI_HOME", install_path.c_str(), 0);
  install_path += "/tools/darwin/runtime/preflight";
  system(install_path.c_str());
#endif
}

bool CApplication::SetupNetwork()
{
#if defined(HAS_LINUX_NETWORK)
  m_network = new CNetworkLinux();
#elif defined(HAS_WIN32_NETWORK)
  m_network = new CNetworkWin32();
#else
  m_network = new CNetwork();
#endif

  return m_network != NULL;
}

bool CApplication::Create()
{
  m_ServiceManager.reset(new CServiceManager());
  if (!m_ServiceManager->Init1())
  {
    return false;
  }

  SetupNetwork();
  Preflight();

  // here we register all global classes for the CApplicationMessenger, 
  // after that we can send messages to the corresponding modules
  CApplicationMessenger::GetInstance().RegisterReceiver(this);
  CApplicationMessenger::GetInstance().RegisterReceiver(&g_playlistPlayer);
  CApplicationMessenger::GetInstance().RegisterReceiver(&g_infoManager);

  for (int i = RES_HDTV_1080i; i <= RES_PAL60_16x9; i++)
  {
    g_graphicsContext.ResetScreenParameters((RESOLUTION)i);
    g_graphicsContext.ResetOverscan((RESOLUTION)i, CDisplaySettings::GetInstance().GetResolutionInfo(i).Overscan);
  }

  //! @todo - move to CPlatformXXX
#ifdef TARGET_POSIX
  tzset();   // Initialize timezone information variables
#endif

  // Grab a handle to our thread to be used later in identifying the render thread.
  m_threadID = CThread::GetCurrentThreadId();

  //! @todo - move to CPlatformXXX
#ifndef TARGET_POSIX
  //floating point precision to 24 bits (faster performance)
  _controlfp(_PC_24, _MCW_PC);
#endif

  //! @todo - move to CPlatformXXX
  #if defined(TARGET_POSIX)
    // set special://envhome
    CSpecialProtocol::SetEnvHomePath(getenv("HOME"));
  #endif
    
  // only the InitDirectories* for the current platform should return true
  bool inited = InitDirectoriesLinux();
  if (!inited)
    inited = InitDirectoriesOSX();
  if (!inited)
    inited = InitDirectoriesWin32();

  // copy required files
  CopyUserDataIfNeeded("special://masterprofile/", "RssFeeds.xml");
  CopyUserDataIfNeeded("special://masterprofile/", "favourites.xml");
  CopyUserDataIfNeeded("special://masterprofile/", "Lircmap.xml");
  
  //! @todo - move to CPlatformXXX
  #ifdef TARGET_DARWIN_IOS
    CopyUserDataIfNeeded("special://masterprofile/", "iOS/sources.xml", "sources.xml");
  #endif

  if (!CLog::Init(CSpecialProtocol::TranslatePath("special://logpath").c_str()))
  {
    fprintf(stderr,"Could not init logging classes. Log folder error (%s)\n", CSpecialProtocol::TranslatePath("special://logpath").c_str());
    return false;
  }

  // Init our DllLoaders emu env
  init_emu_environ();

  CProfilesManager::GetInstance().Load();

  CLog::Log(LOGNOTICE, "-----------------------------------------------------------------------");
  CLog::Log(LOGNOTICE, "Starting %s (%s). Platform: %s %s %d-bit", CSysInfo::GetAppName().c_str(), CSysInfo::GetVersion().c_str(),
            g_sysinfo.GetBuildTargetPlatformName().c_str(), g_sysinfo.GetBuildTargetCpuFamily().c_str(), g_sysinfo.GetXbmcBitness());

  std::string buildType;
#if defined(_DEBUG)
  buildType = "Debug";
#elif defined(NDEBUG)
  buildType = "Release";
#else
  buildType = "Unknown";
#endif
  std::string specialVersion;
  
  //! @todo - move to CPlatformXXX
#if defined(TARGET_RASPBERRY_PI)
  specialVersion = " (version for Raspberry Pi)";
//#elif defined(some_ID) // uncomment for special version/fork
//  specialVersion = " (version for XXXX)";
#endif
  CLog::Log(LOGNOTICE, "Using %s %s x%d build%s", buildType.c_str(), CSysInfo::GetAppName().c_str(), g_sysinfo.GetXbmcBitness(), specialVersion.c_str());
  CLog::Log(LOGNOTICE, "%s compiled " __DATE__ " by %s for %s %s %d-bit %s (%s)", CSysInfo::GetAppName().c_str(), g_sysinfo.GetUsedCompilerNameAndVer().c_str(), g_sysinfo.GetBuildTargetPlatformName().c_str(),
            g_sysinfo.GetBuildTargetCpuFamily().c_str(), g_sysinfo.GetXbmcBitness(), g_sysinfo.GetBuildTargetPlatformVersionDecoded().c_str(),
            g_sysinfo.GetBuildTargetPlatformVersion().c_str());

  std::string deviceModel(g_sysinfo.GetModelName());
  if (!g_sysinfo.GetManufacturerName().empty())
    deviceModel = g_sysinfo.GetManufacturerName() + " " + (deviceModel.empty() ? std::string("device") : deviceModel);
  if (!deviceModel.empty())
    CLog::Log(LOGNOTICE, "Running on %s with %s, kernel: %s %s %d-bit version %s", deviceModel.c_str(), g_sysinfo.GetOsPrettyNameWithVersion().c_str(),
              g_sysinfo.GetKernelName().c_str(), g_sysinfo.GetKernelCpuFamily().c_str(), g_sysinfo.GetKernelBitness(), g_sysinfo.GetKernelVersionFull().c_str());
  else
    CLog::Log(LOGNOTICE, "Running on %s, kernel: %s %s %d-bit version %s", g_sysinfo.GetOsPrettyNameWithVersion().c_str(),
              g_sysinfo.GetKernelName().c_str(), g_sysinfo.GetKernelCpuFamily().c_str(), g_sysinfo.GetKernelBitness(), g_sysinfo.GetKernelVersionFull().c_str());

  CLog::Log(LOGNOTICE, "FFmpeg version/source: %s", av_version_info());

  std::string cpuModel(g_cpuInfo.getCPUModel());
  if (!cpuModel.empty())
    CLog::Log(LOGNOTICE, "Host CPU: %s, %d core%s available", cpuModel.c_str(), g_cpuInfo.getCPUCount(), (g_cpuInfo.getCPUCount() == 1) ? "" : "s");
  else
    CLog::Log(LOGNOTICE, "%d CPU core%s available", g_cpuInfo.getCPUCount(), (g_cpuInfo.getCPUCount() == 1) ? "" : "s");
  
  //! @todo - move to CPlatformXXX ???
#if defined(TARGET_WINDOWS)
  CLog::Log(LOGNOTICE, "%s", CWIN32Util::GetResInfoString().c_str());
  CLog::Log(LOGNOTICE, "Running with %s rights", (CWIN32Util::IsCurrentUserLocalAdministrator() == TRUE) ? "administrator" : "restricted");
  CLog::Log(LOGNOTICE, "Aero is %s", (g_sysinfo.IsAeroDisabled() == true) ? "disabled" : "enabled");
#endif
#if defined(TARGET_ANDROID)
  CLog::Log(LOGNOTICE,
        "Product: %s, Device: %s, Board: %s - Manufacturer: %s, Brand: %s, Model: %s, Hardware: %s",
        CJNIBuild::PRODUCT.c_str(), CJNIBuild::DEVICE.c_str(), CJNIBuild::BOARD.c_str(),
        CJNIBuild::MANUFACTURER.c_str(), CJNIBuild::BRAND.c_str(), CJNIBuild::MODEL.c_str(), CJNIBuild::HARDWARE.c_str());
  std::string extstorage;
  bool extready = CXBMCApp::GetExternalStorage(extstorage);
  CLog::Log(LOGNOTICE, "External storage path = %s; status = %s", extstorage.c_str(), extready ? "ok" : "nok");
#endif

#if defined(__arm__) || defined(__aarch64__)
  if (g_cpuInfo.GetCPUFeatures() & CPU_FEATURE_NEON)
    CLog::Log(LOGNOTICE, "ARM Features: Neon enabled");
  else
    CLog::Log(LOGNOTICE, "ARM Features: Neon disabled");
#endif
  CSpecialProtocol::LogPaths();

  std::string executable = CUtil::ResolveExecutablePath();
  CLog::Log(LOGNOTICE, "The executable running is: %s", executable.c_str());
  std::string hostname("[unknown]");
  m_network->GetHostName(hostname);
  CLog::Log(LOGNOTICE, "Local hostname: %s", hostname.c_str());
  std::string lowerAppName = CCompileInfo::GetAppName();
  StringUtils::ToLower(lowerAppName);
  CLog::Log(LOGNOTICE, "Log File is located: %s/%s.log", CSpecialProtocol::TranslatePath("special://logpath").c_str(), lowerAppName.c_str());
  CRegExp::LogCheckUtf8Support();
  CLog::Log(LOGNOTICE, "-----------------------------------------------------------------------");

  std::string strExecutablePath = CUtil::GetHomePath();

  // for python scripts that check the OS
  //! @todo - move to CPlatformXXX
#if defined(TARGET_DARWIN)
  setenv("OS","OS X",true);
#elif defined(TARGET_POSIX)
  setenv("OS","Linux",true);
#elif defined(TARGET_WINDOWS)
  CEnvironment::setenv("OS", "win32");
#endif

  // register ffmpeg lockmanager callback
  av_lockmgr_register(&ffmpeg_lockmgr_cb);
  // register avcodec
  avcodec_register_all();
  // register avformat
  av_register_all();
  // register avfilter
  avfilter_register_all();
  // initialize network protocols
  avformat_network_init();
  // set avutil callback
  av_log_set_callback(ff_avutil_log);

  g_powerManager.Initialize();

  // Load the AudioEngine before settings as they need to query the engine
  if (!CAEFactory::LoadEngine())
  {
    CLog::Log(LOGFATAL, "CApplication::Create: Failed to load an AudioEngine");
    return false;
  }

  // Initialize default Settings - don't move
  CLog::Log(LOGNOTICE, "load settings...");
  if (!CSettings::GetInstance().Initialize())
    return false;

  g_powerManager.SetDefaults();

  // load the actual values
  if (!CSettings::GetInstance().Load())
  {
    CLog::Log(LOGFATAL, "unable to load settings");
    return false;
  }
  CSettings::GetInstance().SetLoaded();

  CLog::Log(LOGINFO, "creating subdirectories");
  CLog::Log(LOGINFO, "userdata folder: %s", CURL::GetRedacted(CProfilesManager::GetInstance().GetProfileUserDataFolder()).c_str());
  CLog::Log(LOGINFO, "recording folder: %s", CURL::GetRedacted(CSettings::GetInstance().GetString(CSettings::SETTING_AUDIOCDS_RECORDINGPATH)).c_str());
  CLog::Log(LOGINFO, "screenshots folder: %s", CURL::GetRedacted(CSettings::GetInstance().GetString(CSettings::SETTING_DEBUG_SCREENSHOTPATH)).c_str());
  CDirectory::Create(CProfilesManager::GetInstance().GetUserDataFolder());
  CDirectory::Create(CProfilesManager::GetInstance().GetProfileUserDataFolder());
  CProfilesManager::GetInstance().CreateProfileFolders();

  update_emu_environ();//apply the GUI settings

  //! @todo - move to CPlatformXXX
#ifdef TARGET_WINDOWS
  CWIN32Util::SetThreadLocalLocale(true); // enable independent locale for each thread, see https://connect.microsoft.com/VisualStudio/feedback/details/794122
#endif // TARGET_WINDOWS

  // initialize the addon database (must be before the addon manager is init'd)
  CDatabaseManager::GetInstance().Initialize(true);

  if (!m_ServiceManager->Init2())
  {
    return false;
  }

  // start the AudioEngine
  if (!CAEFactory::StartEngine())
  {
    CLog::Log(LOGFATAL, "CApplication::Create: Failed to start the AudioEngine");
    return false;
  }

  // restore AE's previous volume state
  SetHardwareVolume(m_volumeLevel);
  CAEFactory::SetMute     (m_muted);
  CAEFactory::SetSoundMode(CSettings::GetInstance().GetInt(CSettings::SETTING_AUDIOOUTPUT_GUISOUNDMODE));

  // initialize m_replayGainSettings
  m_replayGainSettings.iType = CSettings::GetInstance().GetInt(CSettings::SETTING_MUSICPLAYER_REPLAYGAINTYPE);
  m_replayGainSettings.iPreAmp = CSettings::GetInstance().GetInt(CSettings::SETTING_MUSICPLAYER_REPLAYGAINPREAMP);
  m_replayGainSettings.iNoGainPreAmp = CSettings::GetInstance().GetInt(CSettings::SETTING_MUSICPLAYER_REPLAYGAINNOGAINPREAMP);

  // Create the Mouse, Keyboard, Remote, and Joystick devices
  // Initialize after loading settings to get joystick deadzone setting
  CInputManager::GetInstance().InitializeInputs();

  // load the keyboard layouts
  if (!CKeyboardLayoutManager::GetInstance().Load())
  {
    CLog::Log(LOGFATAL, "CApplication::Create: Unable to load keyboard layouts");
    return false;
  }

  //! @todo - move to CPlatformXXX
#if defined(TARGET_DARWIN_OSX)
  // Configure and possible manually start the helper.
  XBMCHelper::GetInstance().Configure();
#endif

  CUtil::InitRandomSeed();

  g_mediaManager.Initialize();

  m_lastRenderTime = XbmcThreads::SystemClockMillis();
  return true;
}

bool CApplication::CreateGUI()
{
  m_frameMoveGuard.lock();

  m_renderGUI = true;
#ifdef HAS_SDL
  CLog::Log(LOGNOTICE, "Setup SDL");

  /* Clean up on exit, exit on window close and interrupt */
  atexit(SDL_Quit);

  uint32_t sdlFlags = 0;

#if defined(TARGET_DARWIN_OSX)
  sdlFlags |= SDL_INIT_VIDEO;
#endif

  //depending on how it's compiled, SDL periodically calls XResetScreenSaver when it's fullscreen
  //this might bring the monitor out of standby, so we have to disable it explicitly
  //by passing 0 for overwrite to setsenv, the user can still override this by setting the environment variable
#if defined(TARGET_POSIX) && !defined(TARGET_DARWIN)
  setenv("SDL_VIDEO_ALLOW_SCREENSAVER", "1", 0);
#endif

#endif // HAS_SDL

  m_bSystemScreenSaverEnable = g_Windowing.IsSystemScreenSaverEnabled();
  g_Windowing.EnableSystemScreenSaver(false);

#ifdef HAS_SDL
  if (SDL_Init(sdlFlags) != 0)
  {
    CLog::Log(LOGFATAL, "XBAppEx: Unable to initialize SDL: %s", SDL_GetError());
    return false;
  }
  #if defined(TARGET_DARWIN)
  // SDL_Init will install a handler for segfaults, restore the default handler.
  signal(SIGSEGV, SIG_DFL);
  #endif
#endif

  // Initialize core peripheral port support. Note: If these parameters
  // are 0 and NULL, respectively, then the default number and types of
  // controllers will be initialized.
  if (!g_Windowing.InitWindowSystem())
  {
    CLog::Log(LOGFATAL, "CApplication::Create: Unable to init windowing system");
    return false;
  }

  // Retrieve the matching resolution based on GUI settings
  bool sav_res = false;
  CDisplaySettings::GetInstance().SetCurrentResolution(CDisplaySettings::GetInstance().GetDisplayResolution());
  CLog::Log(LOGNOTICE, "Checking resolution %i", CDisplaySettings::GetInstance().GetCurrentResolution());
  if (!g_graphicsContext.IsValidResolution(CDisplaySettings::GetInstance().GetCurrentResolution()))
  {
    CLog::Log(LOGNOTICE, "Setting safe mode %i", RES_DESKTOP);
    // defer saving resolution after window was created
    CDisplaySettings::GetInstance().SetCurrentResolution(RES_DESKTOP);
    sav_res = true;
  }

  // update the window resolution
  g_Windowing.SetWindowResolution(CSettings::GetInstance().GetInt(CSettings::SETTING_WINDOW_WIDTH), CSettings::GetInstance().GetInt(CSettings::SETTING_WINDOW_HEIGHT));

  if (g_advancedSettings.m_startFullScreen && CDisplaySettings::GetInstance().GetCurrentResolution() == RES_WINDOW)
  {
    // defer saving resolution after window was created
    CDisplaySettings::GetInstance().SetCurrentResolution(RES_DESKTOP);
    sav_res = true;
  }

  if (!g_graphicsContext.IsValidResolution(CDisplaySettings::GetInstance().GetCurrentResolution()))
  {
    // Oh uh - doesn't look good for starting in their wanted screenmode
    CLog::Log(LOGERROR, "The screen resolution requested is not valid, resetting to a valid mode");
    CDisplaySettings::GetInstance().SetCurrentResolution(RES_DESKTOP);
    sav_res = true;
  }
  if (!InitWindow())
  {
    return false;
  }

  if (sav_res)
    CDisplaySettings::GetInstance().SetCurrentResolution(RES_DESKTOP, true);

  CSplash::GetInstance().Show();

  // The key mappings may already have been loaded by a peripheral
  CLog::Log(LOGINFO, "load keymapping");
  if (!CButtonTranslator::GetInstance().Load())
    return false;

  RESOLUTION_INFO info = g_graphicsContext.GetResInfo();
  CLog::Log(LOGINFO, "GUI format %ix%i, Display %s",
            info.iWidth,
            info.iHeight,
            info.strMode.c_str());

  g_windowManager.Initialize();

  return true;
}

bool CApplication::InitWindow(RESOLUTION res)
{
  if (res == RES_INVALID)
    res = CDisplaySettings::GetInstance().GetCurrentResolution();

  bool bFullScreen = res != RES_WINDOW;
  if (!g_Windowing.CreateNewWindow(CSysInfo::GetAppName(), bFullScreen, CDisplaySettings::GetInstance().GetResolutionInfo(res), OnEvent))
  {
    CLog::Log(LOGFATAL, "CApplication::Create: Unable to create window");
    return false;
  }

  if (!g_Windowing.InitRenderSystem())
  {
    CLog::Log(LOGFATAL, "CApplication::Create: Unable to init rendering system");
    return false;
  }
  // set GUI res and force the clear of the screen
  g_graphicsContext.SetVideoResolution(res);
  return true;
}

bool CApplication::DestroyWindow()
{
  return g_Windowing.DestroyWindow();
}

bool CApplication::InitDirectoriesLinux()
{
/*
   The following is the directory mapping for Platform Specific Mode:

   special://xbmc/          => [read-only] system directory (/usr/share/kodi)
   special://home/          => [read-write] user's directory that will override special://kodi/ system-wide
                               installations like skins, screensavers, etc.
                               ($HOME/.kodi)
                               NOTE: XBMC will look in both special://xbmc/addons and special://home/addons for addons.
   special://masterprofile/ => [read-write] userdata of master profile. It will by default be
                               mapped to special://home/userdata ($HOME/.kodi/userdata)
   special://profile/       => [read-write] current profile's userdata directory.
                               Generally special://masterprofile for the master profile or
                               special://masterprofile/profiles/<profile_name> for other profiles.

   NOTE: All these root directories are lowercase. Some of the sub-directories
         might be mixed case.
*/

#if defined(TARGET_POSIX) && !defined(TARGET_DARWIN)
  std::string userName;
  if (getenv("USER"))
    userName = getenv("USER");
  else
    userName = "root";

  std::string userHome;
  if (getenv("HOME"))
    userHome = getenv("HOME");
  else
    userHome = "/root";

  std::string binaddonAltDir;
  if (getenv("KODI_BINADDON_PATH"))
    binaddonAltDir = getenv("KODI_BINADDON_PATH");

  std::string appPath;
  std::string appName = CCompileInfo::GetAppName();
  std::string dotLowerAppName = "." + appName;
  StringUtils::ToLower(dotLowerAppName);
  const char* envAppHome = "KODI_HOME";
  const char* envAppBinHome = "KODI_BIN_HOME";
  const char* envAppTemp = "KODI_TEMP";

  auto appBinPath = CUtil::GetHomePath(envAppBinHome);
  // overridden by user
  if (getenv(envAppHome))
    appPath = getenv(envAppHome);
  else
  {
    // use build time default
    appPath = INSTALL_PATH;
    /* Check if binaries and arch independent data files are being kept in
     * separate locations. */
    if (!CDirectory::Exists(URIUtils::AddFileToFolder(appPath, "userdata")))
    {
      /* Attempt to locate arch independent data files. */
      appPath = CUtil::GetHomePath(appBinPath);
      if (!CDirectory::Exists(URIUtils::AddFileToFolder(appPath, "userdata")))
      {
        fprintf(stderr, "Unable to find path to %s data files!\n", appName.c_str());
        exit(1);
      }
    }
  }

  /* Set some environment variables */
  setenv(envAppBinHome, appBinPath.c_str(), 0);
  setenv(envAppHome, appPath.c_str(), 0);

  if (m_bPlatformDirectories)
  {
    // map our special drives
    CSpecialProtocol::SetXBMCBinPath(appBinPath);
    CSpecialProtocol::SetXBMCAltBinAddonPath(binaddonAltDir);
    CSpecialProtocol::SetXBMCPath(appPath);
    CSpecialProtocol::SetHomePath(userHome + "/" + dotLowerAppName);
    CSpecialProtocol::SetMasterProfilePath(userHome + "/" + dotLowerAppName + "/userdata");

    std::string strTempPath = userHome;
    strTempPath = URIUtils::AddFileToFolder(strTempPath, dotLowerAppName + "/temp");
    if (getenv(envAppTemp))
      strTempPath = getenv(envAppTemp);
    CSpecialProtocol::SetTempPath(strTempPath);
    CSpecialProtocol::SetLogPath(strTempPath);

    CreateUserDirs();

  }
  else
  {
    URIUtils::AddSlashAtEnd(appPath);

    CSpecialProtocol::SetXBMCBinPath(appBinPath);
    CSpecialProtocol::SetXBMCAltBinAddonPath(binaddonAltDir);
    CSpecialProtocol::SetXBMCPath(appPath);
    CSpecialProtocol::SetHomePath(URIUtils::AddFileToFolder(appPath, "portable_data"));
    CSpecialProtocol::SetMasterProfilePath(URIUtils::AddFileToFolder(appPath, "portable_data/userdata"));

    std::string strTempPath = appPath;
    strTempPath = URIUtils::AddFileToFolder(strTempPath, "portable_data/temp");
    if (getenv(envAppTemp))
      strTempPath = getenv(envAppTemp);
    CSpecialProtocol::SetTempPath(strTempPath);
    CSpecialProtocol::SetLogPath(strTempPath);
    CreateUserDirs();
  }
  CSpecialProtocol::SetXBMCBinAddonPath(appBinPath + "/addons");

#if defined(TARGET_ANDROID)
  CXBMCApp::InitDirectories();
#endif

  return true;
#else
  return false;
#endif
}

bool CApplication::InitDirectoriesOSX()
{
#if defined(TARGET_DARWIN)
  std::string userName;
  if (getenv("USER"))
    userName = getenv("USER");
  else
    userName = "root";

  std::string userHome;
  if (getenv("HOME"))
    userHome = getenv("HOME");
  else
    userHome = "/root";

  std::string binaddonAltDir;
  if (getenv("KODI_BINADDON_PATH"))
    binaddonAltDir = getenv("KODI_BINADDON_PATH");

  std::string appPath = CUtil::GetHomePath();
  setenv("KODI_HOME", appPath.c_str(), 0);

#if defined(TARGET_DARWIN_IOS)
  std::string fontconfigPath;
  fontconfigPath = appPath + "/system/players/VideoPlayer/etc/fonts/fonts.conf";
  setenv("FONTCONFIG_FILE", fontconfigPath.c_str(), 0);
#endif

  // setup path to our internal dylibs so loader can find them
  std::string frameworksPath = CUtil::GetFrameworksPath();
  CSpecialProtocol::SetXBMCFrameworksPath(frameworksPath);

  // OSX always runs with m_bPlatformDirectories == true
  if (m_bPlatformDirectories)
  {
    // map our special drives
    CSpecialProtocol::SetXBMCBinPath(appPath);
    CSpecialProtocol::SetXBMCAltBinAddonPath(binaddonAltDir);
    CSpecialProtocol::SetXBMCPath(appPath);
    #if defined(TARGET_DARWIN_IOS)
      std::string appName = CCompileInfo::GetAppName();
      CSpecialProtocol::SetHomePath(userHome + "/" + CDarwinUtils::GetAppRootFolder() + "/" + appName);
      CSpecialProtocol::SetMasterProfilePath(userHome + "/" + CDarwinUtils::GetAppRootFolder() + "/" + appName + "/userdata");
    #else
      std::string appName = CCompileInfo::GetAppName();
      CSpecialProtocol::SetHomePath(userHome + "/Library/Application Support/" + appName);
      CSpecialProtocol::SetMasterProfilePath(userHome + "/Library/Application Support/" + appName + "/userdata");
    #endif

    std::string dotLowerAppName = "." + appName;
    StringUtils::ToLower(dotLowerAppName);
    // location for temp files
    #if defined(TARGET_DARWIN_IOS)
      std::string strTempPath = URIUtils::AddFileToFolder(userHome,  std::string(CDarwinUtils::GetAppRootFolder()) + "/" + appName + "/temp");
    #else
      std::string strTempPath = URIUtils::AddFileToFolder(userHome, dotLowerAppName + "/");
      CDirectory::Create(strTempPath);
      strTempPath = URIUtils::AddFileToFolder(userHome, dotLowerAppName + "/temp");
    #endif
    CSpecialProtocol::SetTempPath(strTempPath);

    // xbmc.log file location
    #if defined(TARGET_DARWIN_IOS)
      strTempPath = userHome + "/" + std::string(CDarwinUtils::GetAppRootFolder());
    #else
      strTempPath = userHome + "/Library/Logs";
    #endif
    CSpecialProtocol::SetLogPath(strTempPath);
    CreateUserDirs();
  }
  else
  {
    URIUtils::AddSlashAtEnd(appPath);

    CSpecialProtocol::SetXBMCBinPath(appPath);
    CSpecialProtocol::SetXBMCAltBinAddonPath(binaddonAltDir);
    CSpecialProtocol::SetXBMCPath(appPath);
    CSpecialProtocol::SetHomePath(URIUtils::AddFileToFolder(appPath, "portable_data"));
    CSpecialProtocol::SetMasterProfilePath(URIUtils::AddFileToFolder(appPath, "portable_data/userdata"));

    std::string strTempPath = URIUtils::AddFileToFolder(appPath, "portable_data/temp");
    CSpecialProtocol::SetTempPath(strTempPath);
    CSpecialProtocol::SetLogPath(strTempPath);
  }
  CSpecialProtocol::SetXBMCBinAddonPath(appPath + "/addons");
  return true;
#else
  return false;
#endif
}

bool CApplication::InitDirectoriesWin32()
{
#ifdef TARGET_WINDOWS
  std::string xbmcPath = CUtil::GetHomePath();
  CEnvironment::setenv("KODI_HOME", xbmcPath);
  CSpecialProtocol::SetXBMCBinPath(xbmcPath);
  CSpecialProtocol::SetXBMCPath(xbmcPath);
  CSpecialProtocol::SetXBMCBinAddonPath(xbmcPath + "/addons");

  std::string strWin32UserFolder = CWIN32Util::GetProfilePath();
  CSpecialProtocol::SetLogPath(strWin32UserFolder);
  CSpecialProtocol::SetHomePath(strWin32UserFolder);
  CSpecialProtocol::SetMasterProfilePath(URIUtils::AddFileToFolder(strWin32UserFolder, "userdata"));
  CSpecialProtocol::SetTempPath(URIUtils::AddFileToFolder(strWin32UserFolder,"cache"));

  CEnvironment::setenv("KODI_PROFILE_USERDATA", CSpecialProtocol::TranslatePath("special://masterprofile/"));

  CreateUserDirs();

  return true;
#else
  return false;
#endif
}

void CApplication::CreateUserDirs() const
{
  CDirectory::Create("special://home/");
  CDirectory::Create("special://home/addons");
  CDirectory::Create("special://home/addons/packages");
  CDirectory::Create("special://home/addons/temp");
  CDirectory::Create("special://home/media");
  CDirectory::Create("special://home/system");
  CDirectory::Create("special://masterprofile/");
  CDirectory::Create("special://temp/");
  CDirectory::Create("special://logpath");
  CDirectory::Create("special://temp/temp"); // temp directory for python and dllGetTempPathA

  //Let's clear our archive cache before starting up anything more
  auto archiveCachePath = CSpecialProtocol::TranslatePath("special://temp/archive_cache/");
  if (CDirectory::Exists(archiveCachePath))
    if (!CDirectory::RemoveRecursive(archiveCachePath))
      CLog::Log(LOGWARNING, "Failed to remove the archive cache at %s", archiveCachePath.c_str());
  CDirectory::Create(archiveCachePath);

}

bool CApplication::Initialize()
{
#if defined(HAS_DVD_DRIVE) && !defined(TARGET_WINDOWS) // somehow this throws an "unresolved external symbol" on win32
  // turn off cdio logging
  cdio_loglevel_default = CDIO_LOG_ERROR;
#endif

#ifdef TARGET_POSIX //! @todo Win32 has no special://home/ mapping by default, so we
              //!       must create these here. Ideally this should be using special://home/ and
              //!      be platform agnostic (i.e. unify the InitDirectories*() functions)
  if (!m_bPlatformDirectories)
#endif
  {
    CDirectory::Create("special://xbmc/addons");
  }

  // load the language and its translated strings
  if (!LoadLanguage(false))
    return false;

  CEventLog::GetInstance().Add(EventPtr(new CNotificationEvent(
    StringUtils::Format(g_localizeStrings.Get(177).c_str(), g_sysinfo.GetAppName().c_str()),
    StringUtils::Format(g_localizeStrings.Get(178).c_str(), g_sysinfo.GetAppName().c_str()),
    "special://xbmc/media/icon256x256.png", EventLevel::Basic)));

#if !defined(TARGET_DARWIN_IOS)
  g_peripherals.Initialise();
#endif

  // Load curl so curl_global_init gets called before any service threads
  // are started. Unloading will have no effect as curl is never fully unloaded.
  // To quote man curl_global_init:
  //  "This function is not thread safe. You must not call it when any other
  //  thread in the program (i.e. a thread sharing the same memory) is running.
  //  This doesn't just mean no other thread that is using libcurl. Because
  //  curl_global_init() calls functions of other libraries that are similarly
  //  thread unsafe, it could conflict with any other thread that
  //  uses these other libraries."
  g_curlInterface.Load();
  g_curlInterface.Unload();

  // initialize (and update as needed) our databases
  CEvent event(true);
  CJobManager::GetInstance().Submit([&event]() {
    CDatabaseManager::GetInstance().Initialize();
    event.Set();
  });
  std::string localizedStr = g_localizeStrings.Get(24150);
  int iDots = 1;
  while (!event.WaitMSec(1000))
  {
    if (CDatabaseManager::GetInstance().m_bIsUpgrading)
      CSplash::GetInstance().Show(std::string(iDots, ' ') + localizedStr + std::string(iDots, '.'));
    if (iDots == 3)
      iDots = 1;
    else
      ++iDots;
  }
  CSplash::GetInstance().Show();

  StartServices();

  // Init DPMS, before creating the corresponding setting control.
  m_dpms = new DPMSSupport();
  bool uiInitializationFinished = true;
  if (g_windowManager.Initialized())
  {
    CSettings::GetInstance().GetSetting(CSettings::SETTING_POWERMANAGEMENT_DISPLAYSOFF)->SetRequirementsMet(m_dpms->IsSupported());

    g_windowManager.CreateWindows();

    m_confirmSkinChange = false;

    std::vector<std::string> incompatibleAddons;
    event.Reset();
    std::atomic<bool> isMigratingAddons(false);
    CJobManager::GetInstance().Submit([&event, &incompatibleAddons, &isMigratingAddons]() {
        incompatibleAddons = CAddonSystemSettings::GetInstance().MigrateAddons([&isMigratingAddons]() {
          isMigratingAddons = true;
        });
        event.Set();
      }, CJob::PRIORITY_DEDICATED);
    localizedStr = g_localizeStrings.Get(24151);
    iDots = 1;
    while (!event.WaitMSec(1000))
    {
      if (isMigratingAddons)
        CSplash::GetInstance().Show(std::string(iDots, ' ') + localizedStr + std::string(iDots, '.'));
      if (iDots == 3)
        iDots = 1;
      else
        ++iDots;
    }
    CSplash::GetInstance().Show();
    m_incompatibleAddons = incompatibleAddons;
    m_confirmSkinChange = true;

    std::string defaultSkin = ((const CSettingString*)CSettings::GetInstance().GetSetting(CSettings::SETTING_LOOKANDFEEL_SKIN))->GetDefault();
    if (!LoadSkin(CSettings::GetInstance().GetString(CSettings::SETTING_LOOKANDFEEL_SKIN)))
    {
      CLog::Log(LOGERROR, "Failed to load skin '%s'", CSettings::GetInstance().GetString(CSettings::SETTING_LOOKANDFEEL_SKIN).c_str());
      if (!LoadSkin(defaultSkin))
      {
        CLog::Log(LOGFATAL, "Default skin '%s' could not be loaded! Terminating..", defaultSkin.c_str());
        return false;
      }
    }

    // initialize splash window after splash screen disappears
    // because we need a real window in the background which gets
    // rendered while we load the main window or enter the master lock key
    if (g_advancedSettings.m_splashImage)
      g_windowManager.ActivateWindow(WINDOW_SPLASH);

    if (CSettings::GetInstance().GetBool(CSettings::SETTING_MASTERLOCK_STARTUPLOCK) &&
        CProfilesManager::GetInstance().GetMasterProfile().getLockMode() != LOCK_MODE_EVERYONE &&
       !CProfilesManager::GetInstance().GetMasterProfile().getLockCode().empty())
    {
       g_passwordManager.CheckStartUpLock();
    }

    // check if we should use the login screen
    if (CProfilesManager::GetInstance().UsingLoginScreen())
    {
      // the login screen still needs to perform additional initialization
      uiInitializationFinished = false;

      g_windowManager.ActivateWindow(WINDOW_LOGIN_SCREEN);
    }
    else
    {
#ifdef HAS_JSONRPC
      CJSONRPC::Initialize();
#endif
      ADDON::CAddonMgr::GetInstance().StartServices(false);

      // activate the configured start window
      int firstWindow = g_SkinInfo->GetFirstWindow();
      g_windowManager.ActivateWindow(firstWindow);

      if (g_windowManager.GetActiveWindowID() == WINDOW_STARTUP_ANIM)
      {
        CLog::Log(LOGWARNING, "CApplication::Initialize - startup.xml taints init process");
      }

      // the startup window is considered part of the initialization as it most likely switches to the final window
      uiInitializationFinished = firstWindow != WINDOW_STARTUP_ANIM;

      CStereoscopicsManager::GetInstance().Initialize();

      if (!m_ServiceManager->Init3())
      {
        CLog::Log(LOGERROR, "Application - Init3 failed");
      }
    }

  }
  else //No GUI Created
  {
#ifdef HAS_JSONRPC
    CJSONRPC::Initialize();
#endif
    ADDON::CAddonMgr::GetInstance().StartServices(false);
  }

  g_sysinfo.Refresh();

  CLog::Log(LOGINFO, "removing tempfiles");
  CUtil::RemoveTempFiles();

  if (!CProfilesManager::GetInstance().UsingLoginScreen())
  {
    UpdateLibraries();
    SetLoggingIn(false);
  }

  m_slowTimer.StartZero();

  CAddonMgr::GetInstance().StartServices(true);

  // configure seek handler
  CSeekHandler::GetInstance().Configure();

  // register action listeners
  RegisterActionListener(&CSeekHandler::GetInstance());
  RegisterActionListener(&CPlayerController::GetInstance());

  CRepositoryUpdater::GetInstance().Start();

  CLog::Log(LOGNOTICE, "initialize done");

  // reset our screensaver (starts timers etc.)
  ResetScreenSaver();

  // if the user interfaces has been fully initialized let everyone know
  if (uiInitializationFinished)
  {
    CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UI_READY);
    g_windowManager.SendThreadMessage(msg);
  }

  return true;
}

bool CApplication::StartServer(enum ESERVERS eServer, bool bStart, bool bWait/* = false*/)
{
  bool ret = false;
  switch(eServer)
  {
    case ES_WEBSERVER:
      // the callback will take care of starting/stopping webserver
      ret = CSettings::GetInstance().SetBool(CSettings::SETTING_SERVICES_WEBSERVER, bStart);
      break;

    case ES_AIRPLAYSERVER:
      // the callback will take care of starting/stopping airplay
      ret = CSettings::GetInstance().SetBool(CSettings::SETTING_SERVICES_AIRPLAY, bStart);
      break;

    case ES_JSONRPCSERVER:
      // the callback will take care of starting/stopping jsonrpc server
      ret = CSettings::GetInstance().SetBool(CSettings::SETTING_SERVICES_ESENABLED, bStart);
      break;

    case ES_UPNPSERVER:
      // the callback will take care of starting/stopping upnp server
      ret = CSettings::GetInstance().SetBool(CSettings::SETTING_SERVICES_UPNPSERVER, bStart);
      break;

    case ES_UPNPRENDERER:
      // the callback will take care of starting/stopping upnp renderer
      ret = CSettings::GetInstance().SetBool(CSettings::SETTING_SERVICES_UPNPRENDERER, bStart);
      break;

    case ES_EVENTSERVER:
      // the callback will take care of starting/stopping event server
      ret = CSettings::GetInstance().SetBool(CSettings::SETTING_SERVICES_ESENABLED, bStart);
      break;

    case ES_ZEROCONF:
      // the callback will take care of starting/stopping zeroconf
      ret = CSettings::GetInstance().SetBool(CSettings::SETTING_SERVICES_ZEROCONF, bStart);
      break;

    default:
      ret = false;
      break;
  }
  CSettings::GetInstance().Save();

  return ret;
}

void CApplication::StartServices()
{
#if !defined(TARGET_WINDOWS) && defined(HAS_DVD_DRIVE)
  // Start Thread for DVD Mediatype detection
  CLog::Log(LOGNOTICE, "start dvd mediatype detection");
  m_DetectDVDType.Create(false, THREAD_MINSTACKSIZE);
#endif
}

void CApplication::StopServices()
{
  m_network->NetworkMessage(CNetwork::SERVICES_DOWN, 0);

#if !defined(TARGET_WINDOWS) && defined(HAS_DVD_DRIVE)
  CLog::Log(LOGNOTICE, "stop dvd detect media");
  m_DetectDVDType.StopThread();
#endif

  g_peripherals.Clear();
}

void CApplication::OnSettingChanged(const CSetting *setting)
{
  if (setting == NULL)
    return;

  const std::string &settingId = setting->GetId();

  if (settingId == CSettings::SETTING_LOOKANDFEEL_SKIN ||
      settingId == CSettings::SETTING_LOOKANDFEEL_FONT ||
      settingId == CSettings::SETTING_LOOKANDFEEL_SKINTHEME ||
      settingId == CSettings::SETTING_LOOKANDFEEL_SKINCOLORS)
  {
    // check if we should ignore this change event due to changing skins in which case we have to
    // change several settings and each one of them could lead to a complete skin reload which would
    // result in multiple skin reloads. Therefore we manually specify to ignore specific settings
    // which are going to be changed.
    if (m_ignoreSkinSettingChanges)
      return;

    // if the skin changes and the current color/theme/font is not the default one, reset
    // the it to the default value
    if (settingId == CSettings::SETTING_LOOKANDFEEL_SKIN)
    {
      CSetting* skinRelatedSetting = CSettings::GetInstance().GetSetting(CSettings::SETTING_LOOKANDFEEL_SKINCOLORS);
      if (!skinRelatedSetting->IsDefault())
      {
        m_ignoreSkinSettingChanges = true;
        skinRelatedSetting->Reset();
      }

      skinRelatedSetting = CSettings::GetInstance().GetSetting(CSettings::SETTING_LOOKANDFEEL_SKINTHEME);
      if (!skinRelatedSetting->IsDefault())
      {
        m_ignoreSkinSettingChanges = true;
        skinRelatedSetting->Reset();
      }

      skinRelatedSetting = CSettings::GetInstance().GetSetting(CSettings::SETTING_LOOKANDFEEL_FONT);
      if (!skinRelatedSetting->IsDefault())
      {
        m_ignoreSkinSettingChanges = true;
        skinRelatedSetting->Reset();
      }
    }
    else if (settingId == CSettings::SETTING_LOOKANDFEEL_SKINTHEME)
    {
      CSettingString* skinColorsSetting = static_cast<CSettingString*>(CSettings::GetInstance().GetSetting(CSettings::SETTING_LOOKANDFEEL_SKINCOLORS));
      m_ignoreSkinSettingChanges = true;

      // we also need to adjust the skin color setting
      std::string colorTheme = ((CSettingString*)setting)->GetValue();
      URIUtils::RemoveExtension(colorTheme);
      if (setting->IsDefault() || StringUtils::EqualsNoCase(colorTheme, "Textures"))
        skinColorsSetting->Reset();
      else
        skinColorsSetting->SetValue(colorTheme);
    }

    m_ignoreSkinSettingChanges = false;

    if (g_SkinInfo)
    {
      // now we can finally reload skins
      std::string builtin("ReloadSkin");
      if (settingId == CSettings::SETTING_LOOKANDFEEL_SKIN && m_confirmSkinChange)
        builtin += "(confirm)";
      CApplicationMessenger::GetInstance().PostMsg(TMSG_EXECUTE_BUILT_IN, -1, -1, nullptr, builtin);
    }
  }
  else if (settingId == CSettings::SETTING_LOOKANDFEEL_SKINZOOM)
  {
    CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_WINDOW_RESIZE);
    g_windowManager.SendThreadMessage(msg);
  }
  else if (StringUtils::StartsWithNoCase(settingId, "audiooutput."))
  {
    if (settingId == CSettings::SETTING_AUDIOOUTPUT_DSPADDONSENABLED)
    {
      if (((CSettingBool *) setting)->GetValue())
      {
        CApplicationMessenger::GetInstance().PostMsg(TMSG_SETAUDIODSPSTATE, ACTIVE_AE_DSP_STATE_ON, ACTIVE_AE_DSP_SYNC_ACTIVATE);
        CApplicationMessenger::GetInstance().PostMsg(TMSG_MEDIA_RESTART); // send non blocking media restart message
      }
      else
      {
        CAEFactory::OnSettingsChange(settingId);
        CApplicationMessenger::GetInstance().PostMsg(TMSG_MEDIA_RESTART); // send non blocking media restart message
        CApplicationMessenger::GetInstance().PostMsg(TMSG_SETAUDIODSPSTATE, ACTIVE_AE_DSP_STATE_OFF);
      }
      return;
    }

    // AE is master of audio settings and needs to be informed first
    CAEFactory::OnSettingsChange(settingId);

    if (settingId == CSettings::SETTING_AUDIOOUTPUT_GUISOUNDMODE)
    {
      CAEFactory::SetSoundMode(((CSettingInt*)setting)->GetValue());
    }
    // this tells player whether to open an audio stream passthrough or PCM
    // if this is changed, audio stream has to be reopened
    else if (settingId == CSettings::SETTING_AUDIOOUTPUT_PASSTHROUGH)
    {
      CApplicationMessenger::GetInstance().PostMsg(TMSG_MEDIA_RESTART);
    }
  }
  else if (StringUtils::EqualsNoCase(settingId, CSettings::SETTING_MUSICPLAYER_REPLAYGAINTYPE))
    m_replayGainSettings.iType = ((CSettingInt*)setting)->GetValue();
  else if (StringUtils::EqualsNoCase(settingId, CSettings::SETTING_MUSICPLAYER_REPLAYGAINPREAMP))
    m_replayGainSettings.iPreAmp = ((CSettingInt*)setting)->GetValue();
  else if (StringUtils::EqualsNoCase(settingId, CSettings::SETTING_MUSICPLAYER_REPLAYGAINNOGAINPREAMP))
    m_replayGainSettings.iNoGainPreAmp = ((CSettingInt*)setting)->GetValue();
}

void CApplication::OnSettingAction(const CSetting *setting)
{
  if (setting == NULL)
    return;

  const std::string &settingId = setting->GetId();
  if (settingId == CSettings::SETTING_LOOKANDFEEL_SKINSETTINGS)
    g_windowManager.ActivateWindow(WINDOW_SKIN_SETTINGS);
  else if (settingId == CSettings::SETTING_SCREENSAVER_PREVIEW)
    ActivateScreenSaver(true);
  else if (settingId == CSettings::SETTING_SCREENSAVER_SETTINGS)
  {
    AddonPtr addon;
    if (CAddonMgr::GetInstance().GetAddon(CSettings::GetInstance().GetString(CSettings::SETTING_SCREENSAVER_MODE), addon, ADDON_SCREENSAVER))
      CGUIDialogAddonSettings::ShowAndGetInput(addon);
  }
  else if (settingId == CSettings::SETTING_AUDIOCDS_SETTINGS)
  {
    AddonPtr addon;
    if (CAddonMgr::GetInstance().GetAddon(CSettings::GetInstance().GetString(CSettings::SETTING_AUDIOCDS_ENCODER), addon, ADDON_AUDIOENCODER))
      CGUIDialogAddonSettings::ShowAndGetInput(addon);
  }
  else if (settingId == CSettings::SETTING_VIDEOSCREEN_GUICALIBRATION)
    g_windowManager.ActivateWindow(WINDOW_SCREEN_CALIBRATION);
  else if (settingId == CSettings::SETTING_VIDEOSCREEN_TESTPATTERN)
    g_windowManager.ActivateWindow(WINDOW_TEST_PATTERN);
  else if (settingId == CSettings::SETTING_SOURCE_VIDEOS)
  {
    std::vector<std::string> params{"library://video/files.xml", "return"};
    g_windowManager.ActivateWindow(WINDOW_VIDEO_NAV, params);
  }
  else if (settingId == CSettings::SETTING_SOURCE_MUSIC)
  {
    std::vector<std::string> params{"library://music/files.xml", "return"};
    g_windowManager.ActivateWindow(WINDOW_MUSIC_NAV, params);
  }
  else if (settingId == CSettings::SETTING_SOURCE_PICTURES)
    g_windowManager.ActivateWindow(WINDOW_PICTURES);
}

bool CApplication::OnSettingUpdate(CSetting* &setting, const char *oldSettingId, const TiXmlNode *oldSettingNode)
{
  if (setting == NULL)
    return false;

#if defined(HAS_LIBAMCODEC)
  if (setting->GetId() == CSettings::SETTING_VIDEOPLAYER_USEAMCODEC)
  {
    // Do not permit amcodec to be used on non-aml platforms.
    // The setting will be hidden but the default value is true,
    // so change it to false.
    if (!aml_present())
    {
      CSettingBool *useamcodec = (CSettingBool*)setting;
      return useamcodec->SetValue(false);
    }
  }
#endif
#if defined(TARGET_DARWIN_OSX)
  if (setting->GetId() == CSettings::SETTING_AUDIOOUTPUT_AUDIODEVICE)
  {
    CSettingString *audioDevice = (CSettingString*)setting;
    // Gotham and older didn't enumerate audio devices per stream on osx
    // add stream0 per default which should be ok for all old settings.
    if (!StringUtils::EqualsNoCase(audioDevice->GetValue(), "DARWINOSX:default") && 
        StringUtils::FindWords(audioDevice->GetValue().c_str(), ":stream") == std::string::npos)
    {
      std::string newSetting = audioDevice->GetValue();
      newSetting += ":stream0";
      return audioDevice->SetValue(newSetting);
    }
  }
#endif

  return false;
}

bool CApplication::OnSettingsSaving() const
{
  // don't save settings when we're busy stopping the application
  // a lot of screens try to save settings on deinit and deinit is
  // called for every screen when the application is stopping
  if (m_bStop)
    return false;

  return true;
}

void CApplication::ReloadSkin(bool confirm/*=false*/)
{
  if (!g_SkinInfo || m_bInitializing)
    return; // Don't allow reload before skin is loaded by system

  std::string oldSkin = g_SkinInfo->ID();

  CGUIMessage msg(GUI_MSG_LOAD_SKIN, -1, g_windowManager.GetActiveWindow());
  g_windowManager.SendMessage(msg);

  std::string newSkin = CSettings::GetInstance().GetString(CSettings::SETTING_LOOKANDFEEL_SKIN);
  if (LoadSkin(newSkin))
  {
    /* The Reset() or SetString() below will cause recursion, so the m_confirmSkinChange boolean is set so as to not prompt the
       user as to whether they want to keep the current skin. */
    if (confirm && m_confirmSkinChange)
    {
      if (HELPERS::ShowYesNoDialogText(CVariant{13123}, CVariant{13111}, CVariant{""}, CVariant{""}, 10000) != 
        DialogResponse::YES)
      {
        m_confirmSkinChange = false;
        CSettings::GetInstance().SetString(CSettings::SETTING_LOOKANDFEEL_SKIN, oldSkin);
      }
    }
  }
  else
  {
    // skin failed to load - we revert to the default only if we didn't fail loading the default
    std::string defaultSkin = ((CSettingString*)CSettings::GetInstance().GetSetting(CSettings::SETTING_LOOKANDFEEL_SKIN))->GetDefault();
    if (newSkin != defaultSkin)
    {
      m_confirmSkinChange = false;
      CSettings::GetInstance().GetSetting(CSettings::SETTING_LOOKANDFEEL_SKIN)->Reset();
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Error, g_localizeStrings.Get(24102), g_localizeStrings.Get(24103));
    }
  }
  m_confirmSkinChange = true;
}

bool CApplication::Load(const TiXmlNode *settings)
{
  if (settings == NULL)
    return false;

  const TiXmlElement *audioElement = settings->FirstChildElement("audio");
  if (audioElement != NULL)
  {
    XMLUtils::GetBoolean(audioElement, "mute", m_muted);
    if (!XMLUtils::GetFloat(audioElement, "fvolumelevel", m_volumeLevel, VOLUME_MINIMUM, VOLUME_MAXIMUM))
      m_volumeLevel = VOLUME_MAXIMUM;
  }

  return true;
}

bool CApplication::Save(TiXmlNode *settings) const
{
  if (settings == NULL)
    return false;

  TiXmlElement volumeNode("audio");
  TiXmlNode *audioNode = settings->InsertEndChild(volumeNode);
  if (audioNode == NULL)
    return false;

  XMLUtils::SetBoolean(audioNode, "mute", m_muted);
  XMLUtils::SetFloat(audioNode, "fvolumelevel", m_volumeLevel);

  return true;
}

bool CApplication::LoadSkin(const std::string& skinID)
{
  SkinPtr skin;
  {
    AddonPtr addon;
    if (!CAddonMgr::GetInstance().GetAddon(skinID, addon, ADDON_SKIN))
      return false;
    skin = std::static_pointer_cast<ADDON::CSkinInfo>(addon);
  }

  bool bPreviousPlayingState=false;
  bool bPreviousRenderingState=false;
  if (m_pPlayer->IsPlayingVideo())
  {
    bPreviousPlayingState = !m_pPlayer->IsPausedPlayback();
    if (bPreviousPlayingState)
      m_pPlayer->Pause();
    m_pPlayer->FlushRenderer();
    if (g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
    {
      g_windowManager.ActivateWindow(WINDOW_HOME);
      bPreviousRenderingState = true;
    }
  }
  // close the music and video overlays (they're re-opened automatically later)
  CSingleLock lock(g_graphicsContext);

  // save the current window details and focused control
  int currentWindow = g_windowManager.GetActiveWindow();
  int iCtrlID = -1;
  CGUIWindow* pWindow = g_windowManager.GetWindow(currentWindow);
  if (pWindow)
    iCtrlID = pWindow->GetFocusedControlID();
  std::vector<int> currentModelessWindows;
  g_windowManager.GetActiveModelessWindows(currentModelessWindows);

  UnloadSkin();

  skin->Start();

  // migrate any skin-specific settings that are still stored in guisettings.xml
  CSkinSettings::GetInstance().MigrateSettings(skin);

  // check if the skin has been properly loaded and if it has a Home.xml
  if (!skin->HasSkinFile("Home.xml"))
  {
    CLog::Log(LOGERROR, "failed to load requested skin '%s'", skin->ID().c_str());
    return false;
  }

  CLog::Log(LOGINFO, "  load skin from: %s (version: %s)", skin->Path().c_str(), skin->Version().asString().c_str());
  g_SkinInfo = skin;

  CLog::Log(LOGINFO, "  load fonts for skin...");
  g_graphicsContext.SetMediaDir(skin->Path());
  g_directoryCache.ClearSubPaths(skin->Path());

  g_colorManager.Load(CSettings::GetInstance().GetString(CSettings::SETTING_LOOKANDFEEL_SKINCOLORS));

  g_fontManager.LoadFonts(CSettings::GetInstance().GetString(CSettings::SETTING_LOOKANDFEEL_FONT));

  // load in the skin strings
  std::string langPath = URIUtils::AddFileToFolder(skin->Path(), "language");
  URIUtils::AddSlashAtEnd(langPath);

  g_localizeStrings.LoadSkinStrings(langPath, CSettings::GetInstance().GetString(CSettings::SETTING_LOCALE_LANGUAGE));

  g_SkinInfo->LoadIncludes();

  int64_t start;
  start = CurrentHostCounter();

  CLog::Log(LOGINFO, "  load new skin...");

  // Load the user windows
  LoadUserWindows();

  int64_t end, freq;
  end = CurrentHostCounter();
  freq = CurrentHostFrequency();
  CLog::Log(LOGDEBUG,"Load Skin XML: %.2fms", 1000.f * (end - start) / freq);

  CLog::Log(LOGINFO, "  initialize new skin...");
  g_windowManager.AddMsgTarget(this);
  g_windowManager.AddMsgTarget(&g_playlistPlayer);
  g_windowManager.AddMsgTarget(&g_infoManager);
  g_windowManager.AddMsgTarget(&g_fontManager);
  g_windowManager.AddMsgTarget(&CStereoscopicsManager::GetInstance());
  g_windowManager.SetCallback(*this);
  g_windowManager.Initialize();
  CTextureCache::GetInstance().Initialize();
  g_audioManager.Enable(true);
  g_audioManager.Load();

  if (g_SkinInfo->HasSkinFile("DialogFullScreenInfo.xml"))
    g_windowManager.Add(new CGUIDialogFullScreenInfo);

  CLog::Log(LOGINFO, "  skin loaded...");

  // leave the graphics lock
  lock.Leave();

  // restore windows
  if (currentWindow != WINDOW_INVALID)
  {
    g_windowManager.ActivateWindow(currentWindow);
    for (unsigned int i = 0; i < currentModelessWindows.size(); i++)
    {
      CGUIDialog *dialog = (CGUIDialog *)g_windowManager.GetWindow(currentModelessWindows[i]);
      if (dialog)
        dialog->Open();
    }
    if (iCtrlID != -1)
    {
      pWindow = g_windowManager.GetWindow(currentWindow);
      if (pWindow && pWindow->HasSaveLastControl())
      {
        CGUIMessage msg(GUI_MSG_SETFOCUS, currentWindow, iCtrlID, 0);
        pWindow->OnMessage(msg);
      }
    }
  }

  if (m_pPlayer->IsPlayingVideo())
  {
    if (bPreviousPlayingState)
      m_pPlayer->Pause();
    if (bPreviousRenderingState)
      g_windowManager.ActivateWindow(WINDOW_FULLSCREEN_VIDEO);
  }
  return true;
}

void CApplication::UnloadSkin(bool forReload /* = false */)
{
  CLog::Log(LOGINFO, "Unloading old skin %s...", forReload ? "for reload " : "");

  if (g_SkinInfo != nullptr && m_saveSkinOnUnloading)
    g_SkinInfo->SaveSettings();
  else if (!m_saveSkinOnUnloading)
    m_saveSkinOnUnloading = true;

  g_audioManager.Enable(false);

  g_windowManager.DeInitialize();
  CTextureCache::GetInstance().Deinitialize();

  // remove the skin-dependent window
  g_windowManager.Delete(WINDOW_DIALOG_FULLSCREEN_INFO);

  g_TextureManager.Cleanup();
  g_largeTextureManager.CleanupUnusedImages(true);

  g_fontManager.Clear();

  g_colorManager.Clear();

  g_infoManager.Clear();

//  The g_SkinInfo shared_ptr ought to be reset here
// but there are too many places it's used without checking for NULL
// and as a result a race condition on exit can cause a crash.
}

bool CApplication::LoadUserWindows()
{
  // Start from wherever home.xml is
  std::vector<std::string> vecSkinPath;
  g_SkinInfo->GetSkinPaths(vecSkinPath);
  for (unsigned int i = 0;i < vecSkinPath.size();++i)
  {
    CLog::Log(LOGINFO, "Loading user windows, path %s", vecSkinPath[i].c_str());
    CFileItemList items;
    if (CDirectory::GetDirectory(vecSkinPath[i], items, ".xml", DIR_FLAG_NO_FILE_DIRS))
    {
      for (int i = 0; i < items.Size(); ++i)
      {
        if (items[i]->m_bIsFolder)
          continue;
        std::string skinFile = URIUtils::GetFileName(items[i]->GetPath());
        if (StringUtils::StartsWithNoCase(skinFile, "custom"))
        {
          CXBMCTinyXML xmlDoc;
          if (!xmlDoc.LoadFile(items[i]->GetPath()))
          {
            CLog::Log(LOGERROR, "unable to load: %s, Line %d\n%s", items[i]->GetPath().c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
            continue;
          }

          // Root element should be <window>
          TiXmlElement* pRootElement = xmlDoc.RootElement();
          std::string strValue = pRootElement->Value();
          if (!StringUtils::EqualsNoCase(strValue, "window"))
          {
            CLog::Log(LOGERROR, "file: %s doesnt contain <window>", skinFile.c_str());
            continue;
          }

          // Read the <type> element to get the window type to create
          // If no type is specified, create a CGUIWindow as default
          CGUIWindow* pWindow = NULL;
          std::string strType;
          if (pRootElement->Attribute("type"))
            strType = pRootElement->Attribute("type");
          else
          {
            const TiXmlNode *pType = pRootElement->FirstChild("type");
            if (pType && pType->FirstChild())
              strType = pType->FirstChild()->Value();
          }
          int id = WINDOW_INVALID;
          if (!pRootElement->Attribute("id", &id))
          {
            const TiXmlNode *pType = pRootElement->FirstChild("id");
            if (pType && pType->FirstChild())
              id = atol(pType->FirstChild()->Value());
          }
          std::string visibleCondition;
          CGUIControlFactory::GetConditionalVisibility(pRootElement, visibleCondition);

          if (StringUtils::EqualsNoCase(strType, "dialog"))
            pWindow = new CGUIDialog(id + WINDOW_HOME, skinFile, visibleCondition.empty() ? DialogModalityType::MODAL : DialogModalityType::MODELESS);
          else if (StringUtils::EqualsNoCase(strType, "submenu"))
            pWindow = new CGUIDialogSubMenu(id + WINDOW_HOME, skinFile);
          else if (StringUtils::EqualsNoCase(strType, "buttonmenu"))
            pWindow = new CGUIDialogButtonMenu(id + WINDOW_HOME, skinFile);
          else
            pWindow = new CGUIWindow(id + WINDOW_HOME, skinFile);

          // Check to make sure the pointer isn't still null
          if (pWindow == NULL)
          {
            CLog::Log(LOGERROR, "Out of memory / Failed to create new object in LoadUserWindows");
            return false;
          }
          if (id == WINDOW_INVALID || g_windowManager.GetWindow(WINDOW_HOME + id))
          {
            delete pWindow;
            continue;
          }
          pWindow->SetVisibleCondition(visibleCondition);
          pWindow->SetLoadType(CGUIWindow::KEEP_IN_MEMORY);
          g_windowManager.AddCustomWindow(pWindow);
        }
      }
    }
  }
  return true;
}

float CApplication::GetDimScreenSaverLevel() const
{
  if (!m_bScreenSave || !m_screenSaver ||
      (m_screenSaver->ID() != "screensaver.xbmc.builtin.dim" &&
       m_screenSaver->ID() != "screensaver.xbmc.builtin.black" &&
       !m_screenSaver->ID().empty()))
    return 0;

  if (!m_screenSaver->GetSetting("level").empty())
    return 100.0f - (float)atof(m_screenSaver->GetSetting("level").c_str());
  return 100.0f;
}

void CApplication::Render()
{
  // do not render if we are stopped or in background
  if (m_bStop)
    return;

  bool hasRendered = false;

  // Whether externalplayer is playing and we're unfocused
  bool extPlayerActive = m_pPlayer->IsExternalPlaying() && !m_AppFocused;

  if (!extPlayerActive && g_graphicsContext.IsFullScreenVideo() && !m_pPlayer->IsPausedPlayback())
  {
    ResetScreenSaver();
  }

  if(!g_Windowing.BeginRender())
    return;

  CDirtyRegionList dirtyRegions;

  // render gui layer
  if (!m_skipGuiRender)
  {
    dirtyRegions = g_windowManager.GetDirty();
    if (g_graphicsContext.GetStereoMode())
    {
      g_graphicsContext.SetStereoView(RENDER_STEREO_VIEW_LEFT);
      hasRendered |= g_windowManager.Render();

      if (g_graphicsContext.GetStereoMode() != RENDER_STEREO_MODE_MONO)
      {
        g_graphicsContext.SetStereoView(RENDER_STEREO_VIEW_RIGHT);
        hasRendered |= g_windowManager.Render();
      }
      g_graphicsContext.SetStereoView(RENDER_STEREO_VIEW_OFF);
    }
    else
    {
      hasRendered |= g_windowManager.Render();
    }
    // execute post rendering actions (finalize window closing)
    g_windowManager.AfterRender();

    m_lastRenderTime = XbmcThreads::SystemClockMillis();
  }

  // render video layer
  g_windowManager.RenderEx();

  g_Windowing.EndRender();

  // reset our info cache - we do this at the end of Render so that it is
  // fresh for the next process(), or after a windowclose animation (where process()
  // isn't called)
  g_infoManager.ResetCache();

  if (hasRendered)
  {
    g_infoManager.UpdateFPS();
  }

  g_graphicsContext.Flip(hasRendered, m_pPlayer->IsRenderingVideoLayer());

  CTimeUtils::UpdateFrameTime(hasRendered);
}

void CApplication::SetStandAlone(bool value)
{
  g_advancedSettings.m_handleMounting = m_bStandalone = value;
}


// OnAppCommand is called in response to a XBMC_APPCOMMAND event.
// This needs to return true if it processed the appcommand or false if it didn't
bool CApplication::OnAppCommand(const CAction &action)
{
  // Reset the screen saver
  ResetScreenSaver();

  // If we were currently in the screen saver wake up and don't process the appcommand
  if (WakeUpScreenSaverAndDPMS())
    return true;

  // The action ID is the APPCOMMAND code. We need to retrieve the action
  // associated with this appcommand from the mapping table.
  uint32_t appcmd = action.GetID();
  CKey key(appcmd | KEY_APPCOMMAND, (unsigned int) 0);
  int iWin = g_windowManager.GetActiveWindow() & WINDOW_ID_MASK;
  CAction appcmdaction = CButtonTranslator::GetInstance().GetAction(iWin, key);

  // If we couldn't find an action return false to indicate we have not
  // handled this appcommand
  if (!appcmdaction.GetID())
  {
    CLog::LogF(LOGDEBUG, "unknown appcommand %d", appcmd);
    return false;
  }

  // Process the appcommand
  CLog::LogF(LOGDEBUG, "appcommand %d, trying action %s", appcmd, appcmdaction.GetName().c_str());
  OnAction(appcmdaction);

  // Always return true regardless of whether the action succeeded or not.
  // This stops Windows handling the appcommand itself.
  return true;
}

bool CApplication::OnAction(const CAction &action)
{
  // special case for switching between GUI & fullscreen mode.
  if (action.GetID() == ACTION_SHOW_GUI)
  { // Switch to fullscreen mode if we can
    if (SwitchToFullScreen())
    {
      m_navigationTimer.StartZero();
      return true;
    }
  }

  if (action.GetID() == ACTION_TOGGLE_FULLSCREEN)
  {
    g_graphicsContext.ToggleFullScreen();
    m_pPlayer->TriggerUpdateResolution();
    return true;
  }

  if (action.IsMouse())
    CInputManager::GetInstance().SetMouseActive(true);

  
  if (action.GetID() == ACTION_CREATE_EPISODE_BOOKMARK)   
  {
    CGUIDialogVideoBookmarks::OnAddEpisodeBookmark();
  }
  if (action.GetID() == ACTION_CREATE_BOOKMARK)
  {
    CGUIDialogVideoBookmarks::OnAddBookmark();
  }
  
  // The action PLAYPAUSE behaves as ACTION_PAUSE if we are currently
  // playing or ACTION_PLAYER_PLAY if we are seeking (FF/RW) or not playing.
  if (action.GetID() == ACTION_PLAYER_PLAYPAUSE)
  {
    if (m_pPlayer->IsPlaying() && m_pPlayer->GetPlaySpeed() == 1)
      return OnAction(CAction(ACTION_PAUSE));
    else
      return OnAction(CAction(ACTION_PLAYER_PLAY));
  }

  //if the action would start or stop inertial scrolling
  //by gesture - bypass the normal OnAction handler of current window
  if( !m_pInertialScrollingHandler->CheckForInertialScrolling(&action) )
  {
    // in normal case
    // just pass the action to the current window and let it handle it
    if (g_windowManager.OnAction(action))
    {
      m_navigationTimer.StartZero();
      return true;
    }
  }

  // handle extra global presses

  // notify action listeners
  if (NotifyActionListeners(action))
    return true;
  
  // screenshot : take a screenshot :)
  if (action.GetID() == ACTION_TAKE_SCREENSHOT)
  {
    CScreenShot::TakeScreenshot();
    return true;
  }
  // built in functions : execute the built-in
  if (action.GetID() == ACTION_BUILT_IN_FUNCTION)
  {
    if (!CBuiltins::GetInstance().IsSystemPowerdownCommand(action.GetName()) ||
        g_PVRManager.CanSystemPowerdown())
    {
      CBuiltins::GetInstance().Execute(action.GetName());
      m_navigationTimer.StartZero();
    }
    return true;
  }

  // reload keymaps
  if (action.GetID() == ACTION_RELOAD_KEYMAPS)
  {
    CButtonTranslator::GetInstance().Clear();
    CButtonTranslator::GetInstance().Load();
  }

  // show info : Shows the current video or song information
  if (action.GetID() == ACTION_SHOW_INFO)
  {
    g_infoManager.ToggleShowInfo();
    return true;
  }

  if ((action.GetID() == ACTION_INCREASE_RATING || action.GetID() == ACTION_DECREASE_RATING) && m_pPlayer->IsPlayingAudio())
  {
    const CMusicInfoTag *tag = g_infoManager.GetCurrentSongTag();
    if (tag)
    {
      *m_itemCurrentFile->GetMusicInfoTag() = *tag;
      int userrating = tag->GetUserrating();
      bool needsUpdate(false);
      if (userrating > 0 && action.GetID() == ACTION_DECREASE_RATING)
      {
        m_itemCurrentFile->GetMusicInfoTag()->SetUserrating(userrating - 1);
        needsUpdate = true;
      }
      else if (userrating < 10 && action.GetID() == ACTION_INCREASE_RATING)
      {
        m_itemCurrentFile->GetMusicInfoTag()->SetUserrating(userrating + 1);
        needsUpdate = true;
      }
      if (needsUpdate)
      {
        CMusicDatabase db;
        if (db.Open())      // OpenForWrite() ?
        {
          db.SetSongUserrating(m_itemCurrentFile->GetPath(), m_itemCurrentFile->GetMusicInfoTag()->GetUserrating());
          db.Close();
        }
        // send a message to all windows to tell them to update the fileitem (eg playlistplayer, media windows)
        CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_ITEM, 0, m_itemCurrentFile);
        g_windowManager.SendMessage(msg);
      }
    }
    return true;
  }
  else if ((action.GetID() == ACTION_INCREASE_RATING || action.GetID() == ACTION_DECREASE_RATING) && m_pPlayer->IsPlayingVideo())
  {
    const CVideoInfoTag *tag = g_infoManager.GetCurrentMovieTag();
    if (tag)
    {
      *m_itemCurrentFile->GetVideoInfoTag() = *tag;
      int rating = tag->m_iUserRating;
      bool needsUpdate(false);
      if (rating > 1 && action.GetID() == ACTION_DECREASE_RATING)
      {
        m_itemCurrentFile->GetVideoInfoTag()->m_iUserRating = rating - 1;
        needsUpdate = true;
      }
      else if (rating < 10 && action.GetID() == ACTION_INCREASE_RATING)
      {
        m_itemCurrentFile->GetVideoInfoTag()->m_iUserRating = rating + 1;
        needsUpdate = true;
      }
      if (needsUpdate)
      {
        CVideoDatabase db;
        if (db.Open())
        {
          db.SetVideoUserRating(m_itemCurrentFile->GetVideoInfoTag()->m_iDbId, m_itemCurrentFile->GetVideoInfoTag()->m_iUserRating, m_itemCurrentFile->GetVideoInfoTag()->m_type);
          db.Close();
        }
        // send a message to all windows to tell them to update the fileitem (eg playlistplayer, media windows)
        CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_ITEM, 0, m_itemCurrentFile);
        g_windowManager.SendMessage(msg);
      }
    }
    return true;
  }

  // Now check with the playlist player if action can be handled.
  // In case of the action PREV_ITEM, we only allow the playlist player to take it if we're less than 3 seconds into playback.
  if (!(action.GetID() == ACTION_PREV_ITEM && m_pPlayer->CanSeek() && GetTime() > 3) )
  {
    if (g_playlistPlayer.OnAction(action))
      return true;
  }

  // Now check with the player if action can be handled.
  bool bIsPlayingPVRChannel = (g_PVRManager.IsStarted() && g_application.CurrentFileItem().IsPVRChannel());
  if (g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO ||
      (g_windowManager.GetActiveWindow() == WINDOW_VISUALISATION && bIsPlayingPVRChannel) ||
      ((g_windowManager.GetActiveWindow() == WINDOW_DIALOG_VIDEO_OSD || (g_windowManager.GetActiveWindow() == WINDOW_DIALOG_MUSIC_OSD && bIsPlayingPVRChannel)) &&
        (action.GetID() == ACTION_NEXT_ITEM || action.GetID() == ACTION_PREV_ITEM || action.GetID() == ACTION_CHANNEL_UP || action.GetID() == ACTION_CHANNEL_DOWN)) ||
      action.GetID() == ACTION_STOP)
  {
    if (m_pPlayer->OnAction(action))
      return true;
    // Player ignored action; popup the OSD
    if ((action.GetID() == ACTION_MOUSE_MOVE && (action.GetAmount(2) || action.GetAmount(3)))  // filter "false" mouse move from touch
        || action.GetID() == ACTION_MOUSE_LEFT_CLICK)
    {
      CApplicationMessenger::GetInstance().PostMsg(TMSG_GUI_ACTION, WINDOW_INVALID, -1, static_cast<void*>(new CAction(ACTION_TRIGGER_OSD)));
    }
  }

  // stop : stops playing current audio song
  if (action.GetID() == ACTION_STOP)
  {
    StopPlaying();
    return true;
  }

  // In case the playlist player nor the player didn't handle PREV_ITEM, because we are past the 3 secs limit.
  // If so, we just jump to the start of the track.
  if (action.GetID() == ACTION_PREV_ITEM && m_pPlayer->CanSeek())
  {
    SeekTime(0);
    m_pPlayer->SetPlaySpeed(1);
    return true;
  }

  // forward action to graphic context and see if it can handle it
  if (CStereoscopicsManager::GetInstance().OnAction(action))
    return true;

  if (m_pPlayer->IsPlaying())
  {
    // forward channel switches to the player - he knows what to do
    if (action.GetID() == ACTION_CHANNEL_UP || action.GetID() == ACTION_CHANNEL_DOWN)
    {
      m_pPlayer->OnAction(action);
      return true;
    }

    // pause : toggle pause action
    if (action.GetID() == ACTION_PAUSE)
    {
      m_pPlayer->Pause();
      // go back to normal play speed on unpause
      if (!m_pPlayer->IsPaused() && m_pPlayer->GetPlaySpeed() != 1)
        m_pPlayer->SetPlaySpeed(1);

      g_audioManager.Enable(m_pPlayer->IsPaused());
      return true;
    }
    // play: unpause or set playspeed back to normal
    if (action.GetID() == ACTION_PLAYER_PLAY)
    {
      // if currently paused - unpause
      if (m_pPlayer->IsPaused())
        return OnAction(CAction(ACTION_PAUSE));
      // if we do a FF/RW then go back to normal speed
      if (m_pPlayer->GetPlaySpeed() != 1)
        m_pPlayer->SetPlaySpeed(1);
      return true;
    }
    if (!m_pPlayer->IsPaused() && m_pPlayer->CanFFRW())
    {
      if (action.GetID() == ACTION_PLAYER_FORWARD || action.GetID() == ACTION_PLAYER_REWIND)
      {
        float playSpeed = m_pPlayer->GetPlaySpeed();
        if (playSpeed >= 0.75 && playSpeed <= 1.55)
          playSpeed = 1;

        if (action.GetID() == ACTION_PLAYER_REWIND && (playSpeed == 1)) // Enables Rewinding
          playSpeed *= -2;
        else if (action.GetID() == ACTION_PLAYER_REWIND && playSpeed > 1) //goes down a notch if you're FFing
          playSpeed /= 2;
        else if (action.GetID() == ACTION_PLAYER_FORWARD && playSpeed < 1) //goes up a notch if you're RWing
          playSpeed /= 2;
        else
          playSpeed *= 2;

        if (action.GetID() == ACTION_PLAYER_FORWARD && playSpeed == -1) //sets iSpeed back to 1 if -1 (didn't plan for a -1)
          playSpeed = 1;
        if (playSpeed > 32 || playSpeed < -32)
          playSpeed = 1;

        m_pPlayer->SetPlaySpeed(playSpeed);
        return true;
      }
      else if ((action.GetAmount() || m_pPlayer->GetPlaySpeed() != 1) && (action.GetID() == ACTION_ANALOG_REWIND || action.GetID() == ACTION_ANALOG_FORWARD))
      {
        // calculate the speed based on the amount the button is held down
        int iPower = (int)(action.GetAmount() * MAX_FFWD_SPEED + 0.5f);
        // amount can be negative, for example rewind and forward share the same axis
        iPower = std::abs(iPower);
        // returns 0 -> MAX_FFWD_SPEED
        int iSpeed = 1 << iPower;
        if (iSpeed != 1 && action.GetID() == ACTION_ANALOG_REWIND)
          iSpeed = -iSpeed;
        g_application.m_pPlayer->SetPlaySpeed(iSpeed);
        if (iSpeed == 1)
          CLog::Log(LOGDEBUG,"Resetting playspeed");
        return true;
      }
    }
    // allow play to unpause
    else
    {
      if (action.GetID() == ACTION_PLAYER_PLAY)
      {
        // unpause, and set the playspeed back to normal
        m_pPlayer->Pause();
        g_audioManager.Enable(m_pPlayer->IsPaused());

        g_application.m_pPlayer->SetPlaySpeed(1);
        return true;
      }
    }

    // record current file
    if (action.GetID() == ACTION_RECORD)
    {
      if (m_pPlayer->CanRecord())
        m_pPlayer->Record(!m_pPlayer->IsRecording());
    }
  }


  if (action.GetID() == ACTION_SWITCH_PLAYER)
  {
    if(m_pPlayer->IsPlaying())
    {
      std::vector<std::string> players;
      CFileItem item(*m_itemCurrentFile.get());
      CPlayerCoreFactory::GetInstance().GetPlayers(item, players);
      std::string player = CPlayerCoreFactory::GetInstance().SelectPlayerDialog(players);
      if (!player.empty())
      {
        item.m_lStartOffset = (int)(GetTime() * 75);
        PlayFile(std::move(item), player, true);
      }
    }
    else
    {
      std::vector<std::string> players;
      CPlayerCoreFactory::GetInstance().GetRemotePlayers(players);
      std::string player = CPlayerCoreFactory::GetInstance().SelectPlayerDialog(players);
      if (!player.empty())
      {
        PlayFile(CFileItem(), player, false);
      }
    }
  }

  if (g_peripherals.OnAction(action))
    return true;

  if (action.GetID() == ACTION_MUTE)
  {
    ToggleMute();
    ShowVolumeBar(&action);
    return true;
  }

  if (action.GetID() == ACTION_TOGGLE_DIGITAL_ANALOG)
  {
    bool passthrough = CSettings::GetInstance().GetBool(CSettings::SETTING_AUDIOOUTPUT_PASSTHROUGH);
    CSettings::GetInstance().SetBool(CSettings::SETTING_AUDIOOUTPUT_PASSTHROUGH, !passthrough);

    if (g_windowManager.GetActiveWindow() == WINDOW_SETTINGS_SYSTEM)
    {
      CGUIMessage msg(GUI_MSG_WINDOW_INIT, 0,0,WINDOW_INVALID,g_windowManager.GetActiveWindow());
      g_windowManager.SendMessage(msg);
    }
    return true;
  }

  // Check for global volume control
  if ((action.GetAmount() && (action.GetID() == ACTION_VOLUME_UP || action.GetID() == ACTION_VOLUME_DOWN)) || action.GetID() == ACTION_VOLUME_SET)
  {
    if (!m_pPlayer->IsPassthrough())
    {
      if (m_muted)
        UnMute();
      float volume = m_volumeLevel;
      int volumesteps = CSettings::GetInstance().GetInt(CSettings::SETTING_AUDIOOUTPUT_VOLUMESTEPS);
      // sanity check
      if (volumesteps == 0)
        volumesteps = 90;

// Android has steps based on the max available volume level
#if defined(TARGET_ANDROID)
      float step = (VOLUME_MAXIMUM - VOLUME_MINIMUM) / CXBMCApp::GetMaxSystemVolume();
#else
      float step   = (VOLUME_MAXIMUM - VOLUME_MINIMUM) / volumesteps;

      if (action.GetRepeat())
        step *= action.GetRepeat() * 50; // 50 fps
#endif
      if (action.GetID() == ACTION_VOLUME_UP)
        volume += (float)(action.GetAmount() * action.GetAmount() * step);
      else if (action.GetID() == ACTION_VOLUME_DOWN)
        volume -= (float)(action.GetAmount() * action.GetAmount() * step);
      else
        volume = action.GetAmount() * step;
      if (volume != m_volumeLevel)
        SetVolume(volume, false);
    }
    // show visual feedback of volume or passthrough indicator
    ShowVolumeBar(&action);
    return true;
  }
  if (action.GetID() == ACTION_GUIPROFILE_BEGIN)
  {
    CGUIControlProfiler::Instance().SetOutputFile(CSpecialProtocol::TranslatePath("special://home/guiprofiler.xml"));
    CGUIControlProfiler::Instance().Start();
    return true;
  }
  if (action.GetID() == ACTION_SHOW_PLAYLIST)
  {
    int iPlaylist = g_playlistPlayer.GetCurrentPlaylist();
    if (iPlaylist == PLAYLIST_VIDEO && g_windowManager.GetActiveWindow() != WINDOW_VIDEO_PLAYLIST)
      g_windowManager.ActivateWindow(WINDOW_VIDEO_PLAYLIST);
    else if (iPlaylist == PLAYLIST_MUSIC && g_windowManager.GetActiveWindow() != WINDOW_MUSIC_PLAYLIST)
      g_windowManager.ActivateWindow(WINDOW_MUSIC_PLAYLIST);
    return true;
  }
  return false;
}

int CApplication::GetMessageMask()
{
  return TMSG_MASK_APPLICATION;
}

void CApplication::OnApplicationMessage(ThreadMessage* pMsg)
{
  switch (pMsg->dwMessage)
  {
  case TMSG_POWERDOWN:
    Stop(EXITCODE_POWERDOWN);
    g_powerManager.Powerdown();
    break;

  case TMSG_QUIT:
    Stop(EXITCODE_QUIT);
    break;
  
  case TMSG_SHUTDOWN:
    HandleShutdownMessage();
    break;

  case TMSG_RENDERER_FLUSH:
    m_pPlayer->FlushRenderer();
    break;

  case TMSG_HIBERNATE:
    g_powerManager.Hibernate();
    break;

  case TMSG_SUSPEND:
    g_powerManager.Suspend();
    break;

  case TMSG_RESTART:
  case TMSG_RESET:
    Stop(EXITCODE_REBOOT);
    g_powerManager.Reboot();
    break;

  case TMSG_RESTARTAPP:
#if defined(TARGET_WINDOWS) || defined(TARGET_LINUX)
    Stop(EXITCODE_RESTARTAPP);
#endif
    break;

  case TMSG_INHIBITIDLESHUTDOWN:
    InhibitIdleShutdown(pMsg->param1 != 0);
    break;
  
  case TMSG_ACTIVATESCREENSAVER:
    ActivateScreenSaver();
    break;

  case TMSG_VOLUME_SHOW:
  {
    CAction action(pMsg->param1);
    ShowVolumeBar(&action);
  }
  break;

#ifdef TARGET_ANDROID
  case TMSG_DISPLAY_SETUP:
    // We might come from a refresh rate switch destroying the native window; use the context resolution
    *static_cast<bool*>(pMsg->lpVoid) = InitWindow(g_graphicsContext.GetVideoResolution());
    SetRenderGUI(true);
    break;

  case TMSG_DISPLAY_DESTROY:
    *static_cast<bool*>(pMsg->lpVoid) = DestroyWindow();
    SetRenderGUI(false);
    break;
#endif

  case TMSG_SETAUDIODSPSTATE:
    if(pMsg->param1 == ACTIVE_AE_DSP_STATE_ON)
      CServiceBroker::GetADSP().Activate();
    else if(pMsg->param1 == ACTIVE_AE_DSP_STATE_OFF)
      CServiceBroker::GetADSP().Deactivate();
    break;

  case TMSG_START_ANDROID_ACTIVITY:
  {
#if defined(TARGET_ANDROID)
    if (pMsg->params.size())
    {
      CXBMCApp::StartActivity(pMsg->params[0],
        pMsg->params.size() > 1 ? pMsg->params[1] : "",
        pMsg->params.size() > 2 ? pMsg->params[2] : "",
        pMsg->params.size() > 3 ? pMsg->params[3] : "");
    }
#endif
  }
  break;

  case TMSG_NETWORKMESSAGE:
    getNetwork().NetworkMessage((CNetwork::EMESSAGE)pMsg->param1, pMsg->param2);
    break;

  case TMSG_SETLANGUAGE:
    SetLanguage(pMsg->strParam);
    break;


  case TMSG_SWITCHTOFULLSCREEN:
    if (g_windowManager.GetActiveWindow() != WINDOW_FULLSCREEN_VIDEO)
      SwitchToFullScreen(true);
    break;

  case TMSG_VIDEORESIZE:
  {
    XBMC_Event newEvent;
    memset(&newEvent, 0, sizeof(newEvent));
    newEvent.type = XBMC_VIDEORESIZE;
    newEvent.resize.w = pMsg->param1;
    newEvent.resize.h = pMsg->param2;
    OnEvent(newEvent);
    g_windowManager.MarkDirty();
  }
    break;

  case TMSG_SETVIDEORESOLUTION:
    g_graphicsContext.SetVideoResolution(static_cast<RESOLUTION>(pMsg->param1), pMsg->param2 == 1);
    break;

  case TMSG_TOGGLEFULLSCREEN:
    g_graphicsContext.ToggleFullScreen();
    m_pPlayer->TriggerUpdateResolution();
    break;

  case TMSG_MINIMIZE:
    Minimize();
    break;

  case TMSG_EXECUTE_OS:
    /* Suspend AE temporarily so exclusive or hog-mode sinks */
    /* don't block external player's access to audio device  */
    if (!CAEFactory::Suspend())
    {
      CLog::Log(LOGNOTICE, "%s: Failed to suspend AudioEngine before launching external program", __FUNCTION__);
    }
#if defined( TARGET_POSIX) && !defined(TARGET_DARWIN)
    CUtil::RunCommandLine(pMsg->strParam.c_str(), (pMsg->param1 == 1));
#elif defined(TARGET_WINDOWS)
    CWIN32Util::XBMCShellExecute(pMsg->strParam.c_str(), (pMsg->param1 == 1));
#endif
    /* Resume AE processing of XBMC native audio */
    if (!CAEFactory::Resume())
    {
      CLog::Log(LOGFATAL, "%s: Failed to restart AudioEngine after return from external player", __FUNCTION__);
    }
    break;

  case TMSG_EXECUTE_SCRIPT:
    CScriptInvocationManager::GetInstance().ExecuteAsync(pMsg->strParam);
    break;

  case TMSG_EXECUTE_BUILT_IN:
    CBuiltins::GetInstance().Execute(pMsg->strParam.c_str());
    break;

  case TMSG_PICTURE_SHOW:
  {
    CGUIWindowSlideShow *pSlideShow = static_cast<CGUIWindowSlideShow *>(g_windowManager.GetWindow(WINDOW_SLIDESHOW));
    if (!pSlideShow) return;

    // stop playing file
    if (g_application.m_pPlayer->IsPlayingVideo()) g_application.StopPlaying();

    if (g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
      g_windowManager.PreviousWindow();

    g_application.ResetScreenSaver();
    g_application.WakeUpScreenSaverAndDPMS();

    g_graphicsContext.Lock();

    if (g_windowManager.GetActiveWindow() != WINDOW_SLIDESHOW)
      g_windowManager.ActivateWindow(WINDOW_SLIDESHOW);
    if (URIUtils::IsZIP(pMsg->strParam) || URIUtils::IsRAR(pMsg->strParam)) // actually a cbz/cbr
    {
      CFileItemList items;
      CURL pathToUrl;
      if (URIUtils::IsZIP(pMsg->strParam))
        pathToUrl = URIUtils::CreateArchivePath("zip", CURL(pMsg->strParam), "");
      else
        pathToUrl = URIUtils::CreateArchivePath("rar", CURL(pMsg->strParam), "");

      CUtil::GetRecursiveListing(pathToUrl.Get(), items, g_advancedSettings.m_pictureExtensions, XFILE::DIR_FLAG_NO_FILE_DIRS);
      if (items.Size() > 0)
      {
        pSlideShow->Reset();
        for (int i = 0; i<items.Size(); ++i)
        {
          pSlideShow->Add(items[i].get());
        }
        pSlideShow->Select(items[0]->GetPath());
      }
    }
    else
    {
      CFileItem item(pMsg->strParam, false);
      pSlideShow->Reset();
      pSlideShow->Add(&item);
      pSlideShow->Select(pMsg->strParam);
    }
    g_graphicsContext.Unlock();
  }
  break;

  case TMSG_PICTURE_SLIDESHOW:
  {
    CGUIWindowSlideShow *pSlideShow = static_cast<CGUIWindowSlideShow *>(g_windowManager.GetWindow(WINDOW_SLIDESHOW));
    if (!pSlideShow) return;

    if (g_application.m_pPlayer->IsPlayingVideo())
      g_application.StopPlaying();

    g_graphicsContext.Lock();
    pSlideShow->Reset();

    CFileItemList items;
    std::string strPath = pMsg->strParam;
    std::string extensions = g_advancedSettings.m_pictureExtensions;
    if (pMsg->param1)
      extensions += "|.tbn";
    CUtil::GetRecursiveListing(strPath, items, extensions);

    if (items.Size() > 0)
    {
      for (int i = 0; i<items.Size(); ++i)
        pSlideShow->Add(items[i].get());
      pSlideShow->StartSlideShow(); //Start the slideshow!
    }

    if (g_windowManager.GetActiveWindow() != WINDOW_SLIDESHOW)
    {
      if (items.Size() == 0)
      {
        CSettings::GetInstance().SetString(CSettings::SETTING_SCREENSAVER_MODE, "screensaver.xbmc.builtin.dim");
        g_application.ActivateScreenSaver();
      }
      else
        g_windowManager.ActivateWindow(WINDOW_SLIDESHOW);
    }

    g_graphicsContext.Unlock();
  }
  break;

  case TMSG_LOADPROFILE:
    CGUIWindowLoginScreen::LoadProfile(pMsg->param1);
    break;

  default:
    CLog::Log(LOGERROR, "%s: Unhandled threadmessage sent, %u", __FUNCTION__, pMsg->dwMessage);
    break;
  }
}

void CApplication::HandleShutdownMessage()
{
  switch (CSettings::GetInstance().GetInt(CSettings::SETTING_POWERMANAGEMENT_SHUTDOWNSTATE))
  {
  case POWERSTATE_SHUTDOWN:
    CApplicationMessenger::GetInstance().PostMsg(TMSG_POWERDOWN);
    break;

  case POWERSTATE_SUSPEND:
    CApplicationMessenger::GetInstance().PostMsg(TMSG_SUSPEND);
    break;

  case POWERSTATE_HIBERNATE:
    CApplicationMessenger::GetInstance().PostMsg(TMSG_HIBERNATE);
    break;

  case POWERSTATE_QUIT:
    CApplicationMessenger::GetInstance().PostMsg(TMSG_QUIT);
    break;

  case POWERSTATE_MINIMIZE:
    CApplicationMessenger::GetInstance().PostMsg(TMSG_MINIMIZE);
    break;

  default:
    CLog::Log(LOGERROR, "%s: No valid shutdownstate matched", __FUNCTION__);
    break;
  }
}

void CApplication::LockFrameMoveGuard()
{
  ++m_WaitingExternalCalls;
  m_frameMoveGuard.lock();
  ++m_ProcessedExternalCalls;
  g_graphicsContext.Lock();
};

void CApplication::UnlockFrameMoveGuard()
{
  --m_WaitingExternalCalls;
  g_graphicsContext.Unlock();
  m_frameMoveGuard.unlock();
};

void CApplication::FrameMove(bool processEvents, bool processGUI)
{
  MEASURE_FUNCTION;

  if (processEvents)
  {
    // currently we calculate the repeat time (ie time from last similar keypress) just global as fps
    float frameTime = m_frameTime.GetElapsedSeconds();
    m_frameTime.StartZero();
    // never set a frametime less than 2 fps to avoid problems when debuggin and on breaks
    if( frameTime > 0.5 ) frameTime = 0.5;

    if (processGUI && m_renderGUI)
    {
      g_graphicsContext.Lock();
      // check if there are notifications to display
      CGUIDialogKaiToast *toast = (CGUIDialogKaiToast *)g_windowManager.GetWindow(WINDOW_DIALOG_KAI_TOAST);
      if (toast && toast->DoWork())
      {
        if (!toast->IsDialogRunning())
        {
          toast->Open();
        }
      }
      g_graphicsContext.Unlock();
    }
    CWinEvents::MessagePump();

    CInputManager::GetInstance().Process(g_windowManager.GetActiveWindowID(), frameTime);

    if (processGUI && m_renderGUI)
    {
      m_pInertialScrollingHandler->ProcessInertialScroll(frameTime);
      CSeekHandler::GetInstance().FrameMove();
    }

    // Open the door for external calls e.g python exactly here.
    // Window size can be between 2 and 10ms and depends on number of continuous requests
    if (m_WaitingExternalCalls)
    {
      CSingleExit ex(g_graphicsContext);
      m_frameMoveGuard.unlock();
      // Calculate a window size between 2 and 10ms, 4 continuous requests let the window grow by 1ms
      unsigned int sleepTime = std::max(static_cast<unsigned int>(2), std::min(m_ProcessedExternalCalls >> 2, static_cast<unsigned int>(10)));
      Sleep(sleepTime);
      m_frameMoveGuard.lock();
    }
    else
      m_ProcessedExternalCalls = 0;
  }

  if (processGUI && m_renderGUI)
  {
    m_skipGuiRender = false;
    int fps = 0;

#if defined(TARGET_RASPBERRY_PI) || defined(HAS_IMXVPU)
    // This code reduces rendering fps of the GUI layer when playing videos in fullscreen mode
    // it makes only sense on architectures with multiple layers
    if (g_graphicsContext.IsFullScreenVideo() && !m_pPlayer->IsPausedPlayback() && m_pPlayer->IsRenderingVideoLayer())
      fps = CSettings::GetInstance().GetInt(CSettings::SETTING_VIDEOPLAYER_LIMITGUIUPDATE);
#endif

    unsigned int now = XbmcThreads::SystemClockMillis();
    unsigned int frameTime = now - m_lastRenderTime;
    if (fps > 0 && frameTime * fps < 1000)
      m_skipGuiRender = true;

    if (!m_bStop)
    {
      if (!m_skipGuiRender)
        g_windowManager.Process(CTimeUtils::GetFrameTime());
    }
    g_windowManager.FrameMove();
  }

  m_pPlayer->FrameMove();
}



bool CApplication::Cleanup()
{
  try
  {
    CLog::Log(LOGNOTICE, "unload skin");
    UnloadSkin();

    // stop all remaining scripts; must be done after skin has been unloaded,
    // not before some windows still need it when deinitializing during skin
    // unloading
    CScriptInvocationManager::GetInstance().Uninitialize();

    g_Windowing.DestroyRenderSystem();
    g_Windowing.DestroyWindow();
    g_Windowing.DestroyWindowSystem();
    g_windowManager.DestroyWindows();

    CLog::Log(LOGNOTICE, "unload sections");

#ifdef HAS_PERFORMANCE_SAMPLE
    CLog::Log(LOGNOTICE, "performance statistics");
    m_perfStats.DumpStats();
#endif

    //  Shutdown as much as possible of the
    //  application, to reduce the leaks dumped
    //  to the vc output window before calling
    //  _CrtDumpMemoryLeaks(). Most of the leaks
    //  shown are no real leaks, as parts of the app
    //  are still allocated.

    g_localizeStrings.Clear();
    g_LangCodeExpander.Clear();
    g_charsetConverter.clear();
    g_directoryCache.Clear();
    CButtonTranslator::GetInstance().Clear();
#ifdef HAS_EVENT_SERVER
    CEventServer::RemoveInstance();
#endif
    DllLoaderContainer::Clear();
    g_playlistPlayer.Clear();
    CSettings::GetInstance().Uninitialize();
    g_advancedSettings.Clear();

#ifdef TARGET_POSIX
    CXHandle::DumpObjectTracker();

#ifdef HAS_DVD_DRIVE
    CLibcdio::ReleaseInstance();
#endif
#endif 
#if defined(TARGET_ANDROID)
    // enable for all platforms once it's safe
    g_sectionLoader.UnloadAll();
#endif
#ifdef _CRTDBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
    while(1); // execution ends
#endif

    delete m_network;
    m_network = NULL;

    // Cleanup was called more than once on exit during my tests
    if (m_ServiceManager)
    {
      m_ServiceManager->Deinit();
      m_ServiceManager.reset();
    }

    return true;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "Exception in CApplication::Cleanup()");
    return false;
  }
}

void CApplication::Stop(int exitCode)
{
  try
  {
    m_frameMoveGuard.unlock();

    CVariant vExitCode(CVariant::VariantTypeObject);
    vExitCode["exitcode"] = exitCode;
    CAnnouncementManager::GetInstance().Announce(System, "xbmc", "OnQuit", vExitCode);

    // Abort any active screensaver
    WakeUpScreenSaverAndDPMS();

    SaveFileState(true);

    g_alarmClock.StopThread();

    if( m_bSystemScreenSaverEnable )
      g_Windowing.EnableSystemScreenSaver(true);

    CLog::Log(LOGNOTICE, "Storing total System Uptime");
    g_sysinfo.SetTotalUptime(g_sysinfo.GetTotalUptime() + (int)(CTimeUtils::GetFrameTime() / 60000));

    // Update the settings information (volume, uptime etc. need saving)
    if (CFile::Exists(CProfilesManager::GetInstance().GetSettingsFile()))
    {
      CLog::Log(LOGNOTICE, "Saving settings");
      CSettings::GetInstance().Save();
    }
    else
      CLog::Log(LOGNOTICE, "Not saving settings (settings.xml is not present)");

    // kodi may crash or deadlock during exit (shutdown / reboot) due to
    // either a bug in core or misbehaving addons. so try saving
    // skin settings early
    CLog::Log(LOGNOTICE, "Saving skin settings");
    if (g_SkinInfo != nullptr)
      g_SkinInfo->SaveSettings();

    m_bStop = true;
    m_AppFocused = false;
    m_ExitCode = exitCode;
    CLog::Log(LOGNOTICE, "stop all");

    // cancel any jobs from the jobmanager
    CJobManager::GetInstance().CancelJobs();

    // stop scanning before we kill the network and so on
    if (m_musicInfoScanner->IsScanning())
      m_musicInfoScanner->Stop(true);

    if (CVideoLibraryQueue::GetInstance().IsRunning())
      CVideoLibraryQueue::GetInstance().CancelAllJobs();

    CServiceBroker::GetADSP().Deactivate();
    CApplicationMessenger::GetInstance().Cleanup();

    CLog::Log(LOGNOTICE, "stop player");
    m_pPlayer->ClosePlayer();

    StopServices();

#ifdef HAS_ZEROCONF
    if(CZeroconfBrowser::IsInstantiated())
    {
      CLog::Log(LOGNOTICE, "stop zeroconf browser");
      CZeroconfBrowser::GetInstance()->Stop();
      CZeroconfBrowser::ReleaseInstance();
    }
#endif

    CLog::Log(LOGNOTICE, "clean cached files!");
#ifdef HAS_FILESYSTEM_RAR
    g_RarManager.ClearCache(true);
#endif

#ifdef HAS_FILESYSTEM_SFTP
    CSFTPSessionManager::DisconnectAllSessions();
#endif

#if defined(TARGET_POSIX) && defined(HAS_FILESYSTEM_SMB)
    smb.Deinit();
#endif

#if defined(TARGET_DARWIN_OSX)
    if (XBMCHelper::GetInstance().IsAlwaysOn() == false)
      XBMCHelper::GetInstance().Stop();
#endif

    g_mediaManager.Stop();

    // Stop services before unloading Python
    CAddonMgr::GetInstance().StopServices(false);

    // unregister action listeners
    UnregisterActionListener(&CSeekHandler::GetInstance());
    UnregisterActionListener(&CPlayerController::GetInstance());

    g_audioManager.DeInitialize();
    // shutdown the AudioEngine
    CAEFactory::Shutdown();
    CAEFactory::UnLoadEngine();

    CLog::Log(LOGNOTICE, "closing down remote control service");
    CInputManager::GetInstance().DisableRemoteControl();

    // unregister ffmpeg lock manager call back
    av_lockmgr_register(NULL);

    CLog::Log(LOGNOTICE, "stopped");
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "Exception in CApplication::Stop()");
  }

  cleanup_emu_environ();

  Sleep(200);
}

bool CApplication::PlayMedia(const CFileItem& item, const std::string &player, int iPlaylist)
{
  //If item is a plugin, expand out now and run ourselves again
  if (item.IsPlugin())
  {
    CFileItem item_new(item);
    if (XFILE::CPluginDirectory::GetPluginResult(item.GetPath(), item_new))
      return PlayMedia(item_new, player, iPlaylist);
    return false;
  }
  if (item.IsSmartPlayList())
  {
    CFileItemList items;
    CUtil::GetRecursiveListing(item.GetPath(), items, "", DIR_FLAG_NO_FILE_DIRS);
    if (items.Size())
    {
      CSmartPlaylist smartpl;
      //get name and type of smartplaylist, this will always succeed as GetDirectory also did this.
      smartpl.OpenAndReadName(item.GetURL());
      CPlayList playlist;
      playlist.Add(items);
      return ProcessAndStartPlaylist(smartpl.GetName(), playlist, (smartpl.GetType() == "songs" || smartpl.GetType() == "albums") ? PLAYLIST_MUSIC:PLAYLIST_VIDEO);
    }
  }
  else if (item.IsPlayList() || item.IsInternetStream())
  {
    CGUIDialogCache* dlgCache = new CGUIDialogCache(5000, g_localizeStrings.Get(10214), item.GetLabel());

    //is or could be a playlist
    std::unique_ptr<CPlayList> pPlayList (CPlayListFactory::Create(item));
    bool gotPlayList = (pPlayList.get() && pPlayList->Load(item.GetPath()));

    if (dlgCache)
    {
       dlgCache->Close();
       if (dlgCache->IsCanceled())
          return true;
    }

    if (gotPlayList)
    {

      if (iPlaylist != PLAYLIST_NONE)
      {
        int track=0;
        if (item.HasProperty("playlist_starting_track"))
          track = (int)item.GetProperty("playlist_starting_track").asInteger();
        return ProcessAndStartPlaylist(item.GetPath(), *pPlayList, iPlaylist, track);
      }
      else
      {
        CLog::Log(LOGWARNING, "CApplication::PlayMedia called to play a playlist %s but no idea which playlist to use, playing first item", item.GetPath().c_str());
        if(pPlayList->size())
          return PlayFile(*(*pPlayList)[0], "", false) == PLAYBACK_OK;
      }
    }
  }
  else if (item.IsPVR())
  {
    return g_PVRManager.PlayMedia(item);
  }

  //nothing special just play
  return PlayFile(item, player, false) == PLAYBACK_OK;
}

// PlayStack()
// For playing a multi-file video.  Particularly inefficient
// on startup, as we are required to calculate the length
// of each video, so we open + close each one in turn.
// A faster calculation of video time would improve this
// substantially.
// return value: same with PlayFile()
PlayBackRet CApplication::PlayStack(const CFileItem& item, bool bRestart)
{
  if (!item.IsStack())
    return PLAYBACK_FAIL;

  CVideoDatabase dbs;

  // case 1: stacked ISOs
  if (CFileItem(CStackDirectory::GetFirstStackedFile(item.GetPath()),false).IsDiscImage())
  {
    CStackDirectory dir;
    CFileItemList movieList;
    if (!dir.GetDirectory(item.GetURL(), movieList) || movieList.IsEmpty())
      return PLAYBACK_FAIL;

    // first assume values passed to the stack
    int selectedFile = item.m_lStartPartNumber;
    int startoffset = item.m_lStartOffset;

    // check if we instructed the stack to resume from default
    if (startoffset == STARTOFFSET_RESUME) // selected file is not specified, pick the 'last' resume point
    {
      if (dbs.Open())
      {
        CBookmark bookmark;
        std::string path = item.GetPath();
        if (item.HasProperty("original_listitem_url") && URIUtils::IsPlugin(item.GetProperty("original_listitem_url").asString()))
          path = item.GetProperty("original_listitem_url").asString();
        if( dbs.GetResumeBookMark(path, bookmark) )
        {
          startoffset = (int)(bookmark.timeInSeconds*75);
          selectedFile = bookmark.partNumber;
        }
        dbs.Close();
      }
      else
        CLog::LogF(LOGERROR, "Cannot open VideoDatabase");
    }

    // make sure that the selected part is within the boundaries
    if (selectedFile <= 0)
    {
      CLog::LogF(LOGWARNING, "Selected part %d out of range, playing part 1", selectedFile);
      selectedFile = 1;
    }
    else if (selectedFile > movieList.Size())
    {
      CLog::LogF(LOGWARNING, "Selected part %d out of range, playing part %d", selectedFile, movieList.Size());
      selectedFile = movieList.Size();
    }

    // set startoffset in movieitem, track stack item for updating purposes, and finally play disc part
    movieList[selectedFile - 1]->m_lStartOffset = startoffset > 0 ? STARTOFFSET_RESUME : 0;
    movieList[selectedFile - 1]->SetProperty("stackFileItemToUpdate", true);
    *m_stackFileItemToUpdate = item;
    return PlayFile(*(movieList[selectedFile - 1]), "");
  }
  // case 2: all other stacks
  else
  {
    LoadVideoSettings(item);
    
    // see if we have the info in the database
    //! @todo If user changes the time speed (FPS via framerate conversion stuff)
    //!       then these times will be wrong.
    //!       Also, this is really just a hack for the slow load up times we have
    //!       A much better solution is a fast reader of FPS and fileLength
    //!       that we can use on a file to get it's time.
    std::vector<int> times;
    bool haveTimes(false);
    CVideoDatabase dbs;
    if (dbs.Open())
    {
      haveTimes = dbs.GetStackTimes(item.GetPath(), times);
      dbs.Close();
    }


    // calculate the total time of the stack
    CStackDirectory dir;
    if (!dir.GetDirectory(item.GetURL(), *m_currentStack) || m_currentStack->IsEmpty())
      return PLAYBACK_FAIL;
    long totalTime = 0;
    for (int i = 0; i < m_currentStack->Size(); i++)
    {
      if (haveTimes)
        (*m_currentStack)[i]->m_lEndOffset = times[i];
      else
      {
        int duration;
        if (!CDVDFileInfo::GetFileDuration((*m_currentStack)[i]->GetPath(), duration))
        {
          m_currentStack->Clear();
          return PLAYBACK_FAIL;
        }
        totalTime += duration / 1000;
        (*m_currentStack)[i]->m_lEndOffset = totalTime;
        times.push_back(totalTime);
      }
    }

    double seconds = item.m_lStartOffset / 75.0;

    if (!haveTimes || item.m_lStartOffset == STARTOFFSET_RESUME )
    {  // have our times now, so update the dB
      if (dbs.Open())
      {
        if (!haveTimes && !times.empty())
          dbs.SetStackTimes(item.GetPath(), times);

        if (item.m_lStartOffset == STARTOFFSET_RESUME)
        {
          // can only resume seek here, not dvdstate
          CBookmark bookmark;
          std::string path = item.GetPath();
          if (item.HasProperty("original_listitem_url") && URIUtils::IsPlugin(item.GetProperty("original_listitem_url").asString()))
            path = item.GetProperty("original_listitem_url").asString();
          if (dbs.GetResumeBookMark(path, bookmark))
            seconds = bookmark.timeInSeconds;
          else
            seconds = 0.0f;
        }
        dbs.Close();
      }
    }

    m_itemCurrentFile.reset(new CFileItem(item));
    m_currentStackPosition = 0;

    if (seconds > 0)
    {
      // work out where to seek to
      for (int i = 0; i < m_currentStack->Size(); i++)
      {
        if (seconds < (*m_currentStack)[i]->m_lEndOffset)
        {
          CFileItem item(*(*m_currentStack)[i]);
          long start = (i > 0) ? (*m_currentStack)[i-1]->m_lEndOffset : 0;
          item.m_lStartOffset = (long)(seconds - start) * 75;
          m_currentStackPosition = i;
          return PlayFile(item, "", true);
        }
      }
    }

    return PlayFile(*(*m_currentStack)[0], "", true);
  }
  return PLAYBACK_FAIL;
}

PlayBackRet CApplication::PlayFile(CFileItem item, const std::string& player, bool bRestart)
{
  // Ensure the MIME type has been retrieved for http:// and shout:// streams
  if (item.GetMimeType().empty())
    item.FillInMimeType();

  if (!bRestart)
  {
    SaveFileState(true);

    // Switch to default options
    CMediaSettings::GetInstance().GetCurrentVideoSettings() = CMediaSettings::GetInstance().GetDefaultVideoSettings();
    CMediaSettings::GetInstance().GetCurrentAudioSettings() = CMediaSettings::GetInstance().GetDefaultAudioSettings();
    // see if we have saved options in the database

    m_pPlayer->SetPlaySpeed(1);

    m_itemCurrentFile.reset(new CFileItem(item));

    m_nextPlaylistItem = -1;
    m_currentStackPosition = 0;
    m_currentStack->Clear();

    if (item.IsVideo())
      CUtil::ClearSubtitles();
  }

  if (item.IsDiscStub())
  {
#ifdef HAS_DVD_DRIVE
    // Display the Play Eject dialog if there is any optical disc drive
    if (g_mediaManager.HasOpticalDrive())
    {
      if (CGUIDialogPlayEject::ShowAndGetInput(item))
        // PlayDiscAskResume takes path to disc. No parameter means default DVD drive.
        // Can't do better as CGUIDialogPlayEject calls CMediaManager::IsDiscInDrive, which assumes default DVD drive anyway
        return MEDIA_DETECT::CAutorun::PlayDiscAskResume() ? PLAYBACK_OK : PLAYBACK_FAIL;
    }
    else
#endif
      CGUIDialogOK::ShowAndGetInput(CVariant{435}, CVariant{436});

    return PLAYBACK_OK;
  }

  if (item.IsPlayList())
    return PLAYBACK_FAIL;

  if (item.IsPlugin())
  { // we modify the item so that it becomes a real URL
    CFileItem item_new(item);
    if (XFILE::CPluginDirectory::GetPluginResult(item.GetPath(), item_new))
      return PlayFile(std::move(item_new), player, false);
    return PLAYBACK_FAIL;
  }

  // a disc image might be Blu-Ray disc
  if (item.IsBDFile() || item.IsDiscImage())
  {
    //check if we must show the simplified bd menu
    if (!CGUIDialogSimpleMenu::ShowPlaySelection(const_cast<CFileItem&>(item)))
      return PLAYBACK_CANCELED;
  }

#ifdef HAS_UPNP
  if (URIUtils::IsUPnP(item.GetPath()))
  {
    CFileItem item_new(item);
    if (XFILE::CUPnPDirectory::GetResource(item.GetURL(), item_new))
      return PlayFile(std::move(item_new), player, false);
    return PLAYBACK_FAIL;
  }
#endif

  // if we have a stacked set of files, we need to setup our stack routines for
  // "seamless" seeking and total time of the movie etc.
  // will recall with restart set to true
  if (item.IsStack())
    return PlayStack(item, bRestart);

  CPlayerOptions options;

  if( item.HasProperty("StartPercent") )
  {
    double fallback = 0.0f;
    if(item.GetProperty("StartPercent").isString())
      fallback = (double)atof(item.GetProperty("StartPercent").asString().c_str());
    options.startpercent = item.GetProperty("StartPercent").asDouble(fallback);
  }

  if (bRestart)
  {
    // have to be set here due to playstack using this for starting the file
    options.starttime = item.m_lStartOffset / 75.0;
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0 && m_itemCurrentFile->m_lStartOffset != 0)
      m_itemCurrentFile->m_lStartOffset = STARTOFFSET_RESUME; // to force fullscreen switching
  }
  else
  {
    options.starttime = item.m_lStartOffset / 75.0;
    LoadVideoSettings(item);

    if (item.IsVideo())
    {
      // open the d/b and retrieve the bookmarks for the current movie
      CVideoDatabase dbs;
      dbs.Open();

      if( item.m_lStartOffset == STARTOFFSET_RESUME )
      {
        options.starttime = 0.0f;
        CBookmark bookmark;
        std::string path = item.GetPath();
        if (item.HasVideoInfoTag() && StringUtils::StartsWith(item.GetVideoInfoTag()->m_strFileNameAndPath, "removable://"))
          path = item.GetVideoInfoTag()->m_strFileNameAndPath;
        else if (item.HasProperty("original_listitem_url") && URIUtils::IsPlugin(item.GetProperty("original_listitem_url").asString()))
          path = item.GetProperty("original_listitem_url").asString();
        if(dbs.GetResumeBookMark(path, bookmark))
        {
          options.starttime = bookmark.timeInSeconds;
          options.state = bookmark.playerState;
        }
        /*
         override with information from the actual item if available.  We do this as the VFS (eg plugins)
         may set the resume point to override whatever XBMC has stored, yet we ignore it until now so that,
         should the playerState be required, it is fetched from the database.
         See the note in CGUIWindowVideoBase::ShowResumeMenu.
         */
        if (item.IsResumePointSet())
          options.starttime = item.GetCurrentResumeTime();
        else if (item.HasVideoInfoTag())
        {
          // No resume point is set, but check if this item is part of a multi-episode file
          const CVideoInfoTag *tag = item.GetVideoInfoTag();

          if (tag->m_iBookmarkId > 0)
          {
            CBookmark bookmark;
            dbs.GetBookMarkForEpisode(*tag, bookmark);
            options.starttime = bookmark.timeInSeconds;
            options.state = bookmark.playerState;
          }
        }
      }
      else if (item.HasVideoInfoTag())
      {
        const CVideoInfoTag *tag = item.GetVideoInfoTag();

        if (tag->m_iBookmarkId > 0)
        {
          CBookmark bookmark;
          dbs.GetBookMarkForEpisode(*tag, bookmark);
          options.starttime = bookmark.timeInSeconds;
          options.state = bookmark.playerState;
        }
      }

      dbs.Close();
    }
  }

  // this really aught to be inside !bRestart, but since PlayStack
  // uses that to init playback, we have to keep it outside
  int playlist = g_playlistPlayer.GetCurrentPlaylist();
  if (item.IsVideo() && playlist == PLAYLIST_VIDEO && g_playlistPlayer.GetPlaylist(playlist).size() > 1)
  { // playing from a playlist by the looks
    // don't switch to fullscreen if we are not playing the first item...
    options.fullscreen = !g_playlistPlayer.HasPlayedFirstFile() && g_advancedSettings.m_fullScreenOnMovieStart && !CMediaSettings::GetInstance().DoesVideoStartWindowed();
  }
  else if(m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
  {
    //! @todo - this will fail if user seeks back to first file in stack
    if(m_currentStackPosition == 0 || m_itemCurrentFile->m_lStartOffset == STARTOFFSET_RESUME)
      options.fullscreen = g_advancedSettings.m_fullScreenOnMovieStart && !CMediaSettings::GetInstance().DoesVideoStartWindowed();
    else
      options.fullscreen = false;
    // reset this so we don't think we are resuming on seek
    m_itemCurrentFile->m_lStartOffset = 0;
  }
  else
    options.fullscreen = g_advancedSettings.m_fullScreenOnMovieStart && !CMediaSettings::GetInstance().DoesVideoStartWindowed();

  // reset VideoStartWindowed as it's a temp setting
  CMediaSettings::GetInstance().SetVideoStartWindowed(false);

  {
    CSingleLock lock(m_playStateMutex);
    // tell system we are starting a file
    m_bPlaybackStarting = true;
    
    // for playing a new item, previous playing item's callback may already
    // pushed some delay message into the threadmessage list, they are not
    // expected be processed after or during the new item playback starting.
    // so we clean up previous playing item's playback callback delay messages here.
    int previousMsgsIgnoredByNewPlaying[] = {
      GUI_MSG_PLAYBACK_STARTED,
      GUI_MSG_PLAYBACK_ENDED,
      GUI_MSG_PLAYBACK_STOPPED,
      GUI_MSG_PLAYLIST_CHANGED,
      GUI_MSG_PLAYLISTPLAYER_STOPPED,
      GUI_MSG_PLAYLISTPLAYER_STARTED,
      GUI_MSG_PLAYLISTPLAYER_CHANGED,
      GUI_MSG_QUEUE_NEXT_ITEM,
      0
    };
    int dMsgCount = g_windowManager.RemoveThreadMessageByMessageIds(&previousMsgsIgnoredByNewPlaying[0]);
    if (dMsgCount > 0)
      CLog::LogF(LOGDEBUG,"Ignored %d playback thread messages", dMsgCount);
  }

  std::string newPlayer;
  if (!player.empty())
    newPlayer = player;
  else if (bRestart && !m_pPlayer->GetCurrentPlayer().empty())
    newPlayer = m_pPlayer->GetCurrentPlayer();
  else
    newPlayer = CPlayerCoreFactory::GetInstance().GetDefaultPlayer(item);

  // We should restart the player, unless the previous and next tracks are using
  // one of the players that allows gapless playback (paplayer, VideoPlayer)
  m_pPlayer->ClosePlayerGapless(newPlayer);

  // now reset play state to starting, since we already stopped the previous playing item if there is.
  // and from now there should be no playback callback from previous playing item be called.
  m_ePlayState = PLAY_STATE_STARTING;

  m_pPlayer->CreatePlayer(newPlayer, *this);

  PlayBackRet iResult;
  if (m_pPlayer->HasPlayer())
  {
    /* When playing video pause any low priority jobs, they will be unpaused  when playback stops.
     * This should speed up player startup for files on internet filesystems (eg. webdav) and
     * increase performance on low powered systems (Atom/ARM).
     */
    if (item.IsVideo())
    {
      CJobManager::GetInstance().PauseJobs();
    }

    // don't hold graphicscontext here since player
    // may wait on another thread, that requires gfx
    CSingleExit ex(g_graphicsContext);

    iResult = m_pPlayer->OpenFile(item, options);
  }
  else
  {
    CLog::Log(LOGERROR, "Error creating player for item %s (File doesn't exist?)", item.GetPath().c_str());
    iResult = PLAYBACK_FAIL;
  }

  if (iResult == PLAYBACK_OK)
  {
    m_pPlayer->SetVolume(m_volumeLevel);
    m_pPlayer->SetMute(m_muted);

    if(m_pPlayer->IsPlayingAudio())
    {
      if (g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
        g_windowManager.ActivateWindow(WINDOW_VISUALISATION);
    }

#ifdef HAS_VIDEO_PLAYBACK
    else if(m_pPlayer->IsPlayingVideo())
    {
      // if player didn't manage to switch to fullscreen by itself do it here
      if (options.fullscreen && m_pPlayer->IsRenderingVideo() &&
          g_windowManager.GetActiveWindow() != WINDOW_FULLSCREEN_VIDEO )
       SwitchToFullScreen(true);
    }
#endif
    else
    {
      if (g_windowManager.GetActiveWindow() == WINDOW_VISUALISATION ||
          g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
        g_windowManager.PreviousWindow();
    }

#if !defined(TARGET_POSIX)
    g_audioManager.Enable(false);
#endif

    if (item.HasPVRChannelInfoTag())
      g_playlistPlayer.SetCurrentPlaylist(PLAYLIST_NONE);
  }

  CSingleLock lock(m_playStateMutex);
  m_bPlaybackStarting = false;

  if (iResult == PLAYBACK_OK)
  {
    // play state: none, starting; playing; stopped; ended.
    // last 3 states are set by playback callback, they are all ignored during starting,
    // but we recorded the state, here we can make up the callback for the state.
    CLog::LogF(LOGDEBUG,"OpenFile succeed, play state %d", m_ePlayState);
    switch (m_ePlayState)
    {
      case PLAY_STATE_PLAYING:
        OnPlayBackStarted();
        break;
      // FIXME: it seems no meaning to callback started here if there was an started callback
      //        before this stopped/ended callback we recorded. if we callback started here
      //        first, it will delay send OnPlay announce, but then we callback stopped/ended
      //        which will send OnStop announce at once, so currently, just call stopped/ended.
      case PLAY_STATE_ENDED:
        OnPlayBackEnded();
        break;
      case PLAY_STATE_STOPPED:
        OnPlayBackStopped();
        break;
      case PLAY_STATE_STARTING:
        // neither started nor stopped/ended callback be called, that means the item still
        // not started, we need not make up any callback, just leave this and
        // let the player callback do its work.
        break;
      default:
        break;
    }
  }
  else if (iResult == PLAYBACK_FAIL)
  {
    // we send this if it isn't playlistplayer that is doing this
    int next = g_playlistPlayer.GetNextSong();
    int size = g_playlistPlayer.GetPlaylist(g_playlistPlayer.GetCurrentPlaylist()).size();
    if(next < 0
    || next >= size)
      OnPlayBackStopped();
    m_ePlayState = PLAY_STATE_NONE;
  }

  return iResult;
}

void CApplication::OnPlayBackEnded()
{
  CSingleLock lock(m_playStateMutex);
  CLog::LogF(LOGDEBUG,"play state was %d, starting %d", m_ePlayState, m_bPlaybackStarting);
  m_ePlayState = PLAY_STATE_ENDED;
  if(m_bPlaybackStarting)
    return;

  // informs python script currently running playback has ended
  // (does nothing if python is not loaded)
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackEnded();
#endif
#ifdef TARGET_ANDROID
  CXBMCApp::OnPlayBackEnded();
#elif defined(TARGET_DARWIN_IOS)
  CDarwinUtils::EnableOSScreenSaver(true);
#endif

  CVariant data(CVariant::VariantTypeObject);
  data["end"] = true;
  CAnnouncementManager::GetInstance().Announce(Player, "xbmc", "OnStop", m_itemCurrentFile, data);

  CGUIMessage msg(GUI_MSG_PLAYBACK_ENDED, 0, 0);
  g_windowManager.SendThreadMessage(msg);
}

void CApplication::OnPlayBackStarted()
{
  CSingleLock lock(m_playStateMutex);
  CLog::LogF(LOGDEBUG,"play state was %d, starting %d", m_ePlayState, m_bPlaybackStarting);
  m_ePlayState = PLAY_STATE_PLAYING;
  if(m_bPlaybackStarting)
    return;

#ifdef HAS_PYTHON
  // informs python script currently running playback has started
  // (does nothing if python is not loaded)
  g_pythonParser.OnPlayBackStarted();
#endif
#ifdef TARGET_ANDROID
  CXBMCApp::OnPlayBackStarted();
#elif defined(TARGET_DARWIN_IOS)
  if (m_pPlayer->IsPlayingVideo())
    CDarwinUtils::EnableOSScreenSaver(false);
#endif

  CGUIMessage msg(GUI_MSG_PLAYBACK_STARTED, 0, 0);
  g_windowManager.SendThreadMessage(msg);
}

void CApplication::OnQueueNextItem()
{
  CSingleLock lock(m_playStateMutex);
  CLog::LogF(LOGDEBUG,"play state was %d, starting %d", m_ePlayState, m_bPlaybackStarting);
  if(m_bPlaybackStarting)
    return;
  // informs python script currently running that we are requesting the next track
  // (does nothing if python is not loaded)
#ifdef HAS_PYTHON
  g_pythonParser.OnQueueNextItem(); // currently unimplemented
#endif

  CGUIMessage msg(GUI_MSG_QUEUE_NEXT_ITEM, 0, 0);
  g_windowManager.SendThreadMessage(msg);
}

void CApplication::OnPlayBackStopped()
{
  CSingleLock lock(m_playStateMutex);
  CLog::LogF(LOGDEBUG, "play state was %d, starting %d", m_ePlayState, m_bPlaybackStarting);
  m_ePlayState = PLAY_STATE_STOPPED;
  if(m_bPlaybackStarting)
    return;

  // informs python script currently running playback has ended
  // (does nothing if python is not loaded)
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackStopped();
#endif
#ifdef TARGET_ANDROID
  CXBMCApp::OnPlayBackStopped();
#elif defined(TARGET_DARWIN_IOS)
  CDarwinUtils::EnableOSScreenSaver(true);
#endif

  CVariant data(CVariant::VariantTypeObject);
  data["end"] = false;
  CAnnouncementManager::GetInstance().Announce(Player, "xbmc", "OnStop", m_itemCurrentFile, data);

  CGUIMessage msg( GUI_MSG_PLAYBACK_STOPPED, 0, 0 );
  g_windowManager.SendThreadMessage(msg);
}

void CApplication::OnPlayBackPaused()
{
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackPaused();
#endif
#ifdef TARGET_ANDROID
  CXBMCApp::OnPlayBackPaused();
#elif defined(TARGET_DARWIN_IOS)
  CDarwinUtils::EnableOSScreenSaver(true);
#endif

  CVariant param;
  param["player"]["speed"] = 0;
  param["player"]["playerid"] = g_playlistPlayer.GetCurrentPlaylist();
  CAnnouncementManager::GetInstance().Announce(Player, "xbmc", "OnPause", m_itemCurrentFile, param);
}

void CApplication::OnPlayBackResumed()
{
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackResumed();
#endif
#ifdef TARGET_ANDROID
  CXBMCApp::OnPlayBackResumed();
#elif defined(TARGET_DARWIN_IOS)
  if (m_pPlayer->IsPlayingVideo())
    CDarwinUtils::EnableOSScreenSaver(false);
#endif

  CVariant param;
  param["player"]["speed"] = 1;
  param["player"]["playerid"] = g_playlistPlayer.GetCurrentPlaylist();
  CAnnouncementManager::GetInstance().Announce(Player, "xbmc", "OnPlay", m_itemCurrentFile, param);
}

void CApplication::OnPlayBackSpeedChanged(int iSpeed)
{
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackSpeedChanged(iSpeed);
#endif

  CVariant param;
  param["player"]["speed"] = iSpeed;
  param["player"]["playerid"] = g_playlistPlayer.GetCurrentPlaylist();
  CAnnouncementManager::GetInstance().Announce(Player, "xbmc", "OnSpeedChanged", m_itemCurrentFile, param);
}

void CApplication::OnPlayBackSeek(int iTime, int seekOffset)
{
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackSeek(iTime, seekOffset);
#endif

  CVariant param;
  CJSONUtils::MillisecondsToTimeObject(iTime, param["player"]["time"]);
  CJSONUtils::MillisecondsToTimeObject(seekOffset, param["player"]["seekoffset"]);
  param["player"]["playerid"] = g_playlistPlayer.GetCurrentPlaylist();
  param["player"]["speed"] = (int)m_pPlayer->GetPlaySpeed();
  CAnnouncementManager::GetInstance().Announce(Player, "xbmc", "OnSeek", m_itemCurrentFile, param);
  g_infoManager.SetDisplayAfterSeek(2500, seekOffset);
}

void CApplication::OnPlayBackSeekChapter(int iChapter)
{
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackSeekChapter(iChapter);
#endif
}

bool CApplication::IsPlayingFullScreenVideo() const
{
  return m_pPlayer->IsPlayingVideo() && g_graphicsContext.IsFullScreenVideo();
}

bool CApplication::IsFullScreen()
{
  return IsPlayingFullScreenVideo() ||
        (g_windowManager.GetActiveWindow() == WINDOW_VISUALISATION) ||
         g_windowManager.GetActiveWindow() == WINDOW_SLIDESHOW;
}

void CApplication::SaveFileState(bool bForeground /* = false */)
{
  if (!CProfilesManager::GetInstance().GetCurrentProfile().canWriteDatabases())
    return;

  CJob* job = new CSaveFileStateJob(*m_progressTrackingItem,
      *m_stackFileItemToUpdate,
      m_progressTrackingVideoResumeBookmark,
      m_progressTrackingPlayCountUpdate,
      CMediaSettings::GetInstance().GetCurrentVideoSettings(),
      CMediaSettings::GetInstance().GetCurrentAudioSettings());
  
  if (bForeground)
  {
    // Run job in the foreground to make sure it finishes
    job->DoWork();
    delete job;
  }
  else
    CJobManager::GetInstance().AddJob(job, NULL, CJob::PRIORITY_NORMAL);
}

void CApplication::UpdateFileState()
{
  // Did the file change?
  if (m_progressTrackingItem->GetPath() != "" && m_progressTrackingItem->GetPath() != CurrentFile())
  {
    // Ignore for PVR channels, PerformChannelSwitch takes care of this.
    // Also ignore video playlists containing multiple items: video settings have already been saved in PlayFile()
    // and we'd overwrite them with settings for the *previous* item.
    //! @todo these "exceptions" should be removed and the whole logic of saving settings be revisited and
    //! possibly moved out of CApplication.  See PRs 5842, 5958, http://trac.kodi.tv/ticket/15704#comment:3
    int playlist = g_playlistPlayer.GetCurrentPlaylist();
    if (!m_progressTrackingItem->IsPVRChannel() && !(playlist == PLAYLIST_VIDEO && g_playlistPlayer.GetPlaylist(playlist).size() > 1))
      SaveFileState();

    // Reset tracking item
    m_progressTrackingItem->Reset();
  }
  else
  {
    if (m_pPlayer->IsPlaying())
    {
      if (m_progressTrackingItem->GetPath() == "")
      {
        // Init some stuff
        *m_progressTrackingItem = CurrentFileItem();
        m_progressTrackingPlayCountUpdate = false;
      }

      if ((m_progressTrackingItem->IsAudio() && g_advancedSettings.m_audioPlayCountMinimumPercent > 0 &&
          GetPercentage() >= g_advancedSettings.m_audioPlayCountMinimumPercent) ||
          (m_progressTrackingItem->IsVideo() && g_advancedSettings.m_videoPlayCountMinimumPercent > 0 &&
          GetPercentage() >= g_advancedSettings.m_videoPlayCountMinimumPercent))
      {
        m_progressTrackingPlayCountUpdate = true;
      }

      // Check whether we're *really* playing video else we may race when getting eg. stream details
      if (m_pPlayer->IsPlayingVideo())
      {
        /* Always update streamdetails, except for DVDs where we only update
           streamdetails if total duration > 15m (Should yield more correct info) */
        if (!(m_progressTrackingItem->IsDiscImage() || m_progressTrackingItem->IsDVDFile()) || m_pPlayer->GetTotalTime() > 15*60*1000)
        {
          CStreamDetails details;
          // Update with stream details from player, if any
          if (m_pPlayer->GetStreamDetails(details))
            m_progressTrackingItem->GetVideoInfoTag()->m_streamDetails = details;

          if (m_progressTrackingItem->IsStack())
            m_progressTrackingItem->GetVideoInfoTag()->m_streamDetails.SetVideoDuration(0, (int)GetTotalTime()); // Overwrite with CApp's totaltime as it takes into account total stack time
        }

        // Update bookmark for save
        m_progressTrackingVideoResumeBookmark.player = m_pPlayer->GetCurrentPlayer();
        m_progressTrackingVideoResumeBookmark.playerState = m_pPlayer->GetPlayerState();
        m_progressTrackingVideoResumeBookmark.thumbNailImage.clear();

        if (g_advancedSettings.m_videoIgnorePercentAtEnd > 0 &&
            GetTotalTime() - GetTime() < 0.01f * g_advancedSettings.m_videoIgnorePercentAtEnd * GetTotalTime())
        {
          // Delete the bookmark
          m_progressTrackingVideoResumeBookmark.timeInSeconds = -1.0f;
        }
        else
        if (GetTime() > g_advancedSettings.m_videoIgnoreSecondsAtStart)
        {
          // Update the bookmark
          m_progressTrackingVideoResumeBookmark.timeInSeconds = GetTime();
          m_progressTrackingVideoResumeBookmark.totalTimeInSeconds = GetTotalTime();
        }
        else
        {
          // Do nothing
          m_progressTrackingVideoResumeBookmark.timeInSeconds = 0.0f;
        }
      }
    }
  }
}

void CApplication::LoadVideoSettings(const CFileItem& item)
{
  CVideoDatabase dbs;
  if (dbs.Open())
  {
    CLog::Log(LOGDEBUG, "Loading settings for %s", CURL::GetRedacted(item.GetPath()).c_str());

    // Load stored settings if they exist, otherwise use default
    if (!dbs.GetVideoSettings(item, CMediaSettings::GetInstance().GetCurrentVideoSettings()))
      CMediaSettings::GetInstance().GetCurrentVideoSettings() = CMediaSettings::GetInstance().GetDefaultVideoSettings();

    dbs.Close();
  }
}

void CApplication::StopPlaying()
{
  int iWin = g_windowManager.GetActiveWindow();
  if ( m_pPlayer->IsPlaying() )
  {
    m_pPlayer->CloseFile();

    // turn off visualisation window when stopping
    if ((iWin == WINDOW_VISUALISATION
    ||  iWin == WINDOW_FULLSCREEN_VIDEO)
    && !m_bStop)
      g_windowManager.PreviousWindow();

    g_partyModeManager.Disable();
  }
}

void CApplication::ResetSystemIdleTimer()
{
  // reset system idle timer
  m_idleTimer.StartZero();
#if defined(TARGET_DARWIN_IOS)
  CDarwinUtils::ResetSystemIdleTimer();
#endif
}

void CApplication::ResetScreenSaver()
{
  // reset our timers
  m_shutdownTimer.StartZero();

  // screen saver timer is reset only if we're not already in screensaver or
  // DPMS mode
  if ((!m_bScreenSave && m_iScreenSaveLock == 0) && !m_dpmsIsActive)
    ResetScreenSaverTimer();
}

void CApplication::ResetScreenSaverTimer()
{
  m_screenSaverTimer.StartZero();
}

void CApplication::StopScreenSaverTimer()
{
  m_screenSaverTimer.Stop();
}

bool CApplication::ToggleDPMS(bool manual)
{
  if (manual || (m_dpmsIsManual == manual))
  {
    if (m_dpmsIsActive)
    {
      m_dpmsIsActive = false;
      m_dpmsIsManual = false;
      SetRenderGUI(true);
      CAnnouncementManager::GetInstance().Announce(GUI, "xbmc", "OnDPMSDeactivated");
      return m_dpms->DisablePowerSaving();
    }
    else
    {
      if (m_dpms->EnablePowerSaving(m_dpms->GetSupportedModes()[0]))
      {
        m_dpmsIsActive = true;
        m_dpmsIsManual = manual;
        SetRenderGUI(false);
        CAnnouncementManager::GetInstance().Announce(GUI, "xbmc", "OnDPMSActivated");
        return true;
      }
    }
  }
  return false;
}

bool CApplication::WakeUpScreenSaverAndDPMS(bool bPowerOffKeyPressed /* = false */)
{
  bool result;

  // First reset DPMS, if active
  if (m_dpmsIsActive)
  {
    if (m_dpmsIsManual)
      return false;
    //! @todo if screensaver lock is specified but screensaver is not active
    //! (DPMS came first), activate screensaver now.
    ToggleDPMS(false);
    ResetScreenSaverTimer();
    result = !m_bScreenSave || WakeUpScreenSaver(bPowerOffKeyPressed);
  }
  else
    result = WakeUpScreenSaver(bPowerOffKeyPressed);

  if(result)
  {
    // allow listeners to ignore the deactivation if it preceeds a powerdown/suspend etc
    CVariant data(CVariant::VariantTypeObject);
    data["shuttingdown"] = bPowerOffKeyPressed;
    CAnnouncementManager::GetInstance().Announce(GUI, "xbmc", "OnScreensaverDeactivated", data);
#ifdef TARGET_ANDROID
    // Screensaver deactivated -> acquire wake lock
    CXBMCApp::EnableWakeLock(true);
#endif
  }

  return result;
}

bool CApplication::WakeUpScreenSaver(bool bPowerOffKeyPressed /* = false */)
{
  if (m_iScreenSaveLock == 2)
    return false;

  // if Screen saver is active
  if (m_bScreenSave && m_screenSaver)
  {
    if (m_iScreenSaveLock == 0)
      if (CProfilesManager::GetInstance().GetMasterProfile().getLockMode() != LOCK_MODE_EVERYONE &&
          (CProfilesManager::GetInstance().UsingLoginScreen() || CSettings::GetInstance().GetBool(CSettings::SETTING_MASTERLOCK_STARTUPLOCK)) &&
          CProfilesManager::GetInstance().GetCurrentProfile().getLockMode() != LOCK_MODE_EVERYONE &&
          m_screenSaver->ID() != "screensaver.xbmc.builtin.dim" && m_screenSaver->ID() != "screensaver.xbmc.builtin.black" && !m_screenSaver->ID().empty() && m_screenSaver->ID() != "visualization")
      {
        m_iScreenSaveLock = 2;
        CGUIMessage msg(GUI_MSG_CHECK_LOCK,0,0);

        CGUIWindow* pWindow = g_windowManager.GetWindow(WINDOW_SCREENSAVER);
        if (pWindow)
          pWindow->OnMessage(msg);
      }
    if (m_iScreenSaveLock == -1)
    {
      m_iScreenSaveLock = 0;
      return true;
    }

    // disable screensaver
    m_bScreenSave = false;
    m_iScreenSaveLock = 0;
    ResetScreenSaverTimer();

    if (m_screenSaver->ID() == "visualization")
    {
      // we can just continue as usual from vis mode
      return false;
    }
    else if (m_screenSaver->ID() == "screensaver.xbmc.builtin.dim" || m_screenSaver->ID() == "screensaver.xbmc.builtin.black" || m_screenSaver->ID().empty())
      return true;
    else if (!m_screenSaver->ID().empty())
    { // we're in screensaver window
      if (g_windowManager.GetActiveWindow() == WINDOW_SCREENSAVER)
        g_windowManager.PreviousWindow();  // show the previous window
      if (g_windowManager.GetActiveWindow() == WINDOW_SLIDESHOW)
        CApplicationMessenger::GetInstance().SendMsg(TMSG_GUI_ACTION, WINDOW_SLIDESHOW, -1, static_cast<void*>(new CAction(ACTION_STOP)));
    }
    return true;
  }
  else
    return false;
}

void CApplication::CheckScreenSaverAndDPMS()
{
  if (!m_dpmsIsActive)
    g_Windowing.ResetOSScreensaver();

  bool maybeScreensaver =
      !m_dpmsIsActive && !m_bScreenSave
      && !CSettings::GetInstance().GetString(CSettings::SETTING_SCREENSAVER_MODE).empty();
  bool maybeDPMS =
      !m_dpmsIsActive && m_dpms->IsSupported()
      && CSettings::GetInstance().GetInt(CSettings::SETTING_POWERMANAGEMENT_DISPLAYSOFF) > 0;

  // Has the screen saver window become active?
  if (maybeScreensaver && g_windowManager.IsWindowActive(WINDOW_SCREENSAVER))
  {
    m_bScreenSave = true;
    maybeScreensaver = false;
  }

  if (m_bScreenSave && m_pPlayer->IsPlayingVideo() && !m_pPlayer->IsPaused())
  {
    WakeUpScreenSaverAndDPMS();
    return;
  }

  if (!maybeScreensaver && !maybeDPMS) return;  // Nothing to do.

  // See if we need to reset timer.
  // * Are we playing a video and it is not paused?
  if ((m_pPlayer->IsPlayingVideo() && !m_pPlayer->IsPaused())
      // * Are we playing some music in fullscreen vis?
      || (m_pPlayer->IsPlayingAudio() && g_windowManager.GetActiveWindow() == WINDOW_VISUALISATION
          && !CSettings::GetInstance().GetString(CSettings::SETTING_MUSICPLAYER_VISUALISATION).empty()))
  {
    ResetScreenSaverTimer();
    return;
  }

  float elapsed = m_screenSaverTimer.IsRunning() ? m_screenSaverTimer.GetElapsedSeconds() : 0.f;

  // DPMS has priority (it makes the screensaver not needed)
  if (maybeDPMS
      && elapsed > CSettings::GetInstance().GetInt(CSettings::SETTING_POWERMANAGEMENT_DISPLAYSOFF) * 60)
  {
    ToggleDPMS(false);
    WakeUpScreenSaver();
  }
  else if (maybeScreensaver
           && elapsed > CSettings::GetInstance().GetInt(CSettings::SETTING_SCREENSAVER_TIME) * 60)
  {
    ActivateScreenSaver();
  }
}

// activate the screensaver.
// if forceType is true, we ignore the various conditions that can alter
// the type of screensaver displayed
void CApplication::ActivateScreenSaver(bool forceType /*= false */)
{
  if (m_pPlayer->IsPlayingAudio() && CSettings::GetInstance().GetBool(CSettings::SETTING_SCREENSAVER_USEMUSICVISINSTEAD) && !CSettings::GetInstance().GetString(CSettings::SETTING_MUSICPLAYER_VISUALISATION).empty())
  { // just activate the visualisation if user toggled the usemusicvisinstead option
    g_windowManager.ActivateWindow(WINDOW_VISUALISATION);
    return;
  }

  m_bScreenSave = true;

  // Get Screensaver Mode
  m_screenSaver.reset();
  if (!CAddonMgr::GetInstance().GetAddon(CSettings::GetInstance().GetString(CSettings::SETTING_SCREENSAVER_MODE), m_screenSaver))
    m_screenSaver.reset(new CScreenSaver(""));

  CAnnouncementManager::GetInstance().Announce(GUI, "xbmc", "OnScreensaverActivated");

  // disable screensaver lock from the login screen
  m_iScreenSaveLock = g_windowManager.GetActiveWindow() == WINDOW_LOGIN_SCREEN ? 1 : 0;
  if (!forceType)
  {
    // set to Dim in the case of a dialog on screen or playing video
    if (g_windowManager.HasModalDialog() || (m_pPlayer->IsPlayingVideo() && CSettings::GetInstance().GetBool(CSettings::SETTING_SCREENSAVER_USEDIMONPAUSE)) || g_PVRManager.IsRunningChannelScan())
    {
      if (!CAddonMgr::GetInstance().GetAddon("screensaver.xbmc.builtin.dim", m_screenSaver))
        m_screenSaver.reset(new CScreenSaver(""));
    }
  }
  if (m_screenSaver->ID() == "screensaver.xbmc.builtin.dim"
      || m_screenSaver->ID() == "screensaver.xbmc.builtin.black")
  {
#ifdef TARGET_ANDROID
    // Default screensaver activated -> release wake lock
    CXBMCApp::EnableWakeLock(false);
#endif
    return;
  }
  else if (m_screenSaver->ID().empty())
    return;
  else
    g_windowManager.ActivateWindow(WINDOW_SCREENSAVER);
}

void CApplication::CheckShutdown()
{
  // first check if we should reset the timer
  if (m_bInhibitIdleShutdown
      || m_pPlayer->IsPlaying() || m_pPlayer->IsPausedPlayback() // is something playing?
      || m_musicInfoScanner->IsScanning()
      || CVideoLibraryQueue::GetInstance().IsRunning()
      || g_windowManager.IsWindowActive(WINDOW_DIALOG_PROGRESS) // progress dialog is onscreen
      || !g_PVRManager.CanSystemPowerdown(false))
  {
    m_shutdownTimer.StartZero();
    return;
  }

  float elapsed = m_shutdownTimer.IsRunning() ? m_shutdownTimer.GetElapsedSeconds() : 0.f;
  if ( elapsed > CSettings::GetInstance().GetInt(CSettings::SETTING_POWERMANAGEMENT_SHUTDOWNTIME) * 60 )
  {
    // Since it is a sleep instead of a shutdown, let's set everything to reset when we wake up.
    m_shutdownTimer.Stop();

    // Sleep the box
    CApplicationMessenger::GetInstance().PostMsg(TMSG_SHUTDOWN);
  }
}

void CApplication::InhibitIdleShutdown(bool inhibit)
{
  m_bInhibitIdleShutdown = inhibit;
}

bool CApplication::IsIdleShutdownInhibited() const
{
  return m_bInhibitIdleShutdown;
}

bool CApplication::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_NOTIFY_ALL:
    {
      if (message.GetParam1()==GUI_MSG_REMOVED_MEDIA)
      {
        // Update general playlist: Remove DVD playlist items
        int nRemoved = g_playlistPlayer.RemoveDVDItems();
        if ( nRemoved > 0 )
        {
          CGUIMessage msg( GUI_MSG_PLAYLIST_CHANGED, 0, 0 );
          g_windowManager.SendMessage( msg );
        }
        // stop the file if it's on dvd (will set the resume point etc)
        if (m_itemCurrentFile->IsOnDVD())
          StopPlaying();
      }
      else if (message.GetParam1() == GUI_MSG_UI_READY)
      {
        // remove splash window
        g_windowManager.Delete(WINDOW_SPLASH);

        // show the volumebar if the volume is muted
        if (IsMuted() || GetVolume(false) <= VOLUME_MINIMUM)
          ShowVolumeBar();

        if (m_fallbackLanguageLoaded)
          CGUIDialogOK::ShowAndGetInput(CVariant{24133}, CVariant{24134});

        if (!m_incompatibleAddons.empty())
        {
          auto addonList = StringUtils::Join(m_incompatibleAddons, ", ");
          auto msg = StringUtils::Format(g_localizeStrings.Get(24149).c_str(), addonList.c_str());
          CGUIDialogOK::ShowAndGetInput(CVariant{24148}, CVariant{std::move(msg)});
          m_incompatibleAddons.clear();
        }

        // show info dialog about moved configuration files if needed
        ShowAppMigrationMessage();

        m_bInitializing = false;
      }
    }
    break;

  case GUI_MSG_PLAYBACK_STARTED:
    {
#ifdef TARGET_DARWIN_IOS
      CDarwinUtils::SetScheduling(message.GetMessage());
#endif
      CPlayList playList = g_playlistPlayer.GetPlaylist(g_playlistPlayer.GetCurrentPlaylist());

      // Update our infoManager with the new details etc.
      if (m_nextPlaylistItem >= 0)
      { 
        // playing an item which is not in the list - player might be stopped already
        // so do nothing
        if (playList.size() <= m_nextPlaylistItem)
          return true;

        // we've started a previously queued item
        CFileItemPtr item = playList[m_nextPlaylistItem];
        // update the playlist manager
        int currentSong = g_playlistPlayer.GetCurrentSong();
        int param = ((currentSong & 0xffff) << 16) | (m_nextPlaylistItem & 0xffff);
        CGUIMessage msg(GUI_MSG_PLAYLISTPLAYER_CHANGED, 0, 0, g_playlistPlayer.GetCurrentPlaylist(), param, item);
        g_windowManager.SendThreadMessage(msg);
        g_playlistPlayer.SetCurrentSong(m_nextPlaylistItem);
        m_itemCurrentFile.reset(new CFileItem(*item));
      }
      g_infoManager.SetCurrentItem(m_itemCurrentFile);
      g_partyModeManager.OnSongChange(true);

      CVariant param;
      param["player"]["speed"] = 1;
      param["player"]["playerid"] = g_playlistPlayer.GetCurrentPlaylist();
      CAnnouncementManager::GetInstance().Announce(Player, "xbmc", "OnPlay", m_itemCurrentFile, param);
      return true;
    }
    break;

  case GUI_MSG_QUEUE_NEXT_ITEM:
    {
      // Check to see if our playlist player has a new item for us,
      // and if so, we check whether our current player wants the file
      int iNext = g_playlistPlayer.GetNextSong();
      CPlayList& playlist = g_playlistPlayer.GetPlaylist(g_playlistPlayer.GetCurrentPlaylist());
      if (iNext < 0 || iNext >= playlist.size())
      {
        m_pPlayer->OnNothingToQueueNotify();
        return true; // nothing to do
      }

      // ok, grab the next song
      CFileItem file(*playlist[iNext]);
      // handle plugin://
      CURL url(file.GetPath());
      if (url.IsProtocol("plugin"))
        XFILE::CPluginDirectory::GetPluginResult(url.Get(), file);

      // Don't queue if next media type is different from current one
      if ((!file.IsVideo() && m_pPlayer->IsPlayingVideo())
          || ((!file.IsAudio() || file.IsVideo()) && m_pPlayer->IsPlayingAudio()))
      {
        m_pPlayer->OnNothingToQueueNotify();
        return true;
      }

#ifdef HAS_UPNP
      if (URIUtils::IsUPnP(file.GetPath()))
      {
        if (!XFILE::CUPnPDirectory::GetResource(file.GetURL(), file))
          return true;
      }
#endif

      // ok - send the file to the player, if it accepts it
      if (m_pPlayer->QueueNextFile(file))
      {
        // player accepted the next file
        m_nextPlaylistItem = iNext;
      }
      else
      {
        /* Player didn't accept next file: *ALWAYS* advance playlist in this case so the player can
            queue the next (if it wants to) and it doesn't keep looping on this song */
        g_playlistPlayer.SetCurrentSong(iNext);
      }

      return true;
    }
    break;

  case GUI_MSG_PLAYBACK_STOPPED:
  case GUI_MSG_PLAYBACK_ENDED:
  case GUI_MSG_PLAYLISTPLAYER_STOPPED:
    {
#ifdef TARGET_DARWIN_IOS
      CDarwinUtils::SetScheduling(message.GetMessage());
#endif
      // first check if we still have items in the stack to play
      if (message.GetMessage() == GUI_MSG_PLAYBACK_ENDED)
      {
        if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0 && m_currentStackPosition < m_currentStack->Size() - 1)
        { // just play the next item in the stack
          PlayFile(*(*m_currentStack)[++m_currentStackPosition], "", true);
          return true;
        }
      }

      // In case playback ended due to user eg. skipping over the end, clear
      // our resume bookmark here
      if (message.GetMessage() == GUI_MSG_PLAYBACK_ENDED && m_progressTrackingPlayCountUpdate && g_advancedSettings.m_videoIgnorePercentAtEnd > 0)
      {
        // Delete the bookmark
        m_progressTrackingVideoResumeBookmark.timeInSeconds = -1.0f;
      }

      // reset the current playing file
      m_itemCurrentFile->Reset();
      g_infoManager.ResetCurrentItem();
      m_currentStack->Clear();

      if (message.GetMessage() == GUI_MSG_PLAYBACK_ENDED)
      {
        g_playlistPlayer.PlayNext(1, true);
      }
      else
      {
        m_pPlayer->ClosePlayer();
      }

      if (!m_pPlayer->IsPlaying())
      {
        g_audioManager.Enable(true);
      }

      if (!m_pPlayer->IsPlayingVideo())
      {
        if(g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
        {
          g_windowManager.PreviousWindow();
        }
        else
        {
          CSingleLock lock(g_graphicsContext);
          //  resets to res_desktop or look&feel resolution (including refreshrate)
          g_graphicsContext.SetFullScreenVideo(false);
        }
      }

      if (!m_pPlayer->IsPlayingAudio() && g_playlistPlayer.GetCurrentPlaylist() == PLAYLIST_NONE && g_windowManager.GetActiveWindow() == WINDOW_VISUALISATION)
      {
        CSettings::GetInstance().Save();  // save vis settings
        WakeUpScreenSaverAndDPMS();
        g_windowManager.PreviousWindow();
      }

      // DVD ejected while playing in vis ?
      if (!m_pPlayer->IsPlayingAudio() && (m_itemCurrentFile->IsCDDA() || m_itemCurrentFile->IsOnDVD()) && !g_mediaManager.IsDiscInDrive() && g_windowManager.GetActiveWindow() == WINDOW_VISUALISATION)
      {
        // yes, disable vis
        CSettings::GetInstance().Save();    // save vis settings
        WakeUpScreenSaverAndDPMS();
        g_windowManager.PreviousWindow();
      }

      if (IsEnableTestMode())
        CApplicationMessenger::GetInstance().PostMsg(TMSG_QUIT);
      return true;
    }
    break;

  case GUI_MSG_PLAYLISTPLAYER_STARTED:
  case GUI_MSG_PLAYLISTPLAYER_CHANGED:
    {
      return true;
    }
    break;
  case GUI_MSG_FULLSCREEN:
    { // Switch to fullscreen, if we can
      SwitchToFullScreen();
      return true;
    }
    break;
  case GUI_MSG_EXECUTE:
    if (message.GetNumStringParams())
      return ExecuteXBMCAction(message.GetStringParam(), message.GetItem());
    break;
  }
  return false;
}

bool CApplication::ExecuteXBMCAction(std::string actionStr, const CGUIListItemPtr &item /* = NULL */)
{
  // see if it is a user set string

  //We don't know if there is unsecure information in this yet, so we
  //postpone any logging
  const std::string in_actionStr(actionStr);
  if (item)
    actionStr = CGUIInfoLabel::GetItemLabel(actionStr, item.get());
  else
    actionStr = CGUIInfoLabel::GetLabel(actionStr);

  // user has asked for something to be executed
  if (CBuiltins::GetInstance().HasCommand(actionStr))
  {
    if (!CBuiltins::GetInstance().IsSystemPowerdownCommand(actionStr) ||
        g_PVRManager.CanSystemPowerdown())
      CBuiltins::GetInstance().Execute(actionStr);
  }
  else
  {
    // try translating the action from our ButtonTranslator
    int actionID;
    if (CButtonTranslator::TranslateActionString(actionStr.c_str(), actionID))
    {
      OnAction(CAction(actionID));
      return true;
    }
    CFileItem item(actionStr, false);
#ifdef HAS_PYTHON
    if (item.IsPythonScript())
    { // a python script
      CScriptInvocationManager::GetInstance().ExecuteAsync(item.GetPath());
    }
    else
#endif
    if (item.IsAudio() || item.IsVideo())
    { // an audio or video file
      PlayFile(item, "");
    }
    else
    {
      //At this point we have given up to translate, so even though
      //there may be insecure information, we log it.
      CLog::LogF(LOGDEBUG,"Tried translating, but failed to understand %s", in_actionStr.c_str());
      return false;
    }
  }
  return true;
}

// inform the user that the configuration data has moved from old XBMC location
// to new Kodi location - if applicable
void CApplication::ShowAppMigrationMessage()
{
  // .kodi_migration_complete will be created from the installer/packaging
  // once an old XBMC configuration was moved to the new Kodi location
  // if this is the case show the migration info to the user once which
  // tells him to have a look into the wiki where the move of configuration
  // is further explained.
  if (CFile::Exists("special://home/.kodi_data_was_migrated") &&
      !CFile::Exists("special://home/.kodi_migration_info_shown"))
  {
    CGUIDialogOK::ShowAndGetInput(CVariant{24128}, CVariant{24129});
    CFile tmpFile;
    // create the file which will prevent this dialog from appearing in the future
    tmpFile.OpenForWrite("special://home/.kodi_migration_info_shown");
    tmpFile.Close();
  }
}

void CApplication::Process()
{
  MEASURE_FUNCTION;

  // dispatch the messages generated by python or other threads to the current window
  g_windowManager.DispatchThreadMessages();

  // process messages which have to be send to the gui
  // (this can only be done after g_windowManager.Render())
  CApplicationMessenger::GetInstance().ProcessWindowMessages();

  if (m_autoExecScriptExecuted)
  {
    m_autoExecScriptExecuted = false;

    // autoexec.py - profile
    std::string strAutoExecPy = CSpecialProtocol::TranslatePath("special://profile/autoexec.py");

    if (XFILE::CFile::Exists(strAutoExecPy))
      CScriptInvocationManager::GetInstance().ExecuteAsync(strAutoExecPy);
    else
      CLog::Log(LOGDEBUG, "no profile autoexec.py (%s) found, skipping", strAutoExecPy.c_str());
  }

  // handle any active scripts

  {
    // Allow processing of script threads to let them shut down properly.
    CSingleExit ex(g_graphicsContext);
    m_frameMoveGuard.unlock();
    CScriptInvocationManager::GetInstance().Process();
    m_frameMoveGuard.lock();
  }

  // process messages, even if a movie is playing
  CApplicationMessenger::GetInstance().ProcessMessages();
  if (g_application.m_bStop) return; //we're done, everything has been unloaded

  // update sound
  m_pPlayer->DoAudioWork();

  // do any processing that isn't needed on each run
  if( m_slowTimer.GetElapsedMilliseconds() > 500 )
  {
    m_slowTimer.Reset();
    ProcessSlow();
  }
#if !defined(TARGET_DARWIN)
  g_cpuInfo.getUsedPercentage(); // must call it to recalculate pct values
#endif
}

// We get called every 500ms
void CApplication::ProcessSlow()
{
  g_powerManager.ProcessEvents();

#if defined(TARGET_DARWIN_OSX)
  // There is an issue on OS X that several system services ask the cursor to become visible
  // during their startup routines.  Given that we can't control this, we hack it in by
  // forcing the
  if (g_Windowing.IsFullScreen())
  { // SDL thinks it's hidden
    Cocoa_HideMouse();
  }
#endif

  // Temporarely pause pausable jobs when viewing video/picture
  int currentWindow = g_windowManager.GetActiveWindow();
  if (CurrentFileItem().IsVideo() || CurrentFileItem().IsPicture() || currentWindow == WINDOW_FULLSCREEN_VIDEO || currentWindow == WINDOW_SLIDESHOW)
  {
    CJobManager::GetInstance().PauseJobs();
  }
  else
  {
    CJobManager::GetInstance().UnPauseJobs();
  }

  // Store our file state for use on close()
  UpdateFileState();

  // Check if we need to activate the screensaver / DPMS.
  CheckScreenSaverAndDPMS();

  // Check if we need to shutdown (if enabled).
#if defined(TARGET_DARWIN)
  if (CSettings::GetInstance().GetInt(CSettings::SETTING_POWERMANAGEMENT_SHUTDOWNTIME) && g_advancedSettings.m_fullScreen)
#else
  if (CSettings::GetInstance().GetInt(CSettings::SETTING_POWERMANAGEMENT_SHUTDOWNTIME))
#endif
  {
    CheckShutdown();
  }

  // check if we should restart the player
  CheckDelayedPlayerRestart();

  //  check if we can unload any unreferenced dlls or sections
  if (!m_pPlayer->IsPlayingVideo())
    CSectionLoader::UnloadDelayed();

  // check for any idle curl connections
  g_curlInterface.CheckIdle();

  g_largeTextureManager.CleanupUnusedImages();

  g_TextureManager.FreeUnusedTextures(5000);

#ifdef HAS_DVD_DRIVE
  // checks whats in the DVD drive and tries to autostart the content (xbox games, dvd, cdda, avi files...)
  if (!m_pPlayer->IsPlayingVideo())
    m_Autorun->HandleAutorun();
#endif

  // update upnp server/renderer states
#ifdef HAS_UPNP
  if(UPNP::CUPnP::IsInstantiated())
    UPNP::CUPnP::GetInstance()->UpdateState();
#endif

#if defined(TARGET_POSIX) && defined(HAS_FILESYSTEM_SMB)
  smb.CheckIfIdle();
#endif

#ifdef HAS_FILESYSTEM_NFS
  gNfsConnection.CheckIfIdle();
#endif

#ifdef HAS_FILESYSTEM_SFTP
  CSFTPSessionManager::ClearOutIdleSessions();
#endif

  g_mediaManager.ProcessEvents();

  CAEFactory::GarbageCollect();

  // if we don't render the gui there's no reason to start the screensaver.
  // that way the screensaver won't kick in if we maximize the XBMC window
  // after the screensaver start time.
  if(!m_renderGUI)
    ResetScreenSaverTimer();
}

// Global Idle Time in Seconds
// idle time will be resetet if on any OnKey()
// int return: system Idle time in seconds! 0 is no idle!
int CApplication::GlobalIdleTime()
{
  if(!m_idleTimer.IsRunning())
    m_idleTimer.StartZero();
  return (int)m_idleTimer.GetElapsedSeconds();
}

float CApplication::NavigationIdleTime()
{
  if (!m_navigationTimer.IsRunning())
    m_navigationTimer.StartZero();
  return m_navigationTimer.GetElapsedSeconds();
}

void CApplication::DelayedPlayerRestart()
{
  m_restartPlayerTimer.StartZero();
}

void CApplication::CheckDelayedPlayerRestart()
{
  if (m_restartPlayerTimer.GetElapsedSeconds() > 3)
  {
    m_restartPlayerTimer.Stop();
    m_restartPlayerTimer.Reset();
    Restart(true);
  }
}

void CApplication::Restart(bool bSamePosition)
{
  // this function gets called when the user changes a setting (like noninterleaved)
  // and which means we gotta close & reopen the current playing file

  // first check if we're playing a file
  if ( !m_pPlayer->IsPlayingVideo() && !m_pPlayer->IsPlayingAudio())
    return ;

  if( !m_pPlayer->HasPlayer() )
    return ;

  SaveFileState();

  // do we want to return to the current position in the file
  if (false == bSamePosition)
  {
    // no, then just reopen the file and start at the beginning
    PlayFile(*m_itemCurrentFile, "", true);
    return ;
  }

  // else get current position
  double time = GetTime();

  // get player state, needed for dvd's
  std::string state = m_pPlayer->GetPlayerState();

  // set the requested starttime
  m_itemCurrentFile->m_lStartOffset = (long)(time * 75.0);

  // reopen the file
  if ( PlayFile(*m_itemCurrentFile, "", true) == PLAYBACK_OK )
    m_pPlayer->SetPlayerState(state);
}

const std::string& CApplication::CurrentFile()
{
  return m_itemCurrentFile->GetPath();
}

std::shared_ptr<CFileItem> CApplication::CurrentFileItemPtr()
{
  return m_itemCurrentFile;
}

CFileItem& CApplication::CurrentFileItem()
{
  return *m_itemCurrentFile;
}

void CApplication::SetCurrentFileItem(const CFileItem& item)
{
  m_itemCurrentFile.reset(new CFileItem(item));
}

CFileItem& CApplication::CurrentUnstackedItem()
{
  if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
    return *(*m_currentStack)[m_currentStackPosition];
  else
    return *m_itemCurrentFile;
}

void CApplication::ShowVolumeBar(const CAction *action)
{
  CGUIDialog *volumeBar = (CGUIDialog *)g_windowManager.GetWindow(WINDOW_DIALOG_VOLUME_BAR);
  if (volumeBar)
  {
    volumeBar->Open();
    if (action)
      volumeBar->OnAction(*action);
  }
}

bool CApplication::IsMuted() const
{
  if (g_peripherals.IsMuted())
    return true;
  return CAEFactory::IsMuted();
}

void CApplication::ToggleMute(void)
{
  if (m_muted)
    UnMute();
  else
    Mute();
}

void CApplication::SetMute(bool mute)
{
  if (m_muted != mute)
  {
    ToggleMute();
    m_muted = mute;
  }
}

void CApplication::Mute()
{
  if (g_peripherals.Mute())
    return;

  CAEFactory::SetMute(true);
  m_muted = true;
  VolumeChanged();
}

void CApplication::UnMute()
{
  if (g_peripherals.UnMute())
    return;

  CAEFactory::SetMute(false);
  m_muted = false;
  VolumeChanged();
}

void CApplication::SetVolume(float iValue, bool isPercentage/*=true*/)
{
  float hardwareVolume = iValue;

  if(isPercentage)
    hardwareVolume /= 100.0f;

  SetHardwareVolume(hardwareVolume);
  VolumeChanged();
}

void CApplication::SetHardwareVolume(float hardwareVolume)
{
  hardwareVolume = std::max(VOLUME_MINIMUM, std::min(VOLUME_MAXIMUM, hardwareVolume));
  m_volumeLevel = hardwareVolume;

  CAEFactory::SetVolume(hardwareVolume);
}

float CApplication::GetVolume(bool percentage /* = true */) const
{
  if (percentage)
  {
    // converts the hardware volume to a percentage
    return m_volumeLevel * 100.0f;
  }
  
  return m_volumeLevel;
}

void CApplication::VolumeChanged() const
{
  CVariant data(CVariant::VariantTypeObject);
  data["volume"] = GetVolume();
  data["muted"] = m_muted;
  CAnnouncementManager::GetInstance().Announce(Application, "xbmc", "OnVolumeChanged", data);

  // if player has volume control, set it.
  m_pPlayer->SetVolume(m_volumeLevel);
  m_pPlayer->SetMute(m_muted);
}

int CApplication::GetSubtitleDelay() const
{
  // converts subtitle delay to a percentage
  return int(((float)(CMediaSettings::GetInstance().GetCurrentVideoSettings().m_SubtitleDelay + g_advancedSettings.m_videoSubsDelayRange)) / (2 * g_advancedSettings.m_videoSubsDelayRange)*100.0f + 0.5f);
}

int CApplication::GetAudioDelay() const
{
  // converts audio delay to a percentage
  return int(((float)(CMediaSettings::GetInstance().GetCurrentVideoSettings().m_AudioDelay + g_advancedSettings.m_videoAudioDelayRange)) / (2 * g_advancedSettings.m_videoAudioDelayRange)*100.0f + 0.5f);
}

// Returns the total time in seconds of the current media.  Fractional
// portions of a second are possible - but not necessarily supported by the
// player class.  This returns a double to be consistent with GetTime() and
// SeekTime().
double CApplication::GetTotalTime() const
{
  double rc = 0.0;

  if (m_pPlayer->IsPlaying())
  {
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
      rc = (*m_currentStack)[m_currentStack->Size() - 1]->m_lEndOffset;
    else
      rc = static_cast<double>(m_pPlayer->GetTotalTime() * 0.001f);
  }

  return rc;
}

void CApplication::StopShutdownTimer()
{
  m_shutdownTimer.Stop();
}

void CApplication::ResetShutdownTimers()
{
  // reset system shutdown timer
  m_shutdownTimer.StartZero();

  // delete custom shutdown timer
  if (g_alarmClock.HasAlarm("shutdowntimer"))
    g_alarmClock.Stop("shutdowntimer", true);
}

// Returns the current time in seconds of the currently playing media.
// Fractional portions of a second are possible.  This returns a double to
// be consistent with GetTotalTime() and SeekTime().
double CApplication::GetTime() const
{
  double rc = 0.0;

  if (m_pPlayer->IsPlaying())
  {
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
    {
      long startOfCurrentFile = (m_currentStackPosition > 0) ? (*m_currentStack)[m_currentStackPosition-1]->m_lEndOffset : 0;
      rc = (double)startOfCurrentFile + m_pPlayer->GetTime() * 0.001;
    }
    else
      rc = static_cast<double>(m_pPlayer->GetTime() * 0.001f);
  }

  return rc;
}

// Sets the current position of the currently playing media to the specified
// time in seconds.  Fractional portions of a second are valid.  The passed
// time is the time offset from the beginning of the file as opposed to a
// delta from the current position.  This method accepts a double to be
// consistent with GetTime() and GetTotalTime().
void CApplication::SeekTime( double dTime )
{
  if (m_pPlayer->IsPlaying() && (dTime >= 0.0))
  {
    if (!m_pPlayer->CanSeek()) return;
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
    {
      // find the item in the stack we are seeking to, and load the new
      // file if necessary, and calculate the correct seek within the new
      // file.  Otherwise, just fall through to the usual routine if the
      // time is higher than our total time.
      for (int i = 0; i < m_currentStack->Size(); i++)
      {
        if ((*m_currentStack)[i]->m_lEndOffset > dTime)
        {
          long startOfNewFile = (i > 0) ? (*m_currentStack)[i-1]->m_lEndOffset : 0;
          if (m_currentStackPosition == i)
            m_pPlayer->SeekTime((int64_t)((dTime - startOfNewFile) * 1000.0));
          else
          { // seeking to a new file
            m_currentStackPosition = i;
            CFileItem *item = new CFileItem(*(*m_currentStack)[i]);
            item->m_lStartOffset = static_cast<long>((dTime - startOfNewFile) * 75.0);
            // don't just call "PlayFile" here, as we are quite likely called from the
            // player thread, so we won't be able to delete ourselves.
            CApplicationMessenger::GetInstance().PostMsg(TMSG_MEDIA_PLAY, 1, 0, static_cast<void*>(item));
          }
          return;
        }
      }
    }
    // convert to milliseconds and perform seek
    m_pPlayer->SeekTime( static_cast<int64_t>( dTime * 1000.0 ) );
  }
}

float CApplication::GetPercentage() const
{
  if (m_pPlayer->IsPlaying())
  {
    if (m_pPlayer->GetTotalTime() == 0 && m_pPlayer->IsPlayingAudio() && m_itemCurrentFile->HasMusicInfoTag())
    {
      const CMusicInfoTag& tag = *m_itemCurrentFile->GetMusicInfoTag();
      if (tag.GetDuration() > 0)
        return (float)(GetTime() / tag.GetDuration() * 100);
    }

    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
    {
      double totalTime = GetTotalTime();
      if (totalTime > 0.0f)
        return (float)(GetTime() / totalTime * 100);
    }
    else
      return m_pPlayer->GetPercentage();
  }
  return 0.0f;
}

float CApplication::GetCachePercentage() const
{
  if (m_pPlayer->IsPlaying())
  {
    // Note that the player returns a relative cache percentage and we want an absolute percentage
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
    {
      float stackedTotalTime = (float) GetTotalTime();
      // We need to take into account the stack's total time vs. currently playing file's total time
      if (stackedTotalTime > 0.0f)
        return std::min( 100.0f, GetPercentage() + (m_pPlayer->GetCachePercentage() * m_pPlayer->GetTotalTime() * 0.001f / stackedTotalTime ) );
    }
    else
      return std::min( 100.0f, m_pPlayer->GetPercentage() + m_pPlayer->GetCachePercentage() );
  }
  return 0.0f;
}

void CApplication::SeekPercentage(float percent)
{
  if (m_pPlayer->IsPlaying() && (percent >= 0.0))
  {
    if (!m_pPlayer->CanSeek()) return;
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
      SeekTime(percent * 0.01 * GetTotalTime());
    else
      m_pPlayer->SeekPercentage(percent);
  }
}

// SwitchToFullScreen() returns true if a switch is made, else returns false
bool CApplication::SwitchToFullScreen(bool force /* = false */)
{
  // don't switch if the slideshow is active
  if (g_windowManager.GetFocusedWindow() == WINDOW_SLIDESHOW)
    return false;

  // if playing from the video info window, close it first!
  if (g_windowManager.HasModalDialog() && g_windowManager.GetTopMostModalDialogID() == WINDOW_DIALOG_VIDEO_INFO)
  {
    CGUIDialogVideoInfo* pDialog = (CGUIDialogVideoInfo*)g_windowManager.GetWindow(WINDOW_DIALOG_VIDEO_INFO);
    if (pDialog) pDialog->Close(true);
  }

  int windowID = WINDOW_INVALID;
  // See if we're playing a video, and are in GUI mode
  if (m_pPlayer->IsPlayingVideo() && g_windowManager.GetActiveWindow() != WINDOW_FULLSCREEN_VIDEO)
    windowID = WINDOW_FULLSCREEN_VIDEO;

  // special case for switching between GUI & visualisation mode. (only if we're playing an audio song)
  if (m_pPlayer->IsPlayingAudio() && g_windowManager.GetActiveWindow() != WINDOW_VISUALISATION)
    windowID = WINDOW_VISUALISATION;


  if (windowID != WINDOW_INVALID)
  {
    if (force)
      g_windowManager.ForceActivateWindow(windowID);
    else
      g_windowManager.ActivateWindow(windowID);
    return true;
  }

  return false;
}

void CApplication::Minimize()
{
  g_Windowing.Minimize();
}

std::string CApplication::GetCurrentPlayer()
{
  return m_pPlayer->GetCurrentPlayer();
}

void CApplication::UpdateLibraries()
{
  if (CSettings::GetInstance().GetBool(CSettings::SETTING_VIDEOLIBRARY_UPDATEONSTARTUP))
  {
    CLog::LogF(LOGNOTICE, "Starting video library startup scan");
    StartVideoScan("", !CSettings::GetInstance().GetBool(CSettings::SETTING_VIDEOLIBRARY_BACKGROUNDUPDATE));
  }

  if (CSettings::GetInstance().GetBool(CSettings::SETTING_MUSICLIBRARY_UPDATEONSTARTUP))
  {
    CLog::LogF(LOGNOTICE, "Starting music library startup scan");
    StartMusicScan("", !CSettings::GetInstance().GetBool(CSettings::SETTING_MUSICLIBRARY_BACKGROUNDUPDATE));
  }
}

bool CApplication::IsVideoScanning() const
{
  return CVideoLibraryQueue::GetInstance().IsScanningLibrary();
}

bool CApplication::IsMusicScanning() const
{
  return m_musicInfoScanner->IsScanning();
}

void CApplication::StopVideoScan()
{
  CVideoLibraryQueue::GetInstance().StopLibraryScanning();
}

void CApplication::StopMusicScan()
{
  if (m_musicInfoScanner->IsScanning())
    m_musicInfoScanner->Stop();
}

void CApplication::StartVideoCleanup(bool userInitiated /* = true */)
{
  if (userInitiated && CVideoLibraryQueue::GetInstance().IsRunning())
    return;

  std::set<int> paths;
  if (userInitiated)
    CVideoLibraryQueue::GetInstance().CleanLibraryModal(paths);
  else
    CVideoLibraryQueue::GetInstance().CleanLibrary(paths, true);
}

void CApplication::StartVideoScan(const std::string &strDirectory, bool userInitiated /* = true */, bool scanAll /* = false */)
{
  CVideoLibraryQueue::GetInstance().ScanLibrary(strDirectory, scanAll, userInitiated);
}

void CApplication::StartMusicCleanup(bool userInitiated /* = true */)
{
  if (m_musicInfoScanner->IsScanning())
    return;

  if (userInitiated)
    m_musicInfoScanner->CleanDatabase(true);
  else
  {
    m_musicInfoScanner->ShowDialog(false);
    m_musicInfoScanner->StartCleanDatabase();
  }
}

void CApplication::StartMusicScan(const std::string &strDirectory, bool userInitiated /* = true */, int flags /* = 0 */)
{
  if (m_musicInfoScanner->IsScanning())
    return;

  if (!flags)
  { // setup default flags
    if (CSettings::GetInstance().GetBool(CSettings::SETTING_MUSICLIBRARY_DOWNLOADINFO))
      flags |= CMusicInfoScanner::SCAN_ONLINE;
    if (!userInitiated || CSettings::GetInstance().GetBool(CSettings::SETTING_MUSICLIBRARY_BACKGROUNDUPDATE))
      flags |= CMusicInfoScanner::SCAN_BACKGROUND;
  }

  if (!(flags & CMusicInfoScanner::SCAN_BACKGROUND))
    m_musicInfoScanner->ShowDialog(true);

  m_musicInfoScanner->Start(strDirectory, flags);
}

void CApplication::StartMusicAlbumScan(const std::string& strDirectory,
                                       bool refresh)
{
  if (m_musicInfoScanner->IsScanning())
    return;

  m_musicInfoScanner->ShowDialog(true);

  m_musicInfoScanner->FetchAlbumInfo(strDirectory,refresh);
}

void CApplication::StartMusicArtistScan(const std::string& strDirectory,
                                        bool refresh)
{
  if (m_musicInfoScanner->IsScanning())
    return;

  m_musicInfoScanner->ShowDialog(true);

  m_musicInfoScanner->FetchArtistInfo(strDirectory,refresh);
}

bool CApplication::ProcessAndStartPlaylist(const std::string& strPlayList, CPlayList& playlist, int iPlaylist, int track)
{
  CLog::Log(LOGDEBUG,"CApplication::ProcessAndStartPlaylist(%s, %i)",strPlayList.c_str(), iPlaylist);

  // initial exit conditions
  // no songs in playlist just return
  if (playlist.size() == 0)
    return false;

  // illegal playlist
  if (iPlaylist < PLAYLIST_MUSIC || iPlaylist > PLAYLIST_VIDEO)
    return false;

  // setup correct playlist
  g_playlistPlayer.ClearPlaylist(iPlaylist);

  // if the playlist contains an internet stream, this file will be used
  // to generate a thumbnail for musicplayer.cover
  g_application.m_strPlayListFile = strPlayList;

  // add the items to the playlist player
  g_playlistPlayer.Add(iPlaylist, playlist);

  // if we have a playlist
  if (g_playlistPlayer.GetPlaylist(iPlaylist).size())
  {
    // start playing it
    g_playlistPlayer.SetCurrentPlaylist(iPlaylist);
    g_playlistPlayer.Reset();
    g_playlistPlayer.Play(track, "");
    return true;
  }
  return false;
}

bool CApplication::IsCurrentThread() const
{
  return CThread::IsCurrentThread(m_threadID);
}

void CApplication::SetRenderGUI(bool renderGUI)
{
  if (renderGUI && ! m_renderGUI)
    g_windowManager.MarkDirty();
  m_renderGUI = renderGUI;
}

CNetwork& CApplication::getNetwork()
{
  return *m_network;
}
#ifdef HAS_PERFORMANCE_SAMPLE
CPerformanceStats &CApplication::GetPerformanceStats()
{
  return m_perfStats;
}
#endif

bool CApplication::SetLanguage(const std::string &strLanguage)
{
  // nothing to be done if the language hasn't changed
  if (strLanguage == CSettings::GetInstance().GetString(CSettings::SETTING_LOCALE_LANGUAGE))
    return true;

  return CSettings::GetInstance().SetString(CSettings::SETTING_LOCALE_LANGUAGE, strLanguage);
}

bool CApplication::LoadLanguage(bool reload)
{
  // load the configured langauge
  if (!g_langInfo.SetLanguage(m_fallbackLanguageLoaded, "", reload))
    return false;

  // set the proper audio and subtitle languages
  g_langInfo.SetAudioLanguage(CSettings::GetInstance().GetString(CSettings::SETTING_LOCALE_AUDIOLANGUAGE));
  g_langInfo.SetSubtitleLanguage(CSettings::GetInstance().GetString(CSettings::SETTING_LOCALE_SUBTITLELANGUAGE));

  return true;
}

void CApplication::SetLoggingIn(bool switchingProfiles)
{
  // don't save skin settings on unloading when logging into another profile
  // because in that case we have already loaded the new profile and
  // would therefore write the previous skin's settings into the new profile
  // instead of into the previous one
  m_saveSkinOnUnloading = !switchingProfiles;

  // make sure that the autoexec.py script is executed after logging in
  m_autoExecScriptExecuted = true;
}

void CApplication::CloseNetworkShares()
{
  CLog::Log(LOGDEBUG,"CApplication::CloseNetworkShares: Closing all network shares");

#if defined(HAS_FILESYSTEM_SMB) && !defined(TARGET_WINDOWS)
  smb.Deinit();
#endif
  
#ifdef HAS_FILESYSTEM_NFS
  gNfsConnection.Deinit();
#endif

#ifdef HAS_FILESYSTEM_SFTP
  CSFTPSessionManager::DisconnectAllSessions();
#endif
}

void CApplication::RegisterActionListener(IActionListener *listener)
{
  CSingleLock lock(m_critSection);
  std::vector<IActionListener *>::iterator it = std::find(m_actionListeners.begin(), m_actionListeners.end(), listener);
  if (it == m_actionListeners.end())
    m_actionListeners.push_back(listener);
}

void CApplication::UnregisterActionListener(IActionListener *listener)
{
  CSingleLock lock(m_critSection);
  std::vector<IActionListener *>::iterator it = std::find(m_actionListeners.begin(), m_actionListeners.end(), listener);
  if (it != m_actionListeners.end())
    m_actionListeners.erase(it);
}

bool CApplication::NotifyActionListeners(const CAction &action) const
{
  CSingleLock lock(m_critSection);
  for (std::vector<IActionListener *>::const_iterator it = m_actionListeners.begin(); it != m_actionListeners.end(); ++it)
  {
    if ((*it)->OnAction(action))
      return true;
  }
  
  return false;
}
