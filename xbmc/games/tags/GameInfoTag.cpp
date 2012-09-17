/*
 *      Copyright (C) 2012 Garrett Brown
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "GameInfoTag.h"
#include "utils/Variant.h"

using namespace GAME_INFO;

CGameInfoTag::CGameInfoTag()
{
  Reset();
}

CGameInfoTag::CGameInfoTag(const CGameInfoTag &tag)
{
  *this = tag;
}

void CGameInfoTag::Reset()
{
  m_bLoaded = false;
  m_strURL.Empty();
  m_strPlatform.Empty();
  m_strTitle.Empty();
  m_strInternalTitle.Empty();
  m_strOriginalTitle.Empty();
  m_strAlternateTitle.Empty();
  m_strCode.Empty();
  m_strRegion.Empty();
  m_strLanguage.Empty();
  m_strPublisher.Empty();
  m_strDeveloper.Empty();
  m_strReviewer.Empty();
  m_strFormat.Empty();
  m_strCartridgeType.Empty();
  m_iDbId = 0;
  m_iYear = 0;
  m_strDescription.Empty();
  m_strVersion.Empty();
  m_strMedia.Empty();
  m_strPerspective.Empty();
  m_strControllerType.Empty();
  m_strRating.Empty();
  m_iPlayers = 0;
  m_bFavorite = false;
  m_iPlayTime = 0;
  m_genres.clear();
  m_collections.clear();
}


const CGameInfoTag &CGameInfoTag::operator=(const CGameInfoTag &tag)
{
  if (this != &tag)
  {
    m_bLoaded           = tag.m_bLoaded;
    m_strURL            = tag.m_strURL;
    m_strPlatform       = tag.m_strPlatform;
    m_strTitle          = tag.m_strTitle;
    m_strInternalTitle  = tag.m_strInternalTitle;
    m_strOriginalTitle  = tag.m_strOriginalTitle;
    m_strAlternateTitle = tag.m_strAlternateTitle;
    m_strCode           = tag.m_strCode;
    m_strRegion         = tag.m_strRegion;
    m_strLanguage       = tag.m_strLanguage;
    m_strPublisher      = tag.m_strPublisher;
    m_strDeveloper      = tag.m_strDeveloper;
    m_strReviewer       = tag.m_strReviewer;
    m_strFormat         = tag.m_strFormat;
    m_strCartridgeType  = tag.m_strCartridgeType;
    m_iDbId             = tag.m_iDbId;
    m_iYear             = tag.m_iYear;
    m_strDescription    = tag.m_strDescription;
    m_strVersion        = tag.m_strVersion;
    m_strMedia          = tag.m_strMedia;
    m_strPerspective    = tag.m_strPerspective;
    m_strControllerType = tag.m_strControllerType;
    m_strRating         = tag.m_strRating;
    m_bFavorite         = tag.m_bFavorite;
    m_iPlayers          = tag.m_iPlayers;
    m_iPlayTime         = tag.m_iPlayTime;
    m_genres            = tag.m_genres;
    m_collections       = tag.m_collections;
  }
  return *this;
}

bool CGameInfoTag::operator!=(const CGameInfoTag &tag) const
{
  if (this != &tag)
  {
    // Two tags can't be equal if they aren't loaded
    if (!m_bLoaded || !tag.m_bLoaded) return true;
    if (m_strURL != tag.m_strURL) return true;
    if (m_strPlatform != tag.m_strPlatform) return true;
    if (m_strTitle != tag.m_strTitle) return true;
    if (m_strInternalTitle != tag.m_strInternalTitle) return true;
    if (m_strOriginalTitle != tag.m_strOriginalTitle) return true;
    if (m_strAlternateTitle != tag.m_strAlternateTitle) return true;
    if (m_strCode != tag.m_strCode) return true;
    if (m_strRegion != tag.m_strRegion) return true;
    if (m_strLanguage != tag.m_strLanguage) return true;
    if (m_strPublisher != tag.m_strPublisher) return true;
    if (m_strDeveloper != tag.m_strDeveloper) return true;
    if (m_strReviewer != tag.m_strReviewer) return true;
    if (m_strFormat != tag.m_strFormat) return true;
    if (m_strCartridgeType != tag.m_strCartridgeType) return true;
    if (m_iDbId != tag.m_iDbId) return true;
    if (m_iYear != tag.m_iYear) return true;
    if (m_strDescription != tag.m_strDescription) return true;
    if (m_strVersion != tag.m_strVersion) return true;
    if (m_strMedia != tag.m_strMedia) return true;
    if (m_strPerspective != tag.m_strPerspective) return true;
    if (m_strControllerType != tag.m_strControllerType) return true;
    if (m_strRating != tag.m_strRating) return true;
    if (m_iPlayers != tag.m_iPlayers) return true;
    if (m_bFavorite != tag.m_bFavorite) return true;
    if (m_iPlayTime != tag.m_iPlayTime) return true;
    for (unsigned int i = 0; i < m_genres.size(); i++)
      if (m_genres.at(i).compare(tag.m_genres.at(i)) != 0)
        return true;
    for (unsigned int i = 0; i < m_collections.size(); i++)
      if (m_collections.at(i).compare(tag.m_collections.at(i)) != 0)
        return true;
  }
  return false;
}

void CGameInfoTag::Archive(CArchive &ar)
{
  if (ar.IsStoring())
  {
    ar << m_bLoaded;
    ar << m_strURL;
    ar << m_strPlatform;
    ar << m_strTitle;
    ar << m_strInternalTitle;
    ar << m_strOriginalTitle;
    ar << m_strAlternateTitle;
    ar << m_strCode;
    ar << m_strRegion;
    ar << m_strLanguage;
    ar << m_strPublisher;
    ar << m_strDeveloper;
    ar << m_strReviewer;
    ar << m_strFormat;
    ar << m_strCartridgeType;
    ar << m_iDbId;
    ar << m_iYear;
    ar << m_strDescription;
    ar << m_strVersion;
    ar << m_strMedia;
    ar << m_strPerspective;
    ar << m_strControllerType;
    ar << m_strRating;
    ar << m_iPlayers;
    ar << m_bFavorite;
    ar << m_iPlayTime;
    ar << m_genres;
    ar << m_collections;
  }
  else
  {
    int i;
    ar >> m_bLoaded;
    ar >> m_strURL;
    ar >> m_strPlatform;
    ar >> m_strTitle;
    ar >> m_strInternalTitle;
    ar >> m_strOriginalTitle;
    ar >> m_strAlternateTitle;
    ar >> m_strCode;
    ar >> m_strRegion;
    ar >> m_strLanguage;
    ar >> m_strPublisher;
    ar >> m_strDeveloper;
    ar >> m_strReviewer;
    ar >> m_strFormat;
    ar >> m_strCartridgeType;
    ar >> i;
    m_iDbId = (long)i;
    ar >> m_iYear;
    ar >> m_strDescription;
    ar >> m_strVersion;
    ar >> m_strMedia;
    ar >> m_strPerspective;
    ar >> m_strControllerType;
    ar >> m_strRating;
    ar >> m_iPlayers;
    ar >> m_bFavorite;
    ar >> m_iPlayTime;
    ar >> m_genres;
    ar >> m_collections;
  }
}

void CGameInfoTag::Serialize(CVariant &value)
{
  value["loaded"]         = m_bLoaded;
  value["url"]            = m_strURL;
  value["platform"]       = m_strPlatform;
  value["title"]          = m_strTitle;
  value["internaltitle"]  = m_strInternalTitle;
  value["originaltitle"]  = m_strOriginalTitle;
  value["alternatetitle"] = m_strAlternateTitle;
  value["id"]             = m_strCode;
  value["region"]         = m_strRegion;
  value["language"]       = m_strLanguage;
  value["publisher"]      = m_strPublisher;
  value["developer"]      = m_strDeveloper;
  value["reviewer"]       = m_strReviewer;
  value["format"]         = m_strFormat;
  value["cartridgetype"]  = m_strCartridgeType;
  value["dbid"]           = m_iDbId;
  value["year"]           = m_iYear;
  value["description"]    = m_strDescription;
  value["version"]        = m_strVersion;
  value["media"]          = m_strMedia;
  value["perspective"]    = m_strPerspective;
  value["controllertype"] = m_strControllerType;
  value["rating"]         = m_strRating;
  value["players"]        = m_iPlayers;
  value["favorite"]       = m_bFavorite;
  value["playtime"]       = m_iPlayTime;
  value["genres"]         = m_genres;
  value["collections"]    = m_collections;
}

void CGameInfoTag::ToSortable(SortItem &sortable)
{
  // No database entries for games (...yet)
}
