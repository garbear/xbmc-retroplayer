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

#include "Game.h"
#include "GameDefinitions.h"
#include "threads/SingleLock.h"
#include "utils/Variant.h"

#include <algorithm>

using namespace GAME;

CGame::CGame(void) :
  m_id(-1),
  m_playerCount(0),
  m_bCoop(false),
  m_rating(0),
  m_esbr(ESBR_UNKNOWN)
{
}

void CGame::Serialize(CVariant& value) const
{
  CSingleLock lock(m_mutex);
  if (m_id >= 0)               value[GAME_ID] = m_id;
  if (!m_strTitle.empty())     value[GAME_TITLE] = m_strTitle;
  if (!m_publisher.empty())    value[GAME_PUBLISHER] = m_publisher;
  if (!m_developer.empty())    value[GAME_DEVELOPER] = m_developer;
  if (!m_strMediaType.empty()) value[GAME_MEDIA_TYPE] = m_strMediaType;
  if (m_playerCount != 0)      value[GAME_PLAYER_COUNT] = static_cast<int>(m_playerCount);
  if (m_bCoop)                 value[GAME_COOP] = m_bCoop;
  //if (m_releaseDate.IsValid()) value[GAME_RELEASE_DATE] = m_releaseDate; // TODO
  //if (m_addDate.IsValid())     value[GAME_ADD_DATE] = m_addDate; // TODO
  if (m_rating != 0)           value[GAME_RATING] = static_cast<int>(m_rating);
  if (m_esbr != ESBR_UNKNOWN)  value[GAME_ESBR] = static_cast<int>(m_esbr);
  for (const auto& edition : m_editions)
  {
    CVariant subValue;
    subValue[GAME_EDITION_FLAGS]     = edition.flags;
    subValue[GAME_EDITION_COUNTRIES] = edition.countries;
    subValue[GAME_EDITION_PATH]      = edition.path;
    subValue[GAME_EDITION_CRC]       = static_cast<int>(edition.crc);
    subValue[GAME_EDITION_PROVIDER]  = edition.provider;
    value[GAME_EDITIONS].push_back(subValue);
  }
  for (const auto& genre : m_genres)
    value[GAME_GENRES].push_back(genre);
  for (const auto& platform : m_platforms)
    value[GAME_PLATFORMS].push_back(platform);
  for (const auto& series : m_series)
    value[GAME_SERIES].push_back(series);
  for (const auto& artwork : m_artwork)
  {
    CVariant subValue;
    subValue[GAME_ARTWORK_TYPE] = static_cast<int>(artwork.type);
    subValue[GAME_ARTWORK_PATH] = artwork.path;
    value[GAME_ARTWORK].push_back(subValue);
  }
}

void CGame::Deserialize(const CVariant& value)
{
  CSingleLock lock(m_mutex);
  if (value[GAME_ID].isInteger())           m_id = value[GAME_ID].asInteger();
  if (value[GAME_TITLE].isString())         m_strTitle = value[GAME_TITLE].asString();
  if (value[GAME_PUBLISHER].isString())     m_publisher = value[GAME_PUBLISHER].asString();
  if (value[GAME_DEVELOPER].isString())     m_developer = value[GAME_DEVELOPER].asString();
  if (value[GAME_MEDIA_TYPE].isString())    m_strMediaType = value[GAME_MEDIA_TYPE].asString();
  if (value[GAME_PLAYER_COUNT].isInteger()) m_playerCount = static_cast<unsigned int>(value[GAME_PLAYER_COUNT].asInteger());
  if (value[GAME_COOP].isBoolean())         m_bCoop = value[GAME_COOP].asBoolean();
  //if (value[GAME_RELEASE_DATE].isInteger()) m_releaseDate = value[GAME_RELEASE_DATE].asInteger(); // TODO
  //if (value[GAME_ADD_DATE].isInteger())     m_addDate = value[GAME_ADD_DATE].asInteger(); // TODO
  if (value[GAME_RATING].isInteger())       m_rating = static_cast<unsigned int>(value[GAME_RATING].asInteger());
  if (value[GAME_ESBR].isInteger())         m_esbr = static_cast<EsbrRating>(value[GAME_ESBR].asInteger());
  if (value[GAME_RATING].isArray())
  {
    for (auto it = value[GAME_RATING].begin_array(); it != value[GAME_RATING].end_array(); ++it)
    {
      const CVariant& subvalue = *it;
      if (subvalue.isObject())
      {
        GameEdition edition =
          {
            ROM_NO_FLAGS,
            COUNTRY_UNKNOWN,
            "",
            0,
            "",
          };
        if (subvalue[GAME_EDITION_FLAGS].isInteger())     edition.flags = static_cast<GameFlag>(subvalue[GAME_EDITION_FLAGS].asInteger());
        if (subvalue[GAME_EDITION_COUNTRIES].isInteger()) edition.countries = static_cast<CountryCode>(subvalue[GAME_EDITION_COUNTRIES].asInteger());
        if (subvalue[GAME_EDITION_PATH].isString())       edition.path = subvalue[GAME_EDITION_PATH].asString();
        if (subvalue[GAME_EDITION_CRC].isInteger())       edition.crc = static_cast<uint32_t>(subvalue[GAME_EDITION_CRC].asInteger());
        if (subvalue[GAME_EDITION_PROVIDER].isString())   edition.provider = subvalue[GAME_EDITION_PROVIDER].asString();
        m_editions.push_back(edition);
      }
    }
  }
  if (value[GAME_GENRES].isArray())
  {
    for (auto it = value[GAME_GENRES].begin_array(); it != value[GAME_GENRES].end_array(); ++it)
      if (it->isString()) m_genres.push_back(it->asString());
  }
  if (value[GAME_PLATFORMS].isArray())
  {
    for (auto it = value[GAME_PLATFORMS].begin_array(); it != value[GAME_PLATFORMS].end_array(); ++it)
      if (it->isString()) m_platforms.push_back(it->asString());
  }
  if (value[GAME_SERIES].isArray())
  {
    for (auto it = value[GAME_SERIES].begin_array(); it != value[GAME_SERIES].end_array(); ++it)
      if (it->isString()) m_series.push_back(it->asString());
  }
  if (value[GAME_ARTWORK].isArray())
  {
    for (auto it = value[GAME_ARTWORK].begin_array(); it != value[GAME_ARTWORK].end_array(); ++it)
    {
      const CVariant& subvalue = *it;
      if (subvalue.isObject())
      {
        GameArtwork artwork =
          {
            ARTWORK_UNKNOWN,
            "",
          };
        if (subvalue[GAME_ARTWORK_TYPE].isInteger()) artwork.type = static_cast<GameArtworkType>(subvalue[GAME_ARTWORK_TYPE].asInteger());
        if (subvalue[GAME_ARTWORK_PATH].isString())  artwork.path = subvalue[GAME_EDITION_PATH].asString();
        m_artwork.push_back(artwork);
      }
    }
  }
}

int64_t CGame::ID(void) const
{
  CSingleLock lock(m_mutex);
  return m_id;
}

std::string CGame::Title(void) const
{
  CSingleLock lock(m_mutex);
  return m_strTitle;
}

std::string CGame::Publisher(void) const
{
  CSingleLock lock(m_mutex);
  return m_publisher;
}

std::string CGame::Developer(void) const
{
  CSingleLock lock(m_mutex);
  return m_developer;
}

std::string CGame::MediaType(void) const
{
  CSingleLock lock(m_mutex);
  return m_strMediaType;
}

unsigned int CGame::PlayerCount(void) const
{
  CSingleLock lock(m_mutex);
  return m_playerCount;
}

bool CGame::IsCoop(void) const
{
  CSingleLock lock(m_mutex);
  return m_bCoop;
}

CDateTime CGame::ReleaseDate(void) const
{
  CSingleLock lock(m_mutex);
  return m_releaseDate;
}

CDateTime CGame::AddDate(void) const
{
  CSingleLock lock(m_mutex);
  return m_addDate;
}

unsigned int CGame::Rating(void) const
{
  CSingleLock lock(m_mutex);
  return m_rating;
}

EsbrRating CGame::ESBR(void) const
{
  CSingleLock lock(m_mutex);
  return m_esbr;
}

void CGame::SetID(int64_t identifier)
{
  CSingleLock lock(m_mutex);
  m_id = identifier;
  SetChanged();
}

void CGame::SetTitle(const std::string& strTitle)
{
  CSingleLock lock(m_mutex);
  m_strTitle = strTitle;
  SetChanged();
}

void CGame::SetPublisher(const std::string& strPublisher)
{
  CSingleLock lock(m_mutex);
  m_publisher = strPublisher;
  SetChanged();
}

void CGame::SetDeveloper(const std::string& strDeveloper)
{
  CSingleLock lock(m_mutex);
  m_developer = strDeveloper;
  SetChanged();
}

void CGame::SetMediaType(const std::string& strMediaType)
{
  CSingleLock lock(m_mutex);
  m_strMediaType = strMediaType;
  SetChanged();
}

void CGame::SetPlayerCount(unsigned int playerCount)
{
  CSingleLock lock(m_mutex);
  m_playerCount = playerCount;
  SetChanged();
}

void CGame::SetCoop(bool bIsCoop)
{
  CSingleLock lock(m_mutex);
  m_bCoop = bIsCoop;
  SetChanged();
}

void CGame::SetReleaseDate(const CDateTime& date)
{
  CSingleLock lock(m_mutex);
  m_releaseDate = date;
  SetChanged();
}

void CGame::SetAddDate(const CDateTime& date)
{
  CSingleLock lock(m_mutex);
  m_addDate = date;
  SetChanged();
}

void CGame::SetRating(unsigned int rating)
{
  CSingleLock lock(m_mutex);
  m_rating = rating;
  SetChanged();
}

void CGame::SetESBR(EsbrRating rating)
{
  CSingleLock lock(m_mutex);
  m_esbr = rating;
  SetChanged();
}

std::vector<GameEdition> CGame::Editions(void) const
{
  CSingleLock lock(m_mutex);
  return m_editions;
}

std::vector<std::string> CGame::Genres(void) const
{
  CSingleLock lock(m_mutex);
  return m_genres;
}

std::vector<std::string> CGame::Platforms(void) const
{
  CSingleLock lock(m_mutex);
  return m_platforms;
}

std::vector<std::string> CGame::Series(void) const
{
  CSingleLock lock(m_mutex);
  return m_series;
}

std::vector<GameArtwork> CGame::Artwork(void) const
{
  CSingleLock lock(m_mutex);
  return m_artwork;
}

void CGame::GetEditions(std::vector<GameEdition>& editions) const
{
  CSingleLock lock(m_mutex);
  editions.insert(editions.end(), m_editions.begin(), m_editions.end());
}

void CGame::GetGenres(std::vector<std::string>& genres) const
{
  CSingleLock lock(m_mutex);
  genres.insert(genres.end(), m_genres.begin(), m_genres.end());
}

void CGame::GetPlatforms(std::vector<std::string>& platforms) const
{
  CSingleLock lock(m_mutex);
  platforms.insert(platforms.end(), m_platforms.begin(), m_platforms.end());
}

void CGame::GetSeries(std::vector<std::string>& series) const
{
  CSingleLock lock(m_mutex);
  series.insert(series.end(), m_series.begin(), m_series.end());
}

void CGame::GetArtwork(std::vector<GameArtwork>& artwork) const
{
  CSingleLock lock(m_mutex);
  artwork.insert(artwork.end(), m_artwork.begin(), m_artwork.end());
}

/* TODO
bool CGame::HasEdition(const GameEdition& edition) const
{
  CSingleLock lock(m_mutex);
  return std::find(m_editions.begin(), m_editions.end(), edition) != m_editions.end();
}
*/

bool CGame::HasGenre(const std::string& genre) const
{
  CSingleLock lock(m_mutex);
  return std::find(m_genres.begin(), m_genres.end(), genre) != m_genres.end();
}

bool CGame::HasPlatform(const std::string& platform) const
{
  CSingleLock lock(m_mutex);
  return std::find(m_platforms.begin(), m_platforms.end(), platform) != m_platforms.end();
}

bool CGame::HasSeries(const std::string& series) const
{
  CSingleLock lock(m_mutex);
  return std::find(m_series.begin(), m_series.end(), series) != m_series.end();
}

/* TODO
bool CGame::HasArtwork(GameArtwork& artwork) const
{
  CSingleLock lock(m_mutex);
  return std::find(m_artwork.begin(), m_artwork.end(), artwork) != m_artwork.end();
}
*/

void CGame::SetEditions(const std::vector<GameEdition>& editions)
{
  CSingleLock lock(m_mutex);
  m_editions = editions;
  SetChanged();
}

void CGame::SetGenres(const std::vector<std::string>& genres)
{
  CSingleLock lock(m_mutex);
  m_genres = genres;
  SetChanged();
}

void CGame::SetPlatforms(const std::vector<std::string>& platforms)
{
  CSingleLock lock(m_mutex);
  m_platforms = platforms;
  SetChanged();
}

void CGame::SetSeries(const std::vector<std::string>& series)
{
  CSingleLock lock(m_mutex);
  m_series = series;
  SetChanged();
}

void CGame::SetArtwork(const std::vector<GameArtwork>& artwork)
{
  CSingleLock lock(m_mutex);
  m_artwork = artwork;
  SetChanged();
}

void CGame::AddEdition(const GameEdition& edition)
{
  CSingleLock lock(m_mutex);
  m_editions.push_back(edition);
  SetChanged();
}

void CGame::AddGenre(const std::string& genre)
{
  CSingleLock lock(m_mutex);
  m_genres.push_back(genre);
  SetChanged();
}

void CGame::AddPlatform(const std::string& platform)
{
  CSingleLock lock(m_mutex);
  m_platforms.push_back(platform);
  SetChanged();
}

void CGame::AddSeries(const std::string& series)
{
  CSingleLock lock(m_mutex);
  m_series.push_back(series);
  SetChanged();
}

void CGame::AddArtwork(GameArtwork& artwork)
{
  CSingleLock lock(m_mutex);
  m_artwork.push_back(artwork);
  SetChanged();
}

void CGame::ClearEditions(void)
{
  CSingleLock lock(m_mutex);
  m_editions.clear();
  SetChanged();
}

void CGame::ClearGenres(void)
{
  CSingleLock lock(m_mutex);
  m_genres.clear();
  SetChanged();
}

void CGame::ClearPlatforms(void)
{
  CSingleLock lock(m_mutex);
  m_platforms.clear();
  SetChanged();
}

void CGame::ClearSeries(void)
{
  CSingleLock lock(m_mutex);
  m_series.clear();
  SetChanged();
}

void CGame::ClearArtwork(void)
{
  CSingleLock lock(m_mutex);
  m_artwork.clear();
  SetChanged();
}
