/*
 *      Copyright (C) 2012-2013 Team XBMC
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
#include "utils/ISerializable.h"
#include "utils/ISortable.h"


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
    const CStdString& GetTitle() const { return m_strName; }
    void SetTitle(const CStdString& strName) { m_strName = strName; }

    // Game Code (ID)
    const CStdString& GetID() const { return m_strCode; }
    void SetID(const CStdString& strCode) { m_strCode = strCode; }

    // Region
    const CStdString& GetRegion() const { return m_strRegion; }
    void SetRegion(const CStdString& strRegion) { m_strRegion = strRegion; }

    // Publisher / Licensee
    const CStdString& GetPublisher() const { return m_strPublisher; }
    void SetPublisher(const CStdString& strPublisher) { m_strPublisher = strPublisher; }

    // Format (PAL/NTSC); this is empty iff Region is empty
    const CStdString& GetFormat() const { return m_strFormat; }
    void SetFormat(const CStdString& strFormat) { m_strFormat = strFormat; }

    // Cartridge Type, e.g. ROM+MBC5+RAM+BATT
    const CStdString& GetCartridgeType() const { return m_strCartridgeType; }
    void SetCartridgeType(const CStdString& strCartridgeType) { m_strCartridgeType = strCartridgeType; }

    virtual void Archive(CArchive &ar);
    virtual void Serialize(CVariant &value) const;
    virtual void ToSortable(SortItem &sortable);

  private:
    bool       m_bLoaded;
    CStdString m_strURL;
    CStdString m_strPlatform;
    CStdString m_strName;
    CStdString m_strCode;
    CStdString m_strRegion;
    CStdString m_strPublisher;
    CStdString m_strFormat;
    CStdString m_strCartridgeType;
  };
}
