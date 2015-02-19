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

#include "GameControllerLayout.h"
#include "games/GameDefinitions.h"
#include "utils/log.h"
#include "utils/XMLUtils.h"

#include <algorithm>
#include <sstream>

using namespace GAME;

// --- FeatureTypeEqual --------------------------------------------------------

struct FeatureTypeEqual
{
  FeatureTypeEqual(FeatureType type, ButtonType buttonType) : type(type), buttonType(buttonType) { }

  bool operator()(const CGameControllerFeature& feature) const
  {
    if (type == FEATURE_UNKNOWN)
      return true; // Match all feature types

    if (type == FEATURE_BUTTON && feature.Type() == FEATURE_BUTTON)
    {
      if (buttonType == BUTTON_UNKNOWN)
        return true; // Match all button types

      return buttonType == feature.ButtonType();
    }

    return type == feature.Type();
  }

  const FeatureType type;
  const ButtonType  buttonType;
};

// --- CGameControllerLayout ---------------------------------------------------

void CGameControllerLayout::Reset(void)
{
  m_label = 0;
  m_strImage.clear();
  m_strOverlay.clear();
  m_width = 0;
  m_height = 0;
  m_features.clear();
}

unsigned int CGameControllerLayout::FeatureCount(FeatureType type       /* = FEATURE_UNKNOWN */,
                                                 ButtonType  buttonType /* = BUTTON_UNKNOWN */) const
{
  return std::count_if(m_features.begin(), m_features.end(), FeatureTypeEqual(type, buttonType));
}

bool CGameControllerLayout::Deserialize(const TiXmlElement* pElement)
{
  Reset();

  if (!pElement)
    return false;

  // Label
  std::string strLabel = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_LAYOUT_LABEL);
  if (strLabel.empty())
  {
    CLog::Log(LOGERROR, "<%s> tag has no \"%s\" attribute", LAYOUT_XML_ROOT, LAYOUT_XML_ATTR_LAYOUT_LABEL);
    return false;
  }
  std::istringstream(strLabel) >> m_label;

  // Image
  m_strImage = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_LAYOUT_IMAGE);
  if (m_strImage.empty())
    CLog::Log(LOGDEBUG, "<%s> tag has no \"%s\" attribute", LAYOUT_XML_ROOT, LAYOUT_XML_ATTR_LAYOUT_IMAGE);

  // Overlay
  m_strOverlay = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_LAYOUT_OVERLAY);
  if (m_strOverlay.empty())
    CLog::Log(LOGDEBUG, "<%s> tag has no \"%s\" attribute", LAYOUT_XML_ROOT, LAYOUT_XML_ATTR_LAYOUT_OVERLAY);

  // Width
  std::string strWidth = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_LAYOUT_WIDTH);
  if (strWidth.empty())
    CLog::Log(LOGDEBUG, "<%s> tag has no \"%s\" attribute", LAYOUT_XML_ROOT, LAYOUT_XML_ATTR_LAYOUT_WIDTH);
  else
    std::istringstream(strWidth) >> m_width;

  // Height
  std::string strHeight = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_LAYOUT_HEIGHT);
  if (strHeight.empty())
    CLog::Log(LOGDEBUG, "<%s> tag has no \"%s\" attribute", LAYOUT_XML_ROOT, LAYOUT_XML_ATTR_LAYOUT_HEIGHT);
  else
    std::istringstream(strHeight) >> m_height;

  // Features
  const TiXmlElement* pChild = pElement->FirstChildElement();
  while (pChild != NULL)
  {
    CGameControllerFeature feature;

    if (!feature.Deserialize(pChild))
      return false;

    m_features.push_back(feature);

    pChild = pChild->NextSiblingElement();
  }

  return true;
}
