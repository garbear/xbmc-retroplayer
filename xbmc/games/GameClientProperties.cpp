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

#include "GameClientProperties.h"
#include "GameClient.h"

using namespace GAME;

#define GAME_CLIENT_SAVE_DIRECTORY    "save"
#define GAME_CLIENT_SYSTEM_DIRECTORY  "system"

CGameClientProperties::CGameClientProperties(const CGameClient* parent)
  : m_parent(parent)
{
}

const char* CGameClientProperties::GetLibraryPath()
{
  if (m_strLibraryPath.empty())
    m_strLibraryPath = m_parent->LibPath();

  return m_strLibraryPath.c_str();
}

const char* CGameClientProperties::GetSystemDirectory()
{
  if (m_strSystemDirectory.empty())
    m_strSystemDirectory = URIUtils::AddFileToFolder(m_parent->Profile(), GAME_CLIENT_SYSTEM_DIRECTORY);

  return m_strSystemDirectory.c_str();
}

const char* CGameClientProperties::GetContentDirectory()
{
  if (m_strContentDirectory.empty())
    m_strContentDirectory = m_parent->LibPath();

  return m_strContentDirectory.c_str();
}

const char* CGameClientProperties::GetSaveDirectory()
{
  if (m_strSaveDirectory.empty())
    m_strSaveDirectory = URIUtils::AddFileToFolder(m_parent->Profile(), GAME_CLIENT_SAVE_DIRECTORY);

  return m_strSaveDirectory.c_str();
}
