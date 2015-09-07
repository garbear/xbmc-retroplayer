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

#include "GameDatabase.h"
#include "games/Game.h"
#include "games/GameDefinitions.h"
#include "utils/Variant.h"

#include <algorithm>

using namespace dbiplus;
using namespace GAME;

CGameDatabase::CGameDatabase(void)
{
  AddOneToOne(GAME_EDITION_PATH);
  AddOneToOne(GAME_EDITION_CRC);
  AddOneToMany(GAME_PUBLISHER);
  AddOneToMany(GAME_DEVELOPER);
  AddManyToMany(GAME_GENRES);
  AddManyToMany(GAME_PLATFORMS);
  AddOneToMany(GAME_PLAYER_COUNT);
  AddOneToMany(GAME_COOP);
  AddOneToMany(GAME_RELEASE_DATE);
  AddManyToMany(GAME_EDITION_COUNTRIES);
  AddManyToMany(GAME_SERIES);
  AddOneToMany(GAME_ADD_DATE);
}

IDocument* CGameDatabase::CreateDocument(void) const
{
  return new CGame;
}

GamePtr CGameDatabase::GetGameByPath(const std::string& strPath)
{
  return GamePtr(static_cast<CGame*>(GetDocument(GAME_EDITION_PATH, strPath))); // TODO
}

GamePtr CGameDatabase::GetGameByCrc(const uint32_t crc)
{
  return GamePtr(static_cast<CGame*>(GetDocument(GAME_EDITION_CRC, crc))); // TODO
}

GameVector CGameDatabase::GetGames(void)
{
  return DowncastArray(GetDocuments());
}

GameVector CGameDatabase::GetGamesByPublisher(const std::string& strPublisher)
{
  return DowncastArray(GetDocuments(GAME_PUBLISHER, strPublisher));
}

GameVector CGameDatabase::GetGamesByDeveloper(const std::string& strDeveloper)
{
  return DowncastArray(GetDocuments(GAME_DEVELOPER, strDeveloper));
}

GameVector CGameDatabase::GetGamesByGenre(const std::string& strGenre)
{
  return DowncastArray(GetDocuments(GAME_GENRES, strGenre));
}

GameVector CGameDatabase::GetGamesByPlatform(const std::string& strPlatform)
{
  return DowncastArray(GetDocuments(GAME_PLATFORMS, strPlatform));
}

GameVector CGameDatabase::GetGamesByPlayerCount(unsigned int playerCount)
{
  return DowncastArray(GetDocuments(GAME_PLAYER_COUNT, playerCount));
}

GameVector CGameDatabase::GetGamesByCoop(void)
{
  return DowncastArray(GetDocuments(GAME_COOP, true));
}

GameVector CGameDatabase::GetGamesByYear(unsigned int year)
{
  return DowncastArray(GetDocuments(GAME_RELEASE_DATE, year)); // TODO
}

GameVector CGameDatabase::GetGamesByRegion(CountryCode region)
{
  return DowncastArray(GetDocuments(GAME_EDITION_COUNTRIES, static_cast<int>(region))); // TODO
}

GameVector CGameDatabase::GetGamesBySeries(const std::string& strSeries)
{
  return DowncastArray(GetDocuments(GAME_SERIES, strSeries));
}

GameVector CGameDatabase::GetGamesRecentlyAdded(void)
{
  return DowncastArray(GetDocuments(GAME_ADD_DATE, true)); // TODO
}

GameVector CGameDatabase::DowncastArray(const std::vector<IDocument*>& documents)
{
  GameVector games;

  std::transform(documents.begin(), documents.end(), games.begin(),
    [](IDocument* document)
    {
      return GamePtr(static_cast<CGame*>(document));
    });

  return games;
}
