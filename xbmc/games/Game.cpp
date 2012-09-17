/*
 *      Copyright (C) 2005-2012 Team XBMC
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

#include "Game.h"

using namespace std;
using namespace GAME_INFO;

void CGameMap::Add(const CStdString &file, const CGame &song)
{
  CStdString lower = file;
  lower.ToLower();
  m_map.insert(pair<CStdString, CGame>(lower, song));
}

CGame* CGameMap::Find(const CStdString &file)
{
  CStdString lower = file;
  lower.ToLower();
  map<CStdString, CGame>::iterator it = m_map.find(lower);
  if (it == m_map.end())
    return NULL;
  return &(*it).second;
}
