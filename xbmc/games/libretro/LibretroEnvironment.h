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

#include "libretro_wrapped.h"
#include "games/GameClient.h"
#include "utils/StdString.h"

#include <map>

namespace GAMES
{
  class CLibretroEnvironment
  {
  public:
    typedef void (*SetPixelFormat_t)         (LIBRETRO::retro_pixel_format format); // retro_pixel_format defined in libretro.h
    typedef void (*SetKeyboardCallback_t)    (LIBRETRO::retro_keyboard_event_t callback); // retro_keyboard_event_t defined in libretro.h
    typedef void (*SetDiskControlCallback_t) (const LIBRETRO::retro_disk_control_callback *callback_struct); // retro_disk_control_callback defined in libretro.h
    typedef void (*SetRenderCallback_t)      (const LIBRETRO::retro_hw_render_callback *callback_struct); // retro_hw_render_callback defined in libretro.h

    // Libretro interface
    static bool EnvironmentCallback(unsigned cmd, void *data);

    static void SetCallbacks(SetPixelFormat_t spf, SetKeyboardCallback_t skc,
                             SetDiskControlCallback_t sdcc, SetRenderCallback_t src,
                             GameClientPtr activeClient);
    static void ResetCallbacks();

    static bool Abort() { return m_bAbort; }
    static bool ParseVariable(const LIBRETRO::retro_variable &var, CStdString &strDefault);

  private:
    static SetPixelFormat_t         fn_SetPixelFormat;
    static SetKeyboardCallback_t    fn_SetKeyboardCallback;
    static SetDiskControlCallback_t fn_SetDiskControlCallback;
    static SetRenderCallback_t      fn_SetRenderCallback;
    static GameClientPtr            m_activeClient;
    static CStdString               m_systemDirectory;
    static bool                     m_bAbort;
    static std::map<CStdString, CStdString> m_varMap;
  };
} // namespace GAMES
