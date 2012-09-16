/*
 *      Copyright (C) 2012 Team XBMC
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "GameDbUrl.h"
#include "filesystem/GameDatabaseDirectory.h"
#include "utils/StringUtils.h"
#include "utils/Variant.h"

using namespace std;
using namespace XFILE;
using namespace XFILE::GAMEDATABASEDIRECTORY;

bool CGameDbUrl::parse()
{
  // the URL must start with gamedb://
  if (m_url.GetProtocol() != "gamedb" || m_url.GetFileName().empty())
    return false;

  CStdString path = m_url.Get();
  NODE_TYPE dirType = CGameDatabaseDirectory::GetDirectoryType(path);
  NODE_TYPE childType = CGameDatabaseDirectory::GetDirectoryChildType(path);

  switch (dirType)
  {
  case NODE_TYPE_PLATFORM:
  case NODE_TYPE_GAME:
  case NODE_TYPE_GENRE:
  case NODE_TYPE_COLLECTION:
  case NODE_TYPE_PUBLISHER:
  case NODE_TYPE_YEAR:
  /*
  case NODE_TYPE_RATING:
  case NODE_TYPE_MOST_PLAYED:
  case NODE_TYPE_RECENTLY_PLAYED:
  case NODE_TYPE_RECENTLY_ADDED:
  */
    m_type = "games";
    break;
  default:
    break;
  }

  switch (childType)
  {
  case NODE_TYPE_PLATFORM:
    m_type = "platforms";
    break;
  case NODE_TYPE_GAME:
    m_type = "games";
    break;
  case NODE_TYPE_GENRE:
    m_type = "genres";
    break;
  case NODE_TYPE_COLLECTION:
    m_type = "collections";
    break;
  case NODE_TYPE_PUBLISHER:
    m_type = "publishers";
    break;
  case NODE_TYPE_YEAR:
    m_type = "years";
    break;
  /*
  case NODE_TYPE_RATING:
    m_type = "ratings";
    break;
  case NODE_TYPE_MOST_PLAYED:
  case NODE_TYPE_RECENTLY_PLAYED:
  case NODE_TYPE_RECENTLY_ADDED:
     m_type = "games";
     break;
  */
  default:
    break;
  }

  if (m_type.empty())
    return false;

  // parse query params
  CQueryParams queryParams;
  if (!CGameDatabaseDirectory::GetQueryParams(path, queryParams))
    return false;

  // retrieve and parse all options
  AddOptions(m_url.GetOptions());

  // add options based on the QueryParams
  if (queryParams.GetGenreId() != -1)
    AddOption("genreid", (int)queryParams.GetGenreId());
  if (queryParams.GetCollectionId() != -1)
    AddOption("collectionid", (int)queryParams.GetCollectionId());
  if (queryParams.GetPlatformId() != -1)
    AddOption("platformid", (int)queryParams.GetPlatformId());
  if (queryParams.GetPublisherId() != -1)
    AddOption("publisherid", (int)queryParams.GetPublisherId());
  if (queryParams.GetYear() != -1)
    AddOption("year", (int)queryParams.GetYear());

  return true;
}
