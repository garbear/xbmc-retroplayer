/*
 *      Copyright (C) 2015 Team XBMC
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

#include <memory>
#include <vector>

namespace GAME
{
  class CGame;
  typedef std::shared_ptr<CGame> GamePtr;
  typedef std::vector<GamePtr>   GameVector;

  class CGameClient;
  typedef std::shared_ptr<CGameClient> GameClientPtr;
  typedef std::vector<GameClientPtr>   GameClientVector;

  class CSaveState;
  typedef std::shared_ptr<CSaveState> SaveStatePtr;
  typedef std::vector<SaveStatePtr>   SaveStateVector;

  enum GameFlag
  {
    ROM_NO_FLAGS        = 0,
    ROM_ALTERNATE       = (1 << 0),
    ROM_VERIFIED_DUMP   = (1 << 1),
    ROM_BAD_DUMP        = (1 << 2),
    ROM_OVER_DUMP       = (1 << 3),
    ROM_TRAINED         = (1 << 4),
    ROM_HACK            = (1 << 5),
    ROM_OLD_TRANSLATION = (1 << 6),
    ROM_NEW_TRANSLATION = (1 << 7),
  };

  enum GameArtworkType
  {
    ARTWORK_UNKNOWN,
    ARTWORK_THUMBNAIL,
    ARTWORK_FANART,
    ARTWORK_BANNER,
    ARTWORK_CLEAR_LOGO,
    ARTWORK_SCREENSHOT,
    ARTWORK_BOX_ART_FRONT,
    ARTWORK_BOX_ART_REAR,
    ARTWORK_TAILER,
    ARTWORK_GAMEPLAY,
    ARTWORK_PROVIDER,
  };

  enum CountryCode
  {
    COUNTRY_UNKNOWN     = 0,
    COUNTRY_WORLD       = (1 << 0),
    COUNTRY_AUSTRALIA   = (1 << 1),
    COUNTRY_BRAZIL      = (1 << 2),
    COUNTRY_CANADIAN    = (1 << 3),
    COUNTRY_CHINA       = (1 << 4),
    COUNTRA_ENGLAND     = (1 << 5),
    //COUNTRY_EUROPE      = (1 << 6),
    COUNTRY_FRANCE      = (1 << 7),
    COUNTRY_FINLAND     = (1 << 8),
    COUNTRY_GERMANY     = (1 << 9),
    COUNTRY_GREECE      = (1 << 10),
    COUNTRY_HONG_KONG   = (1 << 11),
    COUNTRY_HOLLAND     = (1 << 12),
    COUNTRY_ITALY       = (1 << 13),
    COUNTRY_JAPAN       = (1 << 14),
    COUNTRY_KOREA       = (1 << 15),
    COUNTRY_NETHERLANDS = (1 << 16),
    COUNTRY_SPAIN       = (1 << 17),
    COUNTRY_SWEDEN      = (1 << 18),
    COUNTRY_USA         = (1 << 19),
  };

  enum EsbrRating
  {
    ESBR_UNKNOWN,
    ESBR_EARLY_CHILDHOOD,
    ESBR_EVERYONE,
    ESBR_10_PLUS,
    ESBR_TEEN,
    ESBR_MATURE,
    ESBR_ADULTS_ONLY,
  };
}
