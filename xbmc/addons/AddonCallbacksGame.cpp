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

#include "AddonCallbacksGame.h"
#include "cores/RetroPlayer/RetroPlayer.h"
#include "games/addons/GameClient.h"
#include "utils/log.h"

#include <string>

using namespace GAME;

namespace ADDON
{

CAddonCallbacksGame::CAddonCallbacksGame(CAddon* addon)
{
  m_addon     = addon;
  m_callbacks = new CB_GameLib;

  /* write XBMC game specific add-on function addresses to callback table */
  m_callbacks->CloseGame                      = CloseGame;
  m_callbacks->OpenPort                       = OpenPort;
  m_callbacks->ClosePort                      = ClosePort;
  m_callbacks->EnvironmentSetRotation         = EnvironmentSetRotation;
  m_callbacks->EnvironmentGetOverscan         = EnvironmentGetOverscan;
  m_callbacks->EnvironmentCanDupe             = EnvironmentCanDupe;
  m_callbacks->EnvironmentSetSystemAvInfo     = EnvironmentSetSystemAvInfo;
  m_callbacks->VideoFrame                     = VideoFrame;
  m_callbacks->AudioFrames                    = AudioFrames;
  m_callbacks->RumbleSetState                 = RumbleSetState;
  m_callbacks->PerfGetTimeUsec                = PerfGetTimeUsec;
  m_callbacks->PerfGetCounter                 = PerfGetCounter;
  m_callbacks->PerfGetCpuFeatures             = PerfGetCpuFeatures;
  m_callbacks->PerfLog                        = PerfLog;
  m_callbacks->PerfRegister                   = PerfRegister;
  m_callbacks->PerfStart                      = PerfStart;
  m_callbacks->PerfStop                       = PerfStop;
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

CGameClient* CAddonCallbacksGame::GetGameClient(void* addonData, const char* strFunction)
{
  CAddonCallbacks* addon = static_cast<CAddonCallbacks*>(addonData);
  if (!addon || !addon->GetHelperGame())
  {
    CLog::Log(LOGERROR, "GAME - %s - called with a null pointer", strFunction);
    return NULL;
  }

  return dynamic_cast<CGameClient*>(addon->GetHelperGame()->m_addon);
}

CRetroPlayer* CAddonCallbacksGame::GetRetroPlayer(void* addonData, const char* strFunction)
{
  CGameClient* gameClient = GetGameClient(addonData, strFunction);
  if (!gameClient)
  {
    CLog::Log(LOGERROR, "GAME - %s - invalid handler data", strFunction);
    return NULL;
  }

  if (!gameClient->GetPlayer())
  {
    CLog::Log(LOGERROR, "GAME - %s - game client is not playing a game", strFunction);
    return NULL;
  }

  CRetroPlayer* retroPlayer = dynamic_cast<CRetroPlayer*>(gameClient->GetPlayer());
  if (!retroPlayer)
  {
    CLog::Log(LOGERROR, "GAME - %s - active player is not RetroPlayer!", strFunction);
    return NULL;
  }

  return retroPlayer;
}

void CAddonCallbacksGame::CloseGame(void* addonData)
{
  CRetroPlayer* retroPlayer = GetRetroPlayer(addonData, __FUNCTION__);
  if (!retroPlayer)
    return;

  retroPlayer->CloseFile();
}

bool CAddonCallbacksGame::OpenPort(void* addonData, unsigned int port)
{
  CGameClient* gameClient = GetGameClient(addonData, __FUNCTION__);
  if (!gameClient)
    return false;

  return gameClient->OpenPort(port);
}

void CAddonCallbacksGame::ClosePort(void* addonData, unsigned int port)
{
  CGameClient* gameClient = GetGameClient(addonData, __FUNCTION__);
  if (!gameClient)
    return;

  gameClient->ClosePort(port);
}

void CAddonCallbacksGame::EnvironmentSetRotation(void* addonData, GAME_ROTATION rotation)
{
  CRetroPlayer* retroPlayer = GetRetroPlayer(addonData, __FUNCTION__);
  if (!retroPlayer)
    return;

  //retroPlayer->SetRotation(rotation); // TODO
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

bool CAddonCallbacksGame::EnvironmentSetSystemAvInfo(void* addonData, const game_system_av_info* info)
{
  // TODO
  return false;
}

bool CAddonCallbacksGame::VideoFrame(void* addonData, GAME_RENDER_FORMAT format, unsigned int width, unsigned int height, const uint8_t* data)
{
  CRetroPlayer* retroPlayer = GetRetroPlayer(addonData, __FUNCTION__);
  if (!retroPlayer)
    return false;

  AVPixelFormat pixelFormat(AV_PIX_FMT_NONE);

  switch (format)
  {
    case GAME_RENDER_FMT_YUV420P:
      pixelFormat = AV_PIX_FMT_YUV420P;
      break;
    case GAME_RENDER_FMT_0RGB8888:
      pixelFormat = AV_PIX_FMT_0RGB32;
      break;
    case GAME_RENDER_FMT_RGB565:
      pixelFormat = AV_PIX_FMT_RGB565;
      break;
    case GAME_RENDER_FMT_0RGB1555:
      pixelFormat = AV_PIX_FMT_RGB555;
      break;
    case GAME_RENDER_FMT_NONE:
    default:
      break;
  }

  if (pixelFormat == AV_PIX_FMT_NONE || width == 0 || height == 0 || data == NULL)
    return false;

  return retroPlayer->VideoFrame(pixelFormat, width, height, data);
}

unsigned int CAddonCallbacksGame::AudioFrames(void* addonData, GAME_AUDIO_FORMAT format, unsigned int frames, const uint8_t* data)
{
  CRetroPlayer* retroPlayer = GetRetroPlayer(addonData, __FUNCTION__);
  if (!retroPlayer)
    return 0;

  AEDataFormat audioFormat(AE_FMT_INVALID);

  switch (format)
  {
    case GAME_AUDIO_FMT_S16NE:
      audioFormat = AE_FMT_S16NE;
      break;
    case GAME_RENDER_FMT_NONE:
    default:
      break;
  }

  if (audioFormat == AE_FMT_INVALID || frames == 0 || data == NULL)
    return 0;

  return retroPlayer->AudioFrames(audioFormat, frames, data);
}

bool CAddonCallbacksGame::RumbleSetState(void* addonData, unsigned port, GAME_RUMBLE_EFFECT effect, uint16_t strength)
{
  CRetroPlayer* retroPlayer = GetRetroPlayer(addonData, __FUNCTION__);
  if (!retroPlayer)
    return false;

  // TODO
  return false;
}

game_time_t CAddonCallbacksGame::PerfGetTimeUsec(void* addonData)
{
  // See performance.c in RetroArch
  return 0;
}

game_perf_tick_t CAddonCallbacksGame::PerfGetCounter(void* addonData)
{
  // TODO
  return 0;
}

uint64_t CAddonCallbacksGame::PerfGetCpuFeatures(void* addonData)
{
  // TODO
  return 0;
}

void CAddonCallbacksGame::PerfLog(void* addonData)
{
  // TODO
}

void CAddonCallbacksGame::PerfRegister(void* addonData, game_perf_counter *counter)
{
  // TODO
}

void CAddonCallbacksGame::PerfStart(void* addonData, game_perf_counter *counter)
{
  // TODO
}

void CAddonCallbacksGame::PerfStop(void* addonData, game_perf_counter *counter)
{
  // TODO
}

void CAddonCallbacksGame::CameraSetInfo(void* addonData, game_camera_info *camera_info)
{
  // TODO
}

bool CAddonCallbacksGame::CameraStart(void* addonData)
{
  // TODO
  return false;
}

void CAddonCallbacksGame::CameraStop(void* addonData)
{
  // TODO
}

bool CAddonCallbacksGame::LocationStart(void* addonData)
{
  // TODO
  return false;
}

void CAddonCallbacksGame::LocationStop(void* addonData)
{
  // TODO
}

bool CAddonCallbacksGame::LocationGetPosition(void* addonData, double *lat, double *lon, double *horiz_accuracy, double *vert_accuracy)
{
  // TODO
  return false;
}

void CAddonCallbacksGame::LocationSetInterval(void* addonData, unsigned interval_ms, unsigned interval_distance)
{
  // TODO
}

void CAddonCallbacksGame::LocationInitialized(void* addonData)
{
  // TODO
}

void CAddonCallbacksGame::LocationDeinitialized(void* addonData)
{
  // TODO
}

void CAddonCallbacksGame::FrameTimeSetReference(void* addonData, game_usec_t usec)
{
  // TODO
}

void CAddonCallbacksGame::HwSetInfo(void* addonData, const game_hw_info *hw_info)
{
  // TODO
}

uintptr_t CAddonCallbacksGame::HwGetCurrentFramebuffer(void* addonData)
{
  // TODO
  return 0;
}

game_proc_address_t CAddonCallbacksGame::HwGetProcAddress(void* addonData, const char *sym)
{
  // TODO
  return NULL;
}

}; /* namespace ADDON */
