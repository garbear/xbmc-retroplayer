/*
 *      Copyright (C) 2012-2014 Team XBMC
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

#include "addons/include/xbmc_game_types.h"

#include <string.h>

namespace GAME
{

class CGameClient;

class CGameClientProperties
{
public:
  CGameClientProperties(const CGameClient* parent);

  // LibPath(), passed to game client DLL
  const char* GetLibraryPath();
  
  // "special://profile/addon_data/id/system", passed to game client DLL
  const char* GetSystemDirectory();

  //
  const char* GetContentDirectory();

  //
  const char* GetSaveDirectory();

private:
  const CGameClient* const m_parent;

  // Buffers to hold the strings
  std::string  m_strLibraryPath;
  std::string  m_strSystemDirectory;
  std::string  m_strContentDirectory;
  std::string  m_strSaveDirectory;
};

} // namespace GAME
