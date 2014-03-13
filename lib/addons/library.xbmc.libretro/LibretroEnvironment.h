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
#pragma once

#include <map>
#include <set>
#include <string>

namespace ADDON { class CHelper_libXBMC_addon; }
class CHelper_libXBMC_game;

namespace LIBRETRO
{
  class CClientBridge;
  class CLibretroDLL;

  class CLibretroEnvironment
  {
  public:
    static void Initialize(ADDON::CHelper_libXBMC_addon* xbmc, CHelper_libXBMC_game* frontend, CLibretroDLL* client, CClientBridge* clientBridge);
    static void Deinitialize();
    static ADDON::CHelper_libXBMC_addon* GetXBMC() { return m_xbmc; }
    static CHelper_libXBMC_game* GetFrontend() { return m_frontend; }

    /*!
     * Relies on a value provided by an environment command. Allows the game
     * client to provide this value to the frontend directly without using an
     * unnecessary frontend callback.
     */
    static bool SupportsNoGame() { return m_bSupportsNoGame; }

    /*!
     * FPS info is used to calculate timing for toast messages and possibly
     * other things.
     */
    static void UpdateFramerate(double fps);

  private:
    static bool EnvironmentCallback(unsigned cmd, void *data);

    static ADDON::CHelper_libXBMC_addon* m_xbmc;
    static CHelper_libXBMC_game*         m_frontend;
    static CClientBridge*                m_clientBridge;

    static bool   m_bSupportsNoGame;
    static double m_fps;
    static bool   m_bFramerateKnown; // true if UpdateFramerate() has been called

    // Record the variables reported by libretro core
    static std::map<std::string, std::set<std::string> > m_variables;
    // Record the settings reported by XBMC
    static std::map<std::string, std::string> m_settings;
  };
} // namespace LIBRETRO
