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

#include "GamePeripheral.h"
#include "games/GameDefinitions.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/XBMCTinyXML.h"

using namespace GAME;

const GamePeripheralPtr CGamePeripheral::EmptyPtr;

/*
CCircle CGamePeripheral::Scale(const CCircle& focusArea, float layoutWidth, float layoutHeight) const
{
  const CPoint offset = m_controlRegion.P1();
  const float scaleX = m_controlRegion.Width() / layoutWidth;
  const float scaleY = m_controlRegion.Height() / layoutHeight;

  return CCircle(offset.x + focusArea.x * scaleX, offset.y + focusArea.y * scaleY, focusArea.r * scaleX * scaleY);
}
*/

CGamePeripheral::CGamePeripheral(const ADDON::AddonProps &addonprops)
  : CAddon(addonprops)
{
}

CGamePeripheral::CGamePeripheral(const cp_extension_t *ext)
  : CAddon(ext)
{
}

std::string CGamePeripheral::Label(void)
{
  if (m_layout.Label() > 0)
    return GetString(m_layout.Label());
  return "";
}

std::string CGamePeripheral::ImagePath(void) const
{
  if (!m_layout.Image().empty())
    return URIUtils::AddFileToFolder(URIUtils::GetDirectory(LibPath()), m_layout.Image());
  return "";
}

std::string CGamePeripheral::OverlayPath(void) const
{
  if (!m_layout.Overlay().empty())
    return URIUtils::AddFileToFolder(URIUtils::GetDirectory(LibPath()), m_layout.Overlay());
  return "";
}

bool CGamePeripheral::LoadLayout(void)
{
  std::string strLayoutXmlPath = LibPath();

  CXBMCTinyXML xmlDoc;
  if (!xmlDoc.LoadFile(strLayoutXmlPath))
  {
    CLog::Log(LOGDEBUG, "Unable to load %s: %s at line %d", strLayoutXmlPath.c_str(), xmlDoc.ErrorDesc(), xmlDoc.ErrorRow());
    return false;
  }

  TiXmlElement* pRootElement = xmlDoc.RootElement();
  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueStr() != LAYOUT_XML_ROOT)
  {
    CLog::Log(LOGERROR, "%s: Can't find root <%s> tag", strLayoutXmlPath.c_str(), LAYOUT_XML_ROOT);
    return false;
  }

  CLog::Log(LOGINFO, "Loading controller layout %s", strLayoutXmlPath.c_str());

  if (!m_layout.Deserialize(pRootElement))
    return false;

  return true;
}
