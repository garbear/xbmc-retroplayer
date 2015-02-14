/*
 *      Copyright (C) 2012-2014 Team XBMC
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

#include "utils/IArchivable.h"
#include "utils/ISerializable.h"
#include "utils/ISortable.h"

#include <string>

namespace GAME
{
  class CGameInfoTag : public IArchivable, public ISerializable, public ISortable
  {
  public:
    CGameInfoTag() { Reset(); }
    CGameInfoTag(const CGameInfoTag& tag) { *this = tag; }
    const CGameInfoTag& operator=(const CGameInfoTag& tag);
    ~CGameInfoTag() { }
    void Reset();

    bool operator==(const CGameInfoTag& tag) const;
    bool operator!=(const CGameInfoTag& tag) const { return !(*this == tag); }

    bool IsLoaded() const { return m_bLoaded; }
    void SetLoaded(bool bOnOff = true) { m_bLoaded = bOnOff; }

    // File path
    const std::string& GetURL() const { return m_strURL; }
    void SetURL(const std::string& strURL) { m_strURL = strURL; }

    // Title
    const std::string& GetTitle() const { return m_strName; }
    void SetTitle(const std::string& strName) { m_strName = strName; }

    // Platform
    const std::string& GetPlatform() const { return m_strPlatform; }
    void SetPlatform(const std::string& strPlatform) { m_strPlatform = strPlatform; }

    // Game Code (ID)
    const std::string& GetID() const { return m_strID; }
    void SetID(const std::string& strID) { m_strID = strID; }

    // Region
    const std::string& GetRegion() const { return m_strRegion; }
    void SetRegion(const std::string& strRegion) { m_strRegion = strRegion; }

    // Publisher / Licensee
    const std::string& GetPublisher() const { return m_strPublisher; }
    void SetPublisher(const std::string& strPublisher) { m_strPublisher = strPublisher; }

    // Format (PAL/NTSC)
    const std::string& GetFormat() const { return m_strFormat; }
    void SetFormat(const std::string& strFormat) { m_strFormat = strFormat; }

    // Cartridge Type, e.g. "ROM+MBC5+RAM+BATT" or "CD"
    const std::string& GetCartridgeType() const { return m_strCartridgeType; }
    void SetCartridgeType(const std::string& strCartridgeType) { m_strCartridgeType = strCartridgeType; }

    virtual void Archive(CArchive& ar);
    virtual void Serialize(CVariant& value) const;
    virtual void ToSortable(SortItem& sortable, Field field) const;

  private:
    bool        m_bLoaded;
    std::string m_strURL;
    std::string m_strName;
    std::string m_strPlatform;
    std::string m_strID;
    std::string m_strRegion;
    std::string m_strPublisher;
    std::string m_strFormat;
    std::string m_strCartridgeType;
  };
}
