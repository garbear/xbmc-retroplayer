/*
 *      Copyright (C) 2015 Team Kodi
 *      http://kodi.tv
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

#include "games/GameTypes.h"
#include "dbwrappers/nosql/DenormalizedDatabase.h"

#include <vector>

namespace GAME
{
  class CGameDatabase : public dbiplus::CDenormalizedDatabase
  {
  public:
    CGameDatabase(void);
    virtual ~CGameDatabase(void) { }

    virtual const char* GetBaseDBName(void) const = 0;

    GamePtr GetGameByPath(const std::string& strPath);
    GamePtr GetGameByCrc(const uint32_t crc);

    GameVector GetGames(void);
    GameVector GetGamesByPublisher(const std::string& strPublisher);
    GameVector GetGamesByDeveloper(const std::string& strDeveloper);
    GameVector GetGamesByGenre(const std::string& strGenre);
    GameVector GetGamesByPlatform(const std::string& strPlatform);
    //GameVector GetGamesByRating(const std::string& strRating); // TODO
    GameVector GetGamesByPlayerCount(unsigned int playerCount);
    GameVector GetGamesByCoop(void);
    GameVector GetGamesByYear(unsigned int year);
    GameVector GetGamesByRegion(CountryCode region);
    GameVector GetGamesBySeries(const std::string& strSeries);
    GameVector GetGamesRecentlyAdded(void);

  protected:
    virtual dbiplus::IDocument* CreateDocument(void) const;

  private:
    static GameVector DowncastArray(const std::vector<dbiplus::IDocument*>& documents);
  };
}
