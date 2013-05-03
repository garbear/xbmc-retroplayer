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
  m_strName.Empty();
  m_strCode.Empty();
  m_strRegion.Empty();
  m_strPublisher.Empty();
  m_strFormat.Empty();
  m_strCartridgeType.Empty();
}

const CGameInfoTag &CGameInfoTag::operator=(const CGameInfoTag &tag)
{
  if (this != &tag)
  {
    m_bLoaded          = tag.m_bLoaded;
    m_strURL           = tag.m_strURL;
    m_strPlatform      = tag.m_strPlatform;
    m_strName          = tag.m_strName;
    m_strCode          = tag.m_strCode;
    m_strRegion        = tag.m_strRegion;
    m_strPublisher     = tag.m_strPublisher;
    m_strFormat        = tag.m_strFormat;
    m_strCartridgeType = tag.m_strCartridgeType;
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
    if (m_strName != tag.m_strName) return true;
    if (m_strCode != tag.m_strCode) return true;
    if (m_strRegion != tag.m_strRegion) return true;
    if (m_strPublisher != tag.m_strPublisher) return true;
    if (m_strFormat != tag.m_strFormat) return true;
    if (m_strCartridgeType != tag.m_strCartridgeType) return true;
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
    ar << m_strName;
    ar << m_strCode;
    ar << m_strRegion;
    ar << m_strPublisher;
    ar << m_strFormat;
    ar << m_strCartridgeType;
  }
  else
  {
    ar >> m_bLoaded;
    ar >> m_strURL;
    ar >> m_strPlatform;
    ar >> m_strName;
    ar >> m_strCode;
    ar >> m_strRegion;
    ar >> m_strPublisher;
    ar >> m_strFormat;
    ar >> m_strCartridgeType;
  }
}

void CGameInfoTag::Serialize(CVariant &value) const
{
  value["loaded"]        = m_bLoaded;
  value["url"]           = m_strURL;
  value["platform"]      = m_strPlatform;
  value["name"]          = m_strName;
  value["id"]            = m_strCode;
  value["region"]        = m_strRegion;
  value["publisher"]     = m_strPublisher;
  value["format"]        = m_strFormat;
  value["cartridgetype"] = m_strCartridgeType;
}

void CGameInfoTag::ToSortable(SortItem &sortable)
{
  // No database entries for games (...yet)
}
