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

#include "GameClient.h"
#include "addons/AddonManager.h"
#include "cores/IPlayer.h"
#include "FileItem.h"
#include "filesystem/SpecialProtocol.h"
#include "games/GameManager.h"
#include "input/joysticks/JoystickTypes.h"
#include "input/PortManager.h"
#include "settings/Settings.h"
#include "threads/SingleLock.h"
#include "URL.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

#include <algorithm>
#include <assert.h>

using namespace ADDON;
using namespace GAME;
using namespace XFILE;

#define EXTENSION_SEPARATOR          "|"
#define EXTENSION_WILDCARD           "*"
#define GAME_REGION_NTSC_STRING      "NTSC"
#define GAME_REGION_PAL_STRING       "PAL"

// --- NormalizeExtension ------------------------------------------------------

/*
 * \brief Convert to lower case and canonicalize with a leading "."
 */
std::string NormalizeExtension(const std::string& strExtension)
{
  std::string ext = strExtension;

  if (!ext.empty() && ext != EXTENSION_WILDCARD)
  {
    StringUtils::ToLower(ext);

    if (ext[0] != '.')
      ext.insert(0, ".");
  }

  return ext;
}

// --- CControllerInput ------------------------------------------------------------

CControllerInput::CControllerInput(CGameClient* addon, int port, const GameControllerPtr& controller)
  : m_addon(addon),
    m_port(port),
    m_controller(controller)
{
  assert(m_addon != NULL);
  assert(controller.get() != NULL);
}

std::string CControllerInput::ControllerID(void) const
{
  return m_controller->ID();
}

InputType CControllerInput::GetInputType(const std::string& feature) const
{
  const std::vector<CGameControllerFeature>& features = m_controller->Layout().Features();

  for (std::vector<CGameControllerFeature>::const_iterator it = features.begin(); it != features.end(); ++it)
  {
    if (feature == it->Name())
    {
      switch (it->Type())
      {
        case FEATURE_BUTTON:
          if (it->ButtonType() == BUTTON_DIGITAL)
            return INPUT_TYPE_DIGITAL;
          else if (it->ButtonType() == BUTTON_ANALOG)
            return INPUT_TYPE_ANALOG;
          break;

        default:
          break;
      }
      break;
    }
  }

  return INPUT_TYPE_UNKNOWN;
}

bool CControllerInput::OnButtonPress(const std::string& feature, bool bPressed)
{
  return m_addon->OnButtonPress(m_port, feature, bPressed);
}

bool CControllerInput::OnButtonMotion(const std::string& feature, float magnitude)
{
  return m_addon->OnButtonMotion(m_port, feature, magnitude);
}

bool CControllerInput::OnAnalogStickMotion(const std::string& feature, float x, float y)
{
  return m_addon->OnAnalogStickMotion(m_port, feature, x, y);
}

bool CControllerInput::OnAccelerometerMotion(const std::string& feature, float x, float y, float z)
{
  return m_addon->OnAccelerometerMotion(m_port, feature, x, y, z);
}

// --- CGameClient -------------------------------------------------------------

CGameClient::CGameClient(const AddonProps& props)
  : CAddonDll<DllGameClient, GameClient, game_client_properties>(props),
    m_apiVersion("0.0.0"),
    m_libraryProps(this),
    m_strGameClientPath(CAddon::LibPath())
{
  InitializeProperties();

  InfoMap::const_iterator it;
  /*
  if ((it = props.extrainfo.find("platforms")) != props.extrainfo.end())
    SetPlatforms(it->second);
  */
  if ((it = props.extrainfo.find("extensions")) != props.extrainfo.end())
  {
    std::vector<std::string> extensions = ParseExtensions(it->second);
    m_extensions.insert(extensions.begin(), extensions.end());
  }
  if ((it = props.extrainfo.find("supports_vfs")) != props.extrainfo.end())
    m_bSupportsVFS = (it->second == "true" || it->second == "yes");
  if ((it = props.extrainfo.find("supports_no_game")) != props.extrainfo.end())
    m_bSupportsStandalone = (it->second == "true" || it->second == "yes");
}

CGameClient::CGameClient(const cp_extension_t* ext)
  : CAddonDll<DllGameClient, GameClient, game_client_properties>(ext),
    m_apiVersion("0.0.0"),
    m_libraryProps(this),
    m_strGameClientPath(CAddon::LibPath())
{
  InitializeProperties();

  if (ext)
  {
    /*
    string strPlatforms = CAddonMgr::Get().GetExtValue(ext->configuration, "platforms");
    if (!strPlatforms.empty())
    {
      Props().extrainfo.insert(make_pair("platforms", strPlatforms));
      SetPlatforms(strPlatforms);
    }
    */
    std::string strExtensions = CAddonMgr::Get().GetExtValue(ext->configuration, "extensions");
    if (!strExtensions.empty())
    {
      Props().extrainfo.insert(make_pair("extensions", strExtensions));
      std::vector<std::string> extensions = ParseExtensions(strExtensions);
      m_extensions.insert(extensions.begin(), extensions.end());
    }
    std::string strSupportsVFS = CAddonMgr::Get().GetExtValue(ext->configuration, "supports_vfs");
    if (!strSupportsVFS.empty())
    {
      Props().extrainfo.insert(make_pair("supports_vfs", strSupportsVFS));
      m_bSupportsVFS = (strSupportsVFS == "true" || strSupportsVFS == "yes");
    }
    std::string strSupportsStandalone = CAddonMgr::Get().GetExtValue(ext->configuration, "supports_no_game");
    if (!strSupportsStandalone.empty())
    {
      Props().extrainfo.insert(make_pair("supports_no_game", strSupportsStandalone));
      m_bSupportsStandalone = (strSupportsStandalone == "true" || strSupportsStandalone == "yes");
    }
  }
}

void CGameClient::InitializeProperties(void)
{
  m_bSupportsVFS = false;
  m_bSupportsStandalone = false;
  m_bIsPlaying = false;
  m_player = NULL;
  m_region = GAME_REGION_NTSC;
  m_frameRate = 0.0;
  m_frameRateCorrection = 1.0;
  m_sampleRate = 0.0;
  m_serializeSize = 0;
  m_bRewindEnabled = false;
  m_pInfo = m_libraryProps.CreateProps();
}

CGameClient::~CGameClient(void)
{
  if (m_bIsPlaying && m_player)
    m_player->CloseFile();

  SAFE_DELETE(m_pInfo);
}

AddonPtr CGameClient::GetRunningInstance() const
{
  GameClientPtr gameAddon;
  if (CGameManager::Get().GetClient(ID(), gameAddon))
    return std::dynamic_pointer_cast<CAddon>(gameAddon);

  return CAddon::GetRunningInstance();
}

void CGameClient::OnEnabled()
{
  CGameManager::Get().RegisterAddon(std::dynamic_pointer_cast<CGameClient>(GetRunningInstance()));
}

void CGameClient::OnDisabled()
{
  CGameManager::Get().UnregisterAddonByID(ID());
}

const std::string CGameClient::LibPath() const
{
  // Use helper library add-on to load libretro cores
  // TODO: Compare helper version with required dependency
  const ADDONDEPS& dependencies = GetDeps();
  ADDONDEPS::const_iterator it = dependencies.find(LIBRETRO_WRAPPER_LIBRARY);
  if (it != dependencies.end())
  {
    AddonPtr addon;
    if (CAddonMgr::Get().GetAddon(LIBRETRO_WRAPPER_LIBRARY, addon, ADDON_GAMEDLL) && addon)
      return addon->LibPath();
  }

  return CAddon::LibPath();
}

bool CGameClient::CanOpen(const CFileItem& file) const
{
  // Filter by Addon.ID property
  if (file.HasProperty("Addon.ID") && file.GetProperty("Addon.ID").asString() != ID())
    return false;

  // Filter by extension
  if (!IsExtensionValid(URIUtils::GetExtension(file.GetPath())))
    return false;

  // If the file is on the VFS, the game client must support VFS
  CURL translatedUrl(CSpecialProtocol::TranslatePath(file.GetPath()));

  const bool bIsLocalFS = translatedUrl.GetProtocol() == "file" || translatedUrl.GetProtocol().empty();

  if (!bIsLocalFS && !SupportsVFS())
    return false;

  return true;
}

bool CGameClient::OpenFile(const CFileItem& file, IPlayer* player)
{
  CSingleLock lock(m_critSection);

  if (!Initialized())
    return false;

  if (!m_bSupportsStandalone && !CanOpen(file))
    return false;

  CloseFile();

  GAME_ERROR error = GAME_ERROR_FAILED;
  std::string strFilePath;

  if (m_bSupportsStandalone)
  {
    try { LogError(error = m_pStruct->LoadStandalone(), "LoadStandalone()"); }
    catch (...) { LogException("LoadStandalone()"); }
  }
  else
  {
    // Try to resolve path to a local file, as not all game clients support VFS
    CURL translatedUrl(CSpecialProtocol::TranslatePath(file.GetPath()));
    if (translatedUrl.GetProtocol() == "file")
      translatedUrl.SetProtocol("");

    strFilePath = translatedUrl.Get();

    try { LogError(error = m_pStruct->LoadGame(strFilePath.c_str()), "LoadGame()"); }
    catch (...) { LogException("LoadGame()"); }
  }

  if (error == GAME_ERROR_NO_ERROR && LoadGameInfo())
  {
    m_filePath   = strFilePath;
    m_player     = player;
    m_bIsPlaying = true;

    InitSerialization();

    return true;
  }

  return false;
}

bool CGameClient::LoadGameInfo()
{
  // Get information about system audio/video timings and geometry
  // Can be called only after retro_load_game()
  game_system_av_info av_info = { };

  GAME_ERROR error = GAME_ERROR_FAILED;
  try { LogError(error = m_pStruct->GetGameInfo(&av_info), "GetGameInfo()"); }
  catch (...) { LogException("GetGameInfo()"); }

  if (error != GAME_ERROR_NO_ERROR)
    return false;

  GAME_REGION region;
  try { region = m_pStruct->GetRegion(); }
  catch (...) { LogException("GetRegion()"); return false; }

  CLog::Log(LOGINFO, "GAME: ---------------------------------------");
  CLog::Log(LOGINFO, "GAME: Opened file %s",   m_filePath.c_str());
  CLog::Log(LOGINFO, "GAME: Base Width:   %u", av_info.geometry.base_width);
  CLog::Log(LOGINFO, "GAME: Base Height:  %u", av_info.geometry.base_height);
  CLog::Log(LOGINFO, "GAME: Max Width:    %u", av_info.geometry.max_width);
  CLog::Log(LOGINFO, "GAME: Max Height:   %u", av_info.geometry.max_height);
  CLog::Log(LOGINFO, "GAME: Aspect Ratio: %f", av_info.geometry.aspect_ratio);
  CLog::Log(LOGINFO, "GAME: FPS:          %f", av_info.timing.fps);
  CLog::Log(LOGINFO, "GAME: Sample Rate:  %f", av_info.timing.sample_rate);
  CLog::Log(LOGINFO, "GAME: Region:       %s", region == GAME_REGION_NTSC ? GAME_REGION_NTSC_STRING : GAME_REGION_PAL_STRING);
  CLog::Log(LOGINFO, "GAME: ---------------------------------------");

  m_frameRate  = av_info.timing.fps;
  m_sampleRate = av_info.timing.sample_rate;
  m_region     = region;

  return true;
}

bool CGameClient::InitSerialization()
{
  // Check if serialization is supported so savestates and rewind can be used
  unsigned int serializeSize;
  try { serializeSize = m_pStruct->SerializeSize(); }
  catch (...) { LogException("SerializeSize()"); return false; }

  if (serializeSize == 0)
  {
    CLog::Log(LOGINFO, "GAME: Serialization not supported, continuing without save or rewind");
    return false;
  }

  m_serializeSize = serializeSize;
  m_bRewindEnabled = CSettings::Get().GetBool("gamesgeneral.enablerewind");

  // Set up rewind functionality
  if (m_bRewindEnabled)
  {
    m_serialState.Init(m_serializeSize, (size_t)(CSettings::Get().GetInt("gamesgeneral.rewindtime") * GetFrameRate()));

    GAME_ERROR error = GAME_ERROR_FAILED;
    try { LogError(error = m_pStruct->Serialize(m_serialState.GetState(), m_serialState.GetFrameSize()), "Serialize()"); }
    catch (...) { LogException("Serialize()"); }

    if (error != GAME_ERROR_NO_ERROR)
    {
      m_serializeSize = 0;
      m_bRewindEnabled = false;
      m_serialState.Reset();
      CLog::Log(LOGERROR, "GAME: Unable to serialize state, proceeding without save or rewind");
      return false;
    }
  }

  return true;
}

void CGameClient::Reset()
{
  if (m_bIsPlaying)
  {
    // TODO: Reset all controller ports to their same value. bSNES since v073r01
    // resets controllers to JOYPAD after a reset, so guard against this.
    try { LogError(m_pStruct->Reset(), "Reset()"); }
    catch (...) { LogException("Reset()"); }

    if (m_bRewindEnabled)
    {
      m_serialState.ReInit();

      GAME_ERROR error = GAME_ERROR_FAILED;
      try { LogError(error = m_pStruct->Serialize(m_serialState.GetNextState(), m_serialState.GetFrameSize()), "Serialize()"); }
      catch (...) { LogException("Serialize()"); }

      if (error != GAME_ERROR_NO_ERROR)
        m_bRewindEnabled = false;
    }
  }
}

void CGameClient::CloseFile()
{
  CSingleLock lock(m_critSection);

  if (Initialized() && m_bIsPlaying)
  {
    try { LogError(m_pStruct->UnloadGame(), "UnloadGame()"); }
    catch (...) { LogException("UnloadGame()"); }
  }

  ClearPorts();

  m_bIsPlaying = false;
  m_filePath.clear();
  m_player = NULL;
}

void CGameClient::RunFrame()
{
  CSingleLock lock(m_critSection);

  if (!m_bIsPlaying)
    return;

  try { m_pStruct->FrameEvent(); }
  catch (...) { LogException("FrameEvent()"); }

  // Append a new state delta to the rewind buffer
  if (m_bRewindEnabled)
  {
	GAME_ERROR error = GAME_ERROR_FAILED;
    try { LogError(error = m_pStruct->Serialize(m_serialState.GetNextState(), m_serialState.GetFrameSize()), "Serialize()"); }
    catch (...) { LogException("Serialize()"); }

    if (error == GAME_ERROR_NO_ERROR)
      m_serialState.AdvanceFrame();
    else
      m_bRewindEnabled = false;
  }
}

unsigned int CGameClient::RewindFrames(unsigned int frames)
{
  CSingleLock lock(m_critSection);

  unsigned int rewound = 0;
  if (m_bIsPlaying && m_bRewindEnabled)
  {
    rewound = m_serialState.RewindFrames(frames);
    if (rewound != 0)
    {
      try { LogError(m_pStruct->Deserialize(m_serialState.GetState(), m_serialState.GetFrameSize()), "Deserialize()"); }
      catch (...) { LogException("Deserialize()"); }
    }
  }
  return rewound;
}

bool CGameClient::OpenPort(unsigned int port)
{
  GameControllerVector controllers = GetControllers();
  if (!controllers.empty()) // TODO: Choose controller
  {
    if (port >= m_controllers.size())
      m_controllers.resize(port + 1);

    ClosePort(port);

    m_controllers[port] = new CControllerInput(this, port, controllers[0]);

    CPortManager::Get().OpenPort(m_controllers[port], port);

    UpdatePort(port, controllers[0]);

    return true;
  }

  return false;
}

void CGameClient::ClosePort(unsigned int port)
{
  if (port >= m_controllers.size())
    return;

  if (m_controllers[port] != NULL)
  {
    CPortManager::Get().ClosePort(m_controllers[port]);

    delete m_controllers[port];
    m_controllers[port] = NULL;

    UpdatePort(port, CGameController::EmptyPtr);
  }
}

void CGameClient::ClearPorts(void)
{
  for (unsigned int i = 0; i < m_controllers.size(); i++)
    ClosePort(i);
}

void CGameClient::UpdatePort(unsigned int port, const GameControllerPtr& controller)
{
  if (controller != CGameController::EmptyPtr)
  {
    game_controller controllerStruct;

    const std::string strId = controller->ID();
    controllerStruct.controller_id        = strId.c_str();
    controllerStruct.digital_button_count = controller->Layout().FeatureCount(FEATURE_BUTTON, BUTTON_DIGITAL);
    controllerStruct.analog_button_count  = controller->Layout().FeatureCount(FEATURE_BUTTON, BUTTON_ANALOG);
    controllerStruct.analog_stick_count   = controller->Layout().FeatureCount(FEATURE_ANALOG_STICK);
    controllerStruct.accelerometer_count  = controller->Layout().FeatureCount(FEATURE_ACCELEROMETER);
    controllerStruct.key_count            = controller->Layout().FeatureCount(FEATURE_KEY);
    controllerStruct.rel_pointer_count    = controller->Layout().FeatureCount(FEATURE_RELATIVE_POINTER);
    controllerStruct.abs_pointer_count    = controller->Layout().FeatureCount(FEATURE_ABSOLUTE_POINTER);

    try { m_pStruct->UpdatePort(port, true, &controllerStruct); }
    catch (...) { LogException("UpdatePort()"); }
  }
  else
  {
    try { m_pStruct->UpdatePort(port, false, NULL); }
    catch (...) { LogException("UpdatePort()"); }
  }
}

GameControllerVector CGameClient::GetControllers(void) const
{
  GameControllerVector controllers;

  const ADDONDEPS& dependencies = GetDeps();
  for (ADDONDEPS::const_iterator it = dependencies.begin(); it != dependencies.end(); ++it)
  {
    AddonPtr addon;
    if (CAddonMgr::Get().GetAddon(it->first, addon, ADDON_GAME_CONTROLLER))
    {
      GameControllerPtr controller = std::dynamic_pointer_cast<CGameController>(addon);
      if (controller)
        controllers.push_back(controller);
    }
  }

  if (controllers.empty())
  {
    // Use the default controller
    AddonPtr addon;
    if (CAddonMgr::Get().GetAddon(DEFAULT_GAME_CONTROLLER, addon, ADDON_GAME_CONTROLLER))
      controllers.push_back(std::dynamic_pointer_cast<CGameController>(addon));
  }

  return controllers;
}

bool CGameClient::OnButtonPress(int port, const std::string& feature, bool bPressed)
{
  std::string strControllerId = m_controllers[port]->Controller()->ID();

  bool bHandled = false;

  game_input_event event;

  event.type                   = GAME_INPUT_EVENT_DIGITAL_BUTTON;
  event.port                   = port;
  event.controller_id          = strControllerId.c_str();
  event.feature_name           = feature.c_str();
  event.digital_button.pressed = bPressed;

  try { bHandled = m_pStruct->InputEvent(port, &event); }
  catch (...) { LogException("InputEvent()"); }

  return bHandled;
}

bool CGameClient::OnButtonMotion(int port, const std::string& feature, float magnitude)
{
  std::string strControllerId = m_controllers[port]->Controller()->ID();

  bool bHandled = false;

  game_input_event event;

  event.type                    = GAME_INPUT_EVENT_ANALOG_BUTTON;
  event.port                    = port;
  event.controller_id           = strControllerId.c_str();
  event.feature_name            = feature.c_str();
  event.analog_button.magnitude = magnitude;

  try { bHandled = m_pStruct->InputEvent(port, &event); }
  catch (...) { LogException("InputEvent()"); }

  return bHandled;
}

bool CGameClient::OnAnalogStickMotion(int port, const std::string& feature, float x, float y)
{
  std::string strControllerId = m_controllers[port]->Controller()->ID();

  bool bHandled = false;

  game_input_event event;

  event.type           = GAME_INPUT_EVENT_ANALOG_STICK;
  event.port           = port;
  event.controller_id  = strControllerId.c_str();
  event.feature_name   = feature.c_str();
  event.analog_stick.x = x;
  event.analog_stick.y = y;

  try { bHandled = m_pStruct->InputEvent(port, &event); }
  catch (...) { LogException("InputEvent()"); }

  return bHandled;
}

bool CGameClient::OnAccelerometerMotion(int port, const std::string& feature, float x, float y, float z)
{
  std::string strControllerId = m_controllers[port]->Controller()->ID();

  bool bHandled = false;

  game_input_event event;

  event.type            = GAME_INPUT_EVENT_ACCELEROMETER;
  event.port            = port;
  event.controller_id   = strControllerId.c_str();
  event.feature_name    = feature.c_str();
  event.accelerometer.x = x;
  event.accelerometer.y = y;
  event.accelerometer.z = z;

  try { bHandled = m_pStruct->InputEvent(port, &event); }
  catch (...) { LogException("InputEvent()"); }

  return bHandled;
}

void CGameClient::SetFrameRateCorrection(double correctionFactor)
{
  if (correctionFactor != 0.0)
    m_frameRateCorrection = correctionFactor;
  if (m_bRewindEnabled)
    m_serialState.SetMaxFrames((size_t)(CSettings::Get().GetInt("gamesgeneral.rewindtime") * GetFrameRate()));
}

bool CGameClient::IsExtensionValid(const std::string& strExtension) const
{
  if (strExtension.empty())
    return false;

  if (m_extensions.empty())
    return false; // Game client didn't provide any extensions

  if (m_extensions.find(EXTENSION_WILDCARD) != m_extensions.end())
    return true; // Game client accepts all extensions

  return m_extensions.find(NormalizeExtension(strExtension)) != m_extensions.end();
}

std::vector<std::string> CGameClient::ParseExtensions(const std::string& strExtensionList)
{
  std::vector<std::string> extensions = StringUtils::Split(strExtensionList, EXTENSION_SEPARATOR);

  std::transform(extensions.begin(), extensions.end(), extensions.begin(), NormalizeExtension);

  return extensions;
}

/*
void CGameClient::SetPlatforms(const string& strPlatformList)
{
  m_platforms.clear();

  vector<string> platforms = StringUtils::Split(strPlatformList, EXTENSION_SEPARATOR);
  for (vector<string>::iterator it = platforms.begin(); it != platforms.end(); it++)
  {
    StringUtils::Trim(*it);
    GamePlatform id = CGameInfoTagLoader::GetPlatformInfoByName(*it).id;
    if (id != PLATFORM_UNKNOWN)
      m_platforms.insert(id);
  }
}
*/

bool CGameClient::LogError(GAME_ERROR error, const char* strMethod) const
{
  if (error != GAME_ERROR_NO_ERROR)
  {
    CLog::Log(LOGERROR, "GAME - %s - addon '%s' returned an error: %s",
        strMethod, ID().c_str(), ToString(error));
    return false;
  }
  return true;
}

void CGameClient::LogException(const char* strFunctionName) const
{
  CLog::Log(LOGERROR, "GAME: exception caught while trying to call '%s' on add-on %s",
      strFunctionName, ID().c_str());
  CLog::Log(LOGERROR, "Please contact the developer of this add-on: %s", Author().c_str());
}

void CGameClient::LogAddonProperties(void) const
{
  std::vector<std::string> vecExtensions(m_extensions.begin(), m_extensions.end());

  CLog::Log(LOGINFO, "GAME: ------------------------------------");
  CLog::Log(LOGINFO, "GAME: Loaded DLL for %s", ID().c_str());
  CLog::Log(LOGINFO, "GAME: Client: %s at version %s", Name().c_str(), Version().asString().c_str());
  CLog::Log(LOGINFO, "GAME: Valid extensions: %s", StringUtils::Join(vecExtensions, " ").c_str());
  CLog::Log(LOGINFO, "GAME: Supports VFS: %s", m_bSupportsVFS ? "yes" : "no");
  CLog::Log(LOGINFO, "GAME: Supports standalone execution: %s", m_bSupportsStandalone ? "yes" : "no");
  CLog::Log(LOGINFO, "GAME: ------------------------------------");
}

const char* CGameClient::ToString(GAME_ERROR error)
{
  switch (error)
  {
    case GAME_ERROR_NO_ERROR:
      return "no error";
    case GAME_ERROR_NOT_IMPLEMENTED:
      return "not implemented";
    case GAME_ERROR_REJECTED:
      return "rejected by the client";
    case GAME_ERROR_INVALID_PARAMETERS:
      return "invalid parameters for this method";
    case GAME_ERROR_FAILED:
      return "the command failed";
    case GAME_ERROR_UNKNOWN:
    default:
      return "unknown error";
  }
}
