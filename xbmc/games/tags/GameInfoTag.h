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
#pragma once

#include "utils/Archive.h"
#include "utils/Fanart.h"
#include "utils/ISerializable.h"
#include "utils/ISortable.h"
#include "XBDateTime.h"

#include <vector>
#include <string>
//#include <stdint.h>

namespace GAME_INFO
{
  class CGameInfoTag : public IArchivable, public ISerializable, public ISortable
  {
  public:
    CGameInfoTag();
    CGameInfoTag(const CGameInfoTag &tag);
    const CGameInfoTag &operator=(const CGameInfoTag &tag);
    ~CGameInfoTag() { }
    void Reset();
    bool operator!=(const CGameInfoTag &tag) const;

    bool Loaded() const { return m_bLoaded; }
    void SetLoaded(bool bOnOff = true) { m_bLoaded = bOnOff; }

    // File path
    const CStdString& GetURL() const { return m_strURL; }
    void SetURL(const CStdString& strURL) { m_strURL = strURL; }

    // Platform
    const CStdString& GetPlatform() const { return m_strPlatform; }
    void SetPlatform(const CStdString& strPlatform) { m_strPlatform = strPlatform; }
    
    // Title
    const CStdString& GetTitle() const { return m_strTitle; }
    void SetTitle(const CStdString& strTitle) { m_strTitle = strTitle; }

    // Internal Title
    const CStdString& GetInternalTitle() const { return m_strInternalTitle; }
    void SetInternalTitle(const CStdString& strInternalTitle) { m_strInternalTitle = strInternalTitle; }

    // Original Title
    const CStdString& GetOriginalTitle() const { return m_strOriginalTitle; }
    void SetOriginalTitle(const CStdString& strOriginalTitle) { m_strOriginalTitle = strOriginalTitle; }

    // Alternate Title
    const CStdString& GetAlternateTitle() const { return m_strAlternateTitle; }
    void SetAlternateTitle(const CStdString& strAlternateTitle) { m_strAlternateTitle = strAlternateTitle; }

    // Game Code
    const CStdString& GetCode() const { return m_strCode; }
    void SetCode(const CStdString& strCode) { m_strCode = strCode; }

    // Region (Country or multiple e.g. USA & Canada)
    const CStdString& GetRegion() const { return m_strRegion; }
    void SetRegion(const CStdString& strRegion) { m_strRegion = strRegion; }
    
    // Publisher / Licensee
    const CStdString& GetPublisher() const { return m_strPublisher; }
    void SetPublisher(const CStdString& strPublisher) { m_strPublisher = strPublisher; }

    // Developer
    const CStdString& GetDeveloper() const { return m_strDeveloper; }
    void SetDeveloper(const CStdString& strDeveloper) { m_strDeveloper = strDeveloper; }

    // Reviewer
    const CStdString& GetReviewer() const { return m_strReviewer; }
    void SetReviewer(const CStdString& strReviewer) { m_strReviewer = strReviewer; }

    // Format (PAL/NTSC)
    const CStdString& GetFormat() const { return m_strFormat; }
    void SetFormat(const CStdString& strFormat) { m_strFormat = strFormat; }

    // Cartridge Type, e.g. ROM+MBC5+RAM+BATT
    const CStdString& GetCartridgeType() const { return m_strCartridgeType; }
    void SetCartridgeType(const CStdString& strCartridgeType) { m_strCartridgeType = strCartridgeType; }
    
    // Game Database ID
    long GetDatabaseID() { return m_iDbId; }
    void SetDatabaseID(long dbId) { m_iDbId = dbId; }

    // Year
    int GetYear() { return m_iYear; }
    void SetYear(int year) { m_iYear = year; }
    
    // Description
    const CStdString& GetDescription() const { return m_strDescription; }
    void SetDescription(const CStdString& strDescription) { m_strDescription = strDescription; }
    
    // Version
    const CStdString& GetVersion() const { return m_strVersion; }
    void SetVersion(const CStdString& strVersion) { m_strVersion = strVersion; }
    
    // Media
    const CStdString& GetMedia() const { return m_strMedia; }
    void SetMedia(const CStdString& strMedia) { m_strMedia = strMedia; }
    
    // Perspective
    const CStdString& GetPerspective() const { return m_strPerspective; }
    void SetPerspective(const CStdString& strPerspective) { m_strPerspective = strPerspective; }
    
    // Controller Type
    const CStdString& GetControllerType() const { return m_strControllerType; }
    void SetControllerType(const CStdString& strControllerType) { m_strControllerType = strControllerType; }
    
    // Rating
    const CStdString& GetRating() const { return m_strRating; }
    void SetRating(const CStdString& strRating) { m_strRating = strRating; }
    
    // Players
    int GetPlayers() { return m_iPlayers; }
    void SetPlayers(int players) { m_iPlayers = players; }
    
    // Favorite
    bool IsFavorite() { return m_bFavorite; }
    void SetFavorite(bool favorite) { m_bFavorite = favorite; }
    void SetFavorite(int favorite) { m_bFavorite = favorite != 0; }
    
    // Play Time (in seconds)
    int GetPlayTime() { return m_iPlayTime; }
    void SetPlayTime(int playTime) { m_iPlayTime = playTime; }
    
    virtual void Archive(CArchive &ar);
    virtual void Serialize(CVariant &value);
    virtual void ToSortable(SortItem &sortable);

  private:
    bool       m_bLoaded;
    CStdString m_strURL;
    CStdString m_strPlatform;
    CStdString m_strTitle;
    CStdString m_strInternalTitle;
    CStdString m_strOriginalTitle;
    CStdString m_strAlternateTitle;
    CStdString m_strCode;
    CStdString m_strRegion;
    CStdString m_strPublisher;
    CStdString m_strDeveloper;
    CStdString m_strReviewer;
    CStdString m_strFormat;
    CStdString m_strCartridgeType;
    long       m_iDbId;
    int        m_iYear;
    CStdString m_strDescription;
    CStdString m_strVersion;
    CStdString m_strMedia;
    CStdString m_strPerspective;
    CStdString m_strControllerType;
    CStdString m_strRating;
    int        m_iPlayers;
    bool       m_bFavorite;
    int        m_iPlayTime; // seconds
    CDateTime  m_lastPlayed;
    CDateTime  m_dateAdded;
    CFanart    m_fanart;

    std::vector<std::string> m_genres; // TODO
    std::vector<std::string> m_collections; // TODO
  };
}
