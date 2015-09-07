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

#include "GameTypes.h"
#include "dbwrappers/nosql/IDocument.h"
#include "threads/SingleLock.h"
#include "XBDateTime.h"

#include <stdint.h>
#include <string>
#include <vector>

namespace GAME
{
  struct GameEdition
  {
    GameFlag     flags;
    CountryCode  countries;
    std::string  path;
    uint32_t     crc; // CRC, or zero if unknown
    std::string  provider;
  };

  struct GameArtwork
  {
    GameArtworkType type;
    std::string     path;
  };

  class CGame : public dbiplus::IDocument
  {
  public:
    CGame(void);

    // Implementation of IDocument
    virtual void Serialize(CVariant& value) const override;
    virtual void Deserialize(const CVariant& value) override;

    /*
     * Single-valued properties
     */
    int64_t      ID(void) const;           // Unique identifier for this game
    std::string  Title(void) const;        // Game title
    std::string  Publisher(void) const;    // Game publisher
    std::string  Developer(void) const;    // Game developer
    std::string  MediaType(void) const;    // Physical media type, e.g. "ROM+MBC5+RAM+BATT" or "CD"
    unsigned int PlayerCount(void) const;  // Number of players supported by the game
    bool         IsCoop(void) const;       // True if game supports cooperative mode
    CDateTime    ReleaseDate(void) const;  // Release date to best known accuracy
    CDateTime    AddDate(void) const;      // Date the game was added to the database
    unsigned int Rating(void) const;       // Rating out of a possible 10
    EsbrRating   ESBR(void) const;         // ESBR rating

    void SetID(int64_t identifier);
    void SetTitle(const std::string& strTitle);
    void SetPublisher(const std::string& strPublisher);
    void SetDeveloper(const std::string& strDeveloper);
    void SetMediaType(const std::string& strMediaType);
    void SetPlayerCount(unsigned int playerCount);
    void SetCoop(bool bIsCoop);
    void SetReleaseDate(const CDateTime& date);
    void SetAddDate(const CDateTime& date);
    void SetRating(unsigned int rating);
    void SetESBR(EsbrRating rating);

    /*
     * Multi-valued properties
     */
    std::vector<GameEdition>  Editions(void) const;   // Alternate editions of the same game
    std::vector<std::string>  Genres(void) const;     // List of genres
    std::vector<std::string>  Platforms(void) const;  // List of platforms this game runs on
    std::vector<std::string>  Series(void) const;     // List of series this game belongs to
    std::vector<GameArtwork>  Artwork(void) const;    // Artwork related to this game

    void GetEditions(std::vector<GameEdition>& editions) const;
    void GetGenres(std::vector<std::string>& genres) const;
    void GetPlatforms(std::vector<std::string>& platforms) const;
    void GetSeries(std::vector<std::string>& series) const;
    void GetArtwork(std::vector<GameArtwork>& artwork) const;

    //bool HasEdition(const GameEdition& edition) const; // TODO
    bool HasGenre(const std::string& genre) const;
    bool HasPlatform(const std::string& platform) const;
    bool HasSeries(const std::string& series) const;
    //bool HasArtwork(GameArtwork& artwork) const; // TODO

    void SetEditions(const std::vector<GameEdition>& editions);
    void SetGenres(const std::vector<std::string>& genres);
    void SetPlatforms(const std::vector<std::string>& platforms);
    void SetSeries(const std::vector<std::string>& series);
    void SetArtwork(const std::vector<GameArtwork>& artwork);

    void AddEdition(const GameEdition& edition);
    void AddGenre(const std::string& genre);
    void AddPlatform(const std::string& platform);
    void AddSeries(const std::string& series);
    void AddArtwork(GameArtwork& artwork);

    void ClearEditions(void);
    void ClearGenres(void);
    void ClearPlatforms(void);
    void ClearSeries(void);
    void ClearArtwork(void);

  private:
    int64_t                   m_id;
    std::string               m_strTitle;
    std::string               m_publisher;
    std::string               m_developer;
    std::string               m_strMediaType;
    unsigned int              m_playerCount;
    bool                      m_bCoop;
    CDateTime                 m_releaseDate;
    CDateTime                 m_addDate;
    unsigned int              m_rating;
    EsbrRating                m_esbr;
    std::vector<GameEdition>  m_editions;
    std::vector<std::string>  m_genres;
    std::vector<std::string>  m_platforms;
    std::vector<std::string>  m_series;
    std::vector<GameArtwork>  m_artwork;
    CCriticalSection          m_mutex;
  };
}
