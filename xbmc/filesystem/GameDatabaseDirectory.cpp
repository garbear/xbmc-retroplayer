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

#include "GameDatabaseDirectory.h"
#include "games/GameDatabase.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/TextureManager.h"
#include "settings/Settings.h"
#include "utils/Crc32.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "File.h"
#include "FileItem.h"

using namespace std;
using namespace XFILE;
using namespace GAMEDATABASEDIRECTORY;

bool CGameDatabaseDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items)
{
  auto_ptr<CDirectoryNode> pNode(CDirectoryNode::ParseURL(strPath));

  if (!pNode.get())
    return false;

  bool bResult = pNode->GetChilds(items);
  for (int i = 0; i < items.Size(); ++i)
  {
    CFileItemPtr item = items[i];
    if (item->m_bIsFolder && !item->HasIcon() && !item->HasThumbnail())
    {
      CStdString strImage = GetIcon(item->GetPath());
      if (!strImage.IsEmpty() && g_TextureManager.HasTexture(strImage))
        item->SetIconImage(strImage);
    }
  }
  items.SetLabel(pNode->GetLocalizedName());

  return bResult;
}

NODE_TYPE CGameDatabaseDirectory::GetDirectoryChildType(const CStdString& strPath)
{
  auto_ptr<CDirectoryNode> pNode(CDirectoryNode::ParseURL(strPath));

  if (!pNode.get())
    return NODE_TYPE_NONE;

  return pNode->GetChildType();
}

NODE_TYPE CGameDatabaseDirectory::GetDirectoryType(const CStdString& strPath)
{
  auto_ptr<CDirectoryNode> pNode(CDirectoryNode::ParseURL(strPath));

  if (!pNode.get())
    return NODE_TYPE_NONE;

  return pNode->GetType();
}

NODE_TYPE CGameDatabaseDirectory::GetDirectoryParentType(const CStdString& strPath)
{
  auto_ptr<CDirectoryNode> pNode(CDirectoryNode::ParseURL(strPath));

  if (!pNode.get())
    return NODE_TYPE_NONE;

  CDirectoryNode* pParentNode = pNode->GetParent();

  if (!pParentNode)
    return NODE_TYPE_NONE;

  return pParentNode->GetChildType();
}

bool CGameDatabaseDirectory::GetQueryParams(const CStdString& strPath, CQueryParams& params)
{
  auto_ptr<CDirectoryNode> pNode(CDirectoryNode::ParseURL(strPath));

  if (!pNode.get())
    return false;

  CDirectoryNode::GetDatabaseInfo(strPath, params);
  return true;
}

void CGameDatabaseDirectory::ClearDirectoryCache(const CStdString& strDirectory)
{
  CStdString path(strDirectory);
  URIUtils::RemoveSlashAtEnd(path);

  Crc32 crc;
  crc.ComputeFromLowerCase(path);

  CStdString strFileName;
  strFileName.Format("special://temp/%08x.fi", (unsigned __int32) crc);
  CFile::Delete(strFileName);
}

bool CGameDatabaseDirectory::IsAllItem(const CStdString& strDirectory)
{
  if (strDirectory.Right(4).Equals("/-1/"))
    return true;
  return false;
}

bool CGameDatabaseDirectory::GetLabel(const CStdString& strDirectory, CStdString& strLabel)
{
  strLabel = "";

  auto_ptr<CDirectoryNode> pNode(CDirectoryNode::ParseURL(strDirectory));
  if (!pNode.get() || strDirectory.IsEmpty())
    return false;

  // first see if there's any filter criteria
  CQueryParams params;
  CDirectoryNode::GetDatabaseInfo(strDirectory, params);

  CGameDatabase gameDatabase;
  if (!gameDatabase.Open())
    return false;

  // get genre
  if (params.GetGenreId() != -1)
    strLabel += gameDatabase.GetGenreById(params.GetGenreId());

  // get platform
  if (params.GetPlatformId() != -1)
  {
    if (!strLabel.IsEmpty())
      strLabel += " / ";
    strLabel += gameDatabase.GetPlatformById(params.GetPlatformId());
  }
  
  // get collection
  if (params.GetCollectionId() != -1)
  {
    if (!strLabel.IsEmpty())
      strLabel += " / ";
    strLabel += gameDatabase.GetCollectionById(params.GetCollectionId());
  }

  // get publisher
  if (params.GetPublisherId() != -1)
  {
    if (!strLabel.IsEmpty())
      strLabel += " / ";
    strLabel += gameDatabase.GetPublisherById(params.GetPublisherId());
  }

  // get year
  if (params.GetYear() != -1)
  {
    CStdString strTemp;
    strTemp.Format("%i", params.GetYear());
    if (!strLabel.IsEmpty())
      strLabel += " / ";
    strLabel += strTemp;
  }

  if (strLabel.IsEmpty())
  {
    /*
    switch (pNode->GetChildType())
    {
    case NODE_TYPE_TITLE_MOVIES:
    case NODE_TYPE_TITLE_TVSHOWS:
    case NODE_TYPE_TITLE_MUSICVIDEOS:
      strLabel = g_localizeStrings.Get(369); break;
    case NODE_TYPE_ACTOR: // Actor
      strLabel = g_localizeStrings.Get(344); break;
    case NODE_TYPE_GENRE: // Genres
      strLabel = g_localizeStrings.Get(135); break;
    case NODE_TYPE_COUNTRY: // Countries
      strLabel = g_localizeStrings.Get(20451); break;
    case NODE_TYPE_YEAR: // Year
      strLabel = g_localizeStrings.Get(562); break;
    case NODE_TYPE_DIRECTOR: // Director
      strLabel = g_localizeStrings.Get(20348); break;
    case NODE_TYPE_SETS: // Sets
      strLabel = g_localizeStrings.Get(20434); break;
    case NODE_TYPE_TAGS: // Tags
      strLabel = g_localizeStrings.Get(20459); break;
    case NODE_TYPE_MOVIES_OVERVIEW: // Movies
      strLabel = g_localizeStrings.Get(342); break;
    case NODE_TYPE_TVSHOWS_OVERVIEW: // TV Shows
      strLabel = g_localizeStrings.Get(20343); break;
    case NODE_TYPE_RECENTLY_ADDED_MOVIES: // Recently Added Movies
      strLabel = g_localizeStrings.Get(20386); break;
    case NODE_TYPE_RECENTLY_ADDED_EPISODES: // Recently Added Episodes
      strLabel = g_localizeStrings.Get(20387); break;
    case NODE_TYPE_STUDIO: // Studios
      strLabel = g_localizeStrings.Get(20388); break;
    case NODE_TYPE_MUSICVIDEOS_OVERVIEW: // Music Videos
      strLabel = g_localizeStrings.Get(20389); break;
    case NODE_TYPE_RECENTLY_ADDED_MUSICVIDEOS: // Recently Added Music Videos
      strLabel = g_localizeStrings.Get(20390); break;
    case NODE_TYPE_SEASONS: // Seasons
      strLabel = g_localizeStrings.Get(33054); break;
    case NODE_TYPE_EPISODES: // Episodes
      strLabel = g_localizeStrings.Get(20360); break;
    default:
      CLog::Log(LOGWARNING, "%s - Unknown nodetype requested %d", __FUNCTION__, pNode->GetChildType());
      return false;
    }
    */
    strLabel = "*** Games ***";
  }

  return true;
}

CStdString CGameDatabaseDirectory::GetIcon(const CStdString &strDirectory)
{
  /*
  switch (GetDirectoryChildType(strDirectory))
  {
  case NODE_TYPE_TITLE_MOVIES:
    if (strDirectory.Equals("videodb://1/2/"))
    {
      if (g_settings.m_bMyVideoNavFlatten)
        return "DefaultMovies.png";
      return "DefaultMovieTitle.png";
    }
    return "";
  case NODE_TYPE_TITLE_TVSHOWS:
    if (strDirectory.Equals("videodb://2/2/"))
    {
      if (g_settings.m_bMyVideoNavFlatten)
        return "DefaultTVShows.png";
      return "DefaultTVShowTitle.png";
    }
    return "";
  case NODE_TYPE_TITLE_MUSICVIDEOS:
    if (strDirectory.Equals("videodb://3/2/"))
    {
      if (g_settings.m_bMyVideoNavFlatten)
        return "DefaultMusicVideos.png";
      return "DefaultMusicVideoTitle.png";
    }
    return "";
  case NODE_TYPE_ACTOR: // Actor
    return "DefaultActor.png";
  case NODE_TYPE_GENRE: // Genres
    return "DefaultGenre.png";
  case NODE_TYPE_COUNTRY: // Countries
    return "DefaultCountry.png";
  case NODE_TYPE_SETS: // Sets
    return "DefaultSets.png";
  case NODE_TYPE_TAGS: // Tags
    return "DefaultTags.png";
  case NODE_TYPE_YEAR: // Year
    return "DefaultYear.png";
  case NODE_TYPE_DIRECTOR: // Director
    return "DefaultDirector.png";
  case NODE_TYPE_MOVIES_OVERVIEW: // Movies
    return "DefaultMovies.png";
  case NODE_TYPE_TVSHOWS_OVERVIEW: // TV Shows
    return "DefaultTVShows.png";
  case NODE_TYPE_RECENTLY_ADDED_MOVIES: // Recently Added Movies
    return "DefaultRecentlyAddedMovies.png";
  case NODE_TYPE_RECENTLY_ADDED_EPISODES: // Recently Added Episodes
    return "DefaultRecentlyAddedEpisodes.png";
  case NODE_TYPE_RECENTLY_ADDED_MUSICVIDEOS: // Recently Added Episodes
    return "DefaultRecentlyAddedMusicVideos.png";
  case NODE_TYPE_STUDIO: // Studios
    return "DefaultStudios.png";
  case NODE_TYPE_MUSICVIDEOS_OVERVIEW: // Music Videos
    return "DefaultMusicVideos.png";
  case NODE_TYPE_MUSICVIDEOS_ALBUM: // Music Videos - Albums
    return "DefaultMusicAlbums.png";
  default:
    CLog::Log(LOGWARNING, "%s - Unknown nodetype requested %s", __FUNCTION__, strDirectory.c_str());
    break;
  }
  */
  return "";
}

bool CGameDatabaseDirectory::Exists(const char* strPath)
{
  auto_ptr<CDirectoryNode> pNode(CDirectoryNode::ParseURL(strPath));

  if (!pNode.get())
    return false;

  if (pNode->GetChildType() == GAMEDATABASEDIRECTORY::NODE_TYPE_NONE)
    return false;

  return true;
}

bool CGameDatabaseDirectory::CanCache(const CStdString& strPath)
{
  auto_ptr<CDirectoryNode> pNode(CDirectoryNode::ParseURL(strPath));
  if (!pNode.get())
    return false;
  return pNode->CanCache();
}
