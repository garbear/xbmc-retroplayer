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
#pragma once

#include "games/tags/GameInfoTag.h"
#include "utils/ISerializable.h"
#include "utils/StdString.h"
#include "utils/Variant.h"
#include "XBDateTime.h"

#include <map>
#include <vector>

/*!
 * Class to store and read genre information from CGameDatabase.
 */
class CGameGenre
{
public:
  long idGenre;
  CStdString strGenre;
};

/*!
 * Class to store and read collection information from CGameDatabase.
 */
class CCollection
{
public:
  long idCollection;
  CStdString strCollection;
};

/*!
 * Class to store and read game information from CGameDatabase.
 */
class CGame: public ISerializable
{
public:
  CGame() { }
  CGame(const GAME_INFO::CGameInfoTag& tag) { m_tag = tag; }
  virtual ~CGame() { }
  void Clear() { m_tag.Reset(); }
  virtual void Serialize(CVariant& value) { m_tag.Serialize(value); }

  bool operator<(const CGame &game) const
  {
    if (m_tag.GetURL() < game.m_tag.GetURL()) return true;
    if (m_tag.GetURL() > game.m_tag.GetURL()) return false;
    //if (iTrack < song.iTrack) return true;
    return false;
  }

  /*!
   * Whether this game has art associated with it.
   */
  //bool HasArt() const { return !strThumb.empty(); } // also test fanart

  /*!
   * Whether the art from this song matches the art from another.
   */
  //bool ArtMatches(const CGame &game) const { return game.strThumb == strThumb; } // also test fanart

  GAME_INFO::CGameInfoTag m_tag;
};

/*!
 * A map of CGame objects, used for CGameDatabase.
 */
class CGameMap
{
public:
  CGameMap() { }

  std::map<CStdString, CGame>::const_iterator Begin() { return m_map.begin(); }
  std::map<CStdString, CGame>::const_iterator End() { return m_map.end(); }
  void Add(const CStdString &file, const CGame &song);
  CGame *Find(const CStdString &file);
  void Clear() { m_map.erase(m_map.begin(), m_map.end()); }
  int Size() { return (int)m_map.size(); }

private:
  std::map<CStdString, CGame> m_map;
};

/*!
 * A vector of CGame objects, used for CGameDatabase.
 */
typedef std::vector<CGame> VECGAMES;

/*!
 * A vector of CStdString objects, used for CGameDatabase. Named VECGAMEGENRES
 * because Song.h already defines a VECGENRES type
 */
typedef std::vector<CGameGenre> VECGAMEGENRES;
