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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "libretro.h"
#include "games/GameClient.h"
#include "utils/StdString.h"

namespace GAMES
{
  class CLibretroEnvironment
  {
  public:
    typedef void (*SetPixelFormat_t)      (retro_pixel_format format); // retro_pixel_format defined in libretro.h
    typedef void (*SetKeyboardCallback_t) (retro_keyboard_event_t callback); // retro_keyboard_event_t defined in libretro.h

    // Libretro interface
    static bool EnvironmentCallback(unsigned cmd, void *data);

    static void SetCallbacks(SetPixelFormat_t spf, SetKeyboardCallback_t skc, ADDON::GameClientPtr activeClient);
    static void ResetCallbacks();

    static bool Abort() { return m_bAbort; }

  private:
    static SetPixelFormat_t      fn_SetPixelFormat;
    static SetKeyboardCallback_t fn_SetKeyboardCallback;
    static ADDON::GameClientPtr  m_activeClient;
    static CStdString            m_systemDirectory;
    static bool                  m_bAbort;
  };
} // namespace GAMES
