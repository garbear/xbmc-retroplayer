/*
 *      Copyright (C) 2014-2015 Team XBMC
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

#include "addons/AddonCallbacks.h"
#include "addons/include/kodi_game_types.h"

#include <stdio.h>

#ifdef _WIN32
  #include <windows.h>
  #define DLLEXPORT __declspec(dllexport)
#else
  #define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT CB_GameLib* GAME_register_me(AddonCB* frontend)
{
  CB_GameLib* cb = NULL;
  if (!frontend)
    fprintf(stderr, "ERROR: GAME_register_frontend is called with NULL handle!!!\n");
  else
  {
    cb = frontend->GameLib_RegisterMe(frontend->addonData);
    if (!cb)
      fprintf(stderr, "ERROR: GAME_register_frontend can't get callback table from frontend!!!\n");
  }
  return cb;
}

DLLEXPORT void GAME_unregister_me(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return;
  return frontend->GameLib_UnRegisterMe(frontend->addonData, cb);
}

DLLEXPORT void GAME_close_game(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->CloseGame(frontend->addonData);
}

DLLEXPORT void GAME_video_frame(AddonCB* frontend, CB_GameLib* cb, const uint8_t* data, unsigned int width, unsigned int height, GAME_RENDER_FORMAT format)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->VideoFrame(frontend->addonData, data, width, height, format);
}

DLLEXPORT void GAME_audio_frames(AddonCB* frontend, CB_GameLib* cb, const uint8_t* data, unsigned int frames, GAME_AUDIO_FORMAT format)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->AudioFrames(frontend->addonData, data, frames, format);
}

DLLEXPORT void GAME_hw_set_info(AddonCB* frontend, CB_GameLib* cb, game_hw_info* hw_info)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->HwSetInfo(frontend->addonData, hw_info);
}

DLLEXPORT uintptr_t GAME_hw_get_current_framebuffer(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return 0;
  return cb->HwGetCurrentFramebuffer(frontend->addonData);
}

DLLEXPORT game_proc_address_t GAME_hw_get_proc_address(AddonCB* frontend, CB_GameLib* cb, const char* sym)
{
  if (frontend == NULL || cb == NULL)
    return NULL;
  return cb->HwGetProcAddress(frontend->addonData, sym);
}

DLLEXPORT bool GAME_open_port(AddonCB* frontend, CB_GameLib* cb, unsigned int port)
{
  if (frontend == NULL || cb == NULL)
    return false;
  return cb->OpenPort(frontend->addonData, port);
}

DLLEXPORT void GAME_close_port(AddonCB* frontend, CB_GameLib* cb, unsigned int port)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->ClosePort(frontend->addonData, port);
}

DLLEXPORT void GAME_rumble_set_state(AddonCB* frontend, CB_GameLib* cb, unsigned int port, GAME_RUMBLE_EFFECT effect, float strength)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->RumbleSetState(frontend->addonData, port, effect, strength);
}

DLLEXPORT void GAME_set_camera_info(AddonCB* frontend, CB_GameLib* cb, unsigned int width, unsigned int height, GAME_CAMERA_BUFFER caps)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->SetCameraInfo(frontend->addonData, width, height, caps);
}

DLLEXPORT bool GAME_start_camera(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return false;
  return cb->StartCamera(frontend->addonData);
}

DLLEXPORT void GAME_stop_camera(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->StopCamera(frontend->addonData);
}

DLLEXPORT bool GAME_start_location(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return false;
  return cb->StartLocation(frontend->addonData);
}

DLLEXPORT void GAME_stop_location(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->StopLocation(frontend->addonData);
}

DLLEXPORT bool GAME_get_location(AddonCB* frontend, CB_GameLib* cb, double* lat, double* lon, double* horiz_accuracy, double* vert_accuracy)
{
  if (frontend == NULL || cb == NULL)
    return false;
  return cb->GetLocation(frontend->addonData, lat, lon, horiz_accuracy, vert_accuracy);
}

DLLEXPORT void GAME_set_location_interval(AddonCB* frontend, CB_GameLib* cb, unsigned interval_ms, unsigned interval_distance)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->SetLocationInterval(frontend->addonData, interval_ms, interval_distance);
}

#ifdef __cplusplus
}
#endif
