/*
 *      Copyright (C) 2014 Team XBMC
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

#include "ClientBridge.h"
#include "libretro.h"
#include "LibretroDLL.h"
#include "LibretroEnvironment.h"
#include "libXBMC_addon.h"
#include "libXBMC_game.h"
#include "xbmc_addon_dll.h"
#include "xbmc_game_dll.h"

#include <string>

using namespace ADDON;
using namespace LIBRETRO;
using namespace std;

#define GAME_CLIENT_NAME_UNKNOWN      "Unknown libretro core"
#define GAME_CLIENT_VERSION_UNKNOWN   "0.0.0"

#ifndef SAFE_DELETE
#define SAFE_DELETE(x)  do { delete x; x = NULL; } while (0)
#endif

CHelper_libXBMC_addon*  XBMC          = NULL;
CHelper_libXBMC_game*   FRONTEND      = NULL;
CLibretroDLL*           CLIENT        = NULL;
CClientBridge*          CLIENT_BRIDGE = NULL;

extern "C"
{

ADDON_STATUS ADDON_Create(void* callbacks, void* props)
{
  try
  {
    if (!callbacks || !props)
      throw ADDON_STATUS_UNKNOWN;

    game_client_properties* libretro_props = static_cast<game_client_properties*>(props);

    if (!libretro_props->shared_library)
      throw ADDON_STATUS_PERMANENT_FAILURE;
    
    XBMC = new CHelper_libXBMC_addon;
    if (!XBMC || !XBMC->RegisterMe(callbacks))
      throw ADDON_STATUS_PERMANENT_FAILURE;

    FRONTEND = new CHelper_libXBMC_game;
    if (!FRONTEND || !FRONTEND->RegisterMe(callbacks))
      throw ADDON_STATUS_PERMANENT_FAILURE;

    if (libretro_props->version_major != 1)
    {
      XBMC->Log(LOG_ERROR, "Wrapper not needed, libretro version = %u", libretro_props->version_major);
      throw ADDON_STATUS_PERMANENT_FAILURE;
    }

    CLIENT = new CLibretroDLL(XBMC);
    if (!CLIENT->Load(libretro_props->shared_library))
    {
      XBMC->Log(LOG_ERROR, "Failed to load ", libretro_props->version_major);
      throw ADDON_STATUS_PERMANENT_FAILURE;
    }

    unsigned int version = CLIENT->retro_api_version();
    if (version != 1)
    {
      XBMC->Log(LOG_ERROR, "Expected libretro api v1, found version %u", version);
      throw ADDON_STATUS_PERMANENT_FAILURE;
    }

    // Environment must be initialized before calling retro_init()
    CLIENT_BRIDGE = new CClientBridge;
    CLibretroEnvironment::Initialize(XBMC, FRONTEND, CLIENT, CLIENT_BRIDGE);

    CLIENT->retro_init();
  }
  catch (const ADDON_STATUS& status)
  {
    SAFE_DELETE(XBMC);
    SAFE_DELETE(FRONTEND);
    SAFE_DELETE(CLIENT);
    SAFE_DELETE(CLIENT_BRIDGE);
    return status;
  }

  return ADDON_STATUS_OK;
}

void ADDON_Stop()
{
}

void ADDON_Destroy()
{
  if (CLIENT)
    CLIENT->retro_deinit();
  SAFE_DELETE(XBMC);
  SAFE_DELETE(FRONTEND);
  SAFE_DELETE(CLIENT);
  SAFE_DELETE(CLIENT_BRIDGE);
}

ADDON_STATUS ADDON_GetStatus()
{
  return XBMC ? ADDON_STATUS_OK : ADDON_STATUS_UNKNOWN;
}

bool ADDON_HasSettings()
{
  return false;
}

unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
{
  return 0;
}

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  if (!settingName || !settingValue)
    return ADDON_STATUS_UNKNOWN;

  return ADDON_STATUS_OK;
}

void ADDON_FreeSettings()
{
}

void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
{
}
  
const char* GetGameAPIVersion(void)
{
  return GAME_API_VERSION;
}

const char* GetMininumGameAPIVersion(void)
{
  return GAME_MIN_API_VERSION;
}

// Split out from retro_get_system_info()
const char* GetClientName(void)
{
  static string strClientName = GAME_CLIENT_NAME_UNKNOWN;
  
  if (strClientName == GAME_CLIENT_NAME_UNKNOWN && CLIENT)
  {
    retro_system_info info = { };
    CLIENT->retro_get_system_info(&info);
    if (info.library_name)
      strClientName = info.library_name;
  }

  return strClientName.c_str();
}

// Split out from retro_get_system_info()
const char* GetClientVersion(void)
{
  static string strClientVersion = GAME_CLIENT_VERSION_UNKNOWN;
  
  if (strClientVersion == GAME_CLIENT_VERSION_UNKNOWN && CLIENT)
  {
    retro_system_info info = { };
    CLIENT->retro_get_system_info(&info);
    if (info.library_version)
      strClientVersion = info.library_version;
  }

  return strClientVersion.c_str();
}

// Split out from retro_get_system_info()
const char* GetValidExtensions(void)
{
  static string strValidExtensions;
  
  if (strValidExtensions.empty() && CLIENT)
  {
    retro_system_info info = { };
    CLIENT->retro_get_system_info(&info);
    if (info.valid_extensions)
      strValidExtensions = info.valid_extensions;
  }

  return strValidExtensions.c_str();
}

// Split out from retro_get_system_info()
bool AllowVFS(void)
{
  static bool bAllowVfs = false;
  
  if (!bAllowVfs && CLIENT)
  {
    retro_system_info info = { };
    CLIENT->retro_get_system_info(&info);

    // need_fullpath indicates that libretro requires a valid pathname in
    // retro_game_info::path when calling retro_load_game(). Conversely, if
    // need_fullpath is false, retro_load_game() can load from the memory
    // block specified by retro_game_info::data.
    bAllowVfs = !info.need_fullpath;
  }

  return bAllowVfs;
}

// Split out from retro_get_system_info()
bool RequireArchive(void)
{
  static bool bRequireArchive = false;
  
  if (!bRequireArchive && CLIENT)
  {
    retro_system_info info = { };
    CLIENT->retro_get_system_info(&info);
    // "require archive" is a more direct way of saying "block extract"
    bRequireArchive = info.block_extract;
  }

  return bRequireArchive;
}

GAME_ERROR LoadGame(const char* url)
{
  if (!CLIENT)
    return GAME_ERROR_FAILED;

  if (url == NULL)
    return GAME_ERROR_INVALID_PARAMETERS;

  retro_game_info info = { };
  info.path = url;
  bool result = CLIENT->retro_load_game(&info);

  return result ? GAME_ERROR_NO_ERROR : GAME_ERROR_FAILED;
}

GAME_ERROR LoadGameSpecial(GAME_TYPE type, const char** urls, size_t num_urls)
{
  if (!CLIENT)
    return GAME_ERROR_FAILED;

  if (urls == NULL || num_urls == 0)
    return GAME_ERROR_INVALID_PARAMETERS;

  retro_game_info* info = new retro_game_info[num_urls];
  for (unsigned int i = 0; i < num_urls; i++)
    info[i].path = urls[i];
  bool result = CLIENT->retro_load_game_special(type, info, num_urls);
  delete[] info;

  return result ? GAME_ERROR_NO_ERROR : GAME_ERROR_FAILED;
}

GAME_ERROR UnloadGame(void)
{
  if (!CLIENT)
    return GAME_ERROR_FAILED;

  CLIENT->retro_unload_game();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR Run(void)
{
  if (!CLIENT)
    return GAME_ERROR_FAILED;

  CLIENT->retro_run();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR Reset(void)
{
  if (!CLIENT)
    return GAME_ERROR_FAILED;

  CLIENT->retro_reset();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR KeyboardEvent(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers)
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->KeyboardEvent(down, keycode, character, key_modifiers);
}

GAME_ERROR GetSystemAVInfo(game_system_av_info *info)
{
  if (!CLIENT)
    return GAME_ERROR_FAILED;
  
  if (info == NULL)
    return GAME_ERROR_INVALID_PARAMETERS;

  retro_system_av_info retro_info = { };
  CLIENT->retro_get_system_av_info(&retro_info);

  info->geometry.base_width   = retro_info.geometry.base_width;
  info->geometry.base_height  = retro_info.geometry.base_height;
  info->geometry.max_width    = retro_info.geometry.max_width;
  info->geometry.max_height   = retro_info.geometry.max_height;
  info->geometry.aspect_ratio = retro_info.geometry.aspect_ratio;
  info->timing.fps            = retro_info.timing.fps;
  info->timing.sample_rate    = retro_info.timing.sample_rate;

  if (info->timing.fps != 0.0)
  {
    // Report fps to CLibretroEnvironment
    CLibretroEnvironment::UpdateFramerate(info->timing.fps);
  }

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR SetControllerPortDevice(unsigned port, unsigned device)
{
  if (!CLIENT)
    return GAME_ERROR_FAILED;

  CLIENT->retro_set_controller_port_device(port, device);

  return GAME_ERROR_NO_ERROR;
}

size_t SerializeSize(void)
{
  if (!CLIENT)
    return 0;

  return CLIENT->retro_serialize_size();
}

GAME_ERROR Serialize(void *data, size_t size)
{
  if (!CLIENT)
    return GAME_ERROR_FAILED;

  if (data == NULL)
    return GAME_ERROR_INVALID_PARAMETERS;

  bool result = CLIENT->retro_serialize(data, size);

  return result ? GAME_ERROR_NO_ERROR : GAME_ERROR_FAILED;
}

GAME_ERROR Deserialize(const void *data, size_t size)
{
  if (!CLIENT)
    return GAME_ERROR_FAILED;

  if (data == NULL)
    return GAME_ERROR_INVALID_PARAMETERS;

  bool result = CLIENT->retro_unserialize(data, size);

  return result ? GAME_ERROR_NO_ERROR : GAME_ERROR_FAILED;
}

GAME_ERROR CheatReset(void)
{
  if (!CLIENT)
    return GAME_ERROR_FAILED;

  CLIENT->retro_cheat_reset();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CheatSet(unsigned index, bool enabled, const char *code)
{
  if (!CLIENT)
    return GAME_ERROR_FAILED;

  CLIENT->retro_cheat_set(index, enabled, code);

  return GAME_ERROR_NO_ERROR;
}

GAME_REGION GetRegion(void)
{
  if (!CLIENT)
    return GAME_REGION_NTSC;

  return static_cast<GAME_REGION>(CLIENT->retro_get_region());
}

void* GetMemoryData(GAME_MEMORY id)
{
  if (!CLIENT)
    return NULL;

  return CLIENT->retro_get_memory_data(id);
}

size_t GetMemorySize(GAME_MEMORY id)
{
  if (!CLIENT)
    return 0;

  return CLIENT->retro_get_memory_size(id);
}

GAME_ERROR DiskSetEjectState(GAME_EJECT_STATE ejected)
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->DiskSetEjectState(ejected);
}

GAME_EJECT_STATE DiskGetEjectState()
{
  if (!CLIENT_BRIDGE)
    return GAME_NOT_EJECTED;

  return CLIENT_BRIDGE->DiskGetEjectState();
}

unsigned DiskGetImageIndex()
{
  if (!CLIENT_BRIDGE)
    return 0;

  return CLIENT_BRIDGE->DiskGetImageIndex();
}

GAME_ERROR DiskSetImageIndex(unsigned index)
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->DiskSetImageIndex(index);
}

unsigned DiskGetNumImages()
{
  if (!CLIENT_BRIDGE)
    return 0;

  return CLIENT_BRIDGE->DiskGetNumImages();
}

GAME_ERROR DiskReplaceImageIndex(unsigned index, const char* url)
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  if (url == NULL)
    return GAME_ERROR_INVALID_PARAMETERS;

  return CLIENT_BRIDGE->DiskReplaceImageIndex(index, url);
}

GAME_ERROR DiskAddImageIndex()
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->DiskAddImageIndex();
}

GAME_ERROR HwContextReset()
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->HwContextReset();
}

GAME_ERROR HwContextDestroy()
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->HwContextDestroy();
}

GAME_ERROR AudioAvailable()
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->AudioAvailable();
}

GAME_ERROR AudioSetState(bool enabled)
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->AudioSetState(enabled);
}

GAME_ERROR FrameTimeNotify(game_usec_t usec)
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->FrameTimeNotify(usec);
}

GAME_ERROR CameraInitialized()
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->CameraInitialized();
}

GAME_ERROR CameraDeinitialized()
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->CameraDeinitialized();
}

GAME_ERROR CameraFrameRawBuffer(const uint32_t *buffer, unsigned width, unsigned height, size_t pitch)
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->CameraFrameRawBuffer(buffer, width, height, pitch);
}

GAME_ERROR CameraFrameOpenglTexture(unsigned texture_id, unsigned texture_target, const float *affine)
{
  if (!CLIENT_BRIDGE)
    return GAME_ERROR_FAILED;

  return CLIENT_BRIDGE->CameraFrameOpenglTexture(texture_id, texture_target, affine);
}

} // extern "C"
