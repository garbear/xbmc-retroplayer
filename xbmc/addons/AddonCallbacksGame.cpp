/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include "AddonCallbacksGame.h"
#include "games/GameClient.h"
#include "threads/SystemClock.h"
#include "utils/log.h"


//#include "Application.h"
//#include "settings/AdvancedSettings.h"
//#include "dialogs/GUIDialogKaiToast.h"

using namespace GAME;
using namespace XbmcThreads;

namespace ADDON
{

CAddonCallbacksGame::CAddonCallbacksGame(CAddon* addon)
{
  m_addon     = addon;
  m_callbacks = new CB_GameLib;

  /* write XBMC game specific add-on function addresses to callback table */
  m_callbacks->ShutdownFrontend               = ShutdownFrontend;
  m_callbacks->EnvironmentSetRotation         = EnvironmentSetRotation;
  m_callbacks->EnvironmentGetOverscan         = EnvironmentGetOverscan;
  m_callbacks->EnvironmentCanDupe             = EnvironmentCanDupe;
  m_callbacks->EnvironmentGetSystemDirectory  = EnvironmentGetSystemDirectory;
  m_callbacks->EnvironmentSetPixelFormat      = EnvironmentSetPixelFormat;
  m_callbacks->EnvironmentSetInputDescriptors = EnvironmentSetInputDescriptors;
  m_callbacks->EnvironmentGetVariable         = EnvironmentGetVariable;
  m_callbacks->EnvironmentSetVariables        = EnvironmentSetVariables;
  m_callbacks->EnvironmentGetVariableUpdate   = EnvironmentGetVariableUpdate;
  m_callbacks->EnvironmentGetLibretroPath     = EnvironmentGetLibretroPath;
  m_callbacks->EnvironmentGetContentDirectory = EnvironmentGetContentDirectory;
  m_callbacks->EnvironmentGetSaveDirectory    = EnvironmentGetSaveDirectory;
  m_callbacks->EnvironmentSetSystemAvInfo     = EnvironmentSetSystemAvInfo;
  m_callbacks->VideoRefresh                   = VideoRefresh;
  m_callbacks->AudioSample                    = AudioSample;
  m_callbacks->AudioSampleBatch               = AudioSampleBatch;
  m_callbacks->InputPoll                      = InputPoll;
  m_callbacks->InputState                     = InputState;
  m_callbacks->InputGetDeviceCapabilities     = InputGetDeviceCapabilities;
  m_callbacks->RumbleSetState                 = RumbleSetState;
  m_callbacks->PerfGetTimeUsec                = PerfGetTimeUsec;
  m_callbacks->PerfGetCounter                 = PerfGetCounter;
  m_callbacks->PerfGetCpuFeatures             = PerfGetCpuFeatures;
  m_callbacks->PerfLog                        = PerfLog;
  m_callbacks->PerfRegister                   = PerfRegister;
  m_callbacks->PerfStart                      = PerfStart;
  m_callbacks->PerfStop                       = PerfStop;
  m_callbacks->SensorSetState                 = SensorSetState;
  m_callbacks->SensorGetInput                 = SensorGetInput;
  m_callbacks->CameraSetInfo                  = CameraSetInfo;
  m_callbacks->CameraStart                    = CameraStart;
  m_callbacks->CameraStop                     = CameraStop;
  m_callbacks->LocationStart                  = LocationStart;
  m_callbacks->LocationStop                   = LocationStop;
  m_callbacks->LocationGetPosition            = LocationGetPosition;
  m_callbacks->LocationSetInterval            = LocationSetInterval;
  m_callbacks->LocationInitialized            = LocationInitialized;
  m_callbacks->LocationDeinitialized          = LocationDeinitialized;
  m_callbacks->FrameTimeSetReference          = FrameTimeSetReference;
  m_callbacks->HwSetInfo                      = HwSetInfo;
  m_callbacks->HwGetCurrentFramebuffer        = HwGetCurrentFramebuffer;
  m_callbacks->HwGetProcAddress               = HwGetProcAddress;
}

CAddonCallbacksGame::~CAddonCallbacksGame()
{
  /* delete the callback table */
  delete m_callbacks;
}

CGameClient *CAddonCallbacksGame::GetGameClient(void *addonData)
{
  CAddonCallbacks *addon = static_cast<CAddonCallbacks*>(addonData);
  if (!addon || !addon->GetHelperGame())
  {
    CLog::Log(LOGERROR, "GAME - %s - called with a null pointer", __FUNCTION__);
    return NULL;
  }

  return dynamic_cast<CGameClient*>(addon->GetHelperGame()->m_addon);
}

void CAddonCallbacksGame::ShutdownFrontend(void* addonData)
{
  // Stub
}

void CAddonCallbacksGame::EnvironmentSetRotation(void* addonData, GAME_ROTATION rotation)
{
  // Stub
}

bool CAddonCallbacksGame::EnvironmentGetOverscan(void* addonData)
{
  // TODO: Move to library.xbmc.libretro and remove callback
  return true;
}

bool CAddonCallbacksGame::EnvironmentCanDupe(void* addonData)
{
  // TODO: Move to library.xbmc.libretro and remove callback
  return true;
}

char* CAddonCallbacksGame::EnvironmentGetSystemDirectory(void* addonData)
{
  // Stub
  return NULL;
}

bool CAddonCallbacksGame::EnvironmentSetPixelFormat(void* addonData, GAME_PIXEL_FORMAT format)
{
  // Stub
  return false;
}

void CAddonCallbacksGame::EnvironmentSetInputDescriptors(void* addonData, const game_input_descriptor* descriptor, size_t count)
{
  // Stub
}

void CAddonCallbacksGame::EnvironmentGetVariable(void* addonData, game_variable* variable)
{
  // Stub
}

void CAddonCallbacksGame::EnvironmentSetVariables(void* addonData, const game_variable* variables, size_t count)
{
  // Stub
}

bool CAddonCallbacksGame::EnvironmentGetVariableUpdate(void* addonData)
{
  // Stub
  return false;
}

char* CAddonCallbacksGame::EnvironmentGetLibretroPath(void* addonData)
{
  // Stub
  return NULL;
}

char* CAddonCallbacksGame::EnvironmentGetContentDirectory(void* addonData)
{
  // Stub
  return NULL;
}

char* CAddonCallbacksGame::EnvironmentGetSaveDirectory(void* addonData)
{
  // Stub
  return NULL;
}

bool CAddonCallbacksGame::EnvironmentSetSystemAvInfo(void* addonData, const game_system_av_info* info)
{
  // Stub
  return false;
}

void CAddonCallbacksGame::VideoRefresh(void* addonData, const void *data, unsigned width, unsigned height, size_t pitch)
{
  // Stub
}

void CAddonCallbacksGame::AudioSample(void* addonData, int16_t left, int16_t right)
{
  // Stub
}

size_t CAddonCallbacksGame::AudioSampleBatch(void* addonData, const int16_t *data, size_t frames)
{
  // Stub
  return 0;
}

void CAddonCallbacksGame::InputPoll(void* addonData)
{
  // Stub
}

int16_t CAddonCallbacksGame::InputState(void* addonData, unsigned port, unsigned device, unsigned index, unsigned id)
{
  // Stub
  return 0;
}

uint64_t CAddonCallbacksGame::InputGetDeviceCapabilities(void* addonData)
{
  // Stub
  return 0;
}

bool CAddonCallbacksGame::RumbleSetState(void* addonData, unsigned port, GAME_RUMBLE_EFFECT effect, uint16_t strength)
{
  // Stub
  return false;
}

game_time_t CAddonCallbacksGame::PerfGetTimeUsec(void* addonData)
{
  // See performance.c in RetroArch
  return 0;
}

game_perf_tick_t CAddonCallbacksGame::PerfGetCounter(void* addonData)
{
  // Stub
}

uint64_t CAddonCallbacksGame::PerfGetCpuFeatures(void* addonData)
{
  // Stub
}

void CAddonCallbacksGame::PerfLog(void* addonData)
{
  // Stub
}

void CAddonCallbacksGame::PerfRegister(void* addonData, game_perf_counter *counter)
{
  // Stub
}

void CAddonCallbacksGame::PerfStart(void* addonData, game_perf_counter *counter)
{
  // Stub
}

void CAddonCallbacksGame::PerfStop(void* addonData, game_perf_counter *counter)
{
  // Stub
}

bool CAddonCallbacksGame::SensorSetState(void* addonData, unsigned port, GAME_SENSOR_ACTION action, unsigned rate)
{
  // Stub
  return false;
}

float CAddonCallbacksGame::SensorGetInput(void* addonData, unsigned port, unsigned id)
{
  // Stub
  return 0.0f;
}

void CAddonCallbacksGame::CameraSetInfo(void* addonData, game_camera_info *camera_info)
{
  // Stub
}

bool CAddonCallbacksGame::CameraStart(void* addonData)
{
  // Stub
  return false;
}

void CAddonCallbacksGame::CameraStop(void* addonData)
{
  // Stub
}

bool CAddonCallbacksGame::LocationStart(void* addonData)
{
  // Stub
  return false;
}

void CAddonCallbacksGame::LocationStop(void* addonData)
{
  // Stub
}

bool CAddonCallbacksGame::LocationGetPosition(void* addonData, double *lat, double *lon, double *horiz_accuracy, double *vert_accuracy)
{
  // Stub
  return false;
}

void CAddonCallbacksGame::LocationSetInterval(void* addonData, unsigned interval_ms, unsigned interval_distance)
{
  // Stub
}

void CAddonCallbacksGame::LocationInitialized(void* addonData)
{
  // Stub
}

void CAddonCallbacksGame::LocationDeinitialized(void* addonData)
{
  // Stub
}

void CAddonCallbacksGame::FrameTimeSetReference(void* addonData, game_usec_t usec)
{
  // Stub
}

void CAddonCallbacksGame::HwSetInfo(void* addonData, const game_hw_info *hw_info)
{
  // Stub
}

uintptr_t CAddonCallbacksGame::HwGetCurrentFramebuffer(void* addonData)
{
  // Stub
  return 0;
}

game_proc_address_t CAddonCallbacksGame::HwGetProcAddress(void* addonData, const char *sym)
{
  // Stub
  return NULL;
}

}; /* namespace ADDON */
