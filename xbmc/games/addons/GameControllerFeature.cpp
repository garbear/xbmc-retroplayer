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

#include "GameControllerFeature.h"
#include "games/GameDefinitions.h"
#include "games/GameTranslator.h"
#include "utils/log.h"
#include "utils/XMLUtils.h"

#include <sstream>

using namespace GAME;

#ifndef SAFE_DELETE
#define SAFE_DELETE(x)  do { delete (x); (x) = NULL; } while (0)
#endif

void CGameControllerFeature::Reset(void)
{
  m_type = FEATURE_UNKNOWN;
  m_strName.clear();
  m_label = 0;
  SAFE_DELETE(m_geometry);
  m_buttonType = BUTTON_UNKNOWN;
}

CGameControllerFeature& CGameControllerFeature::operator=(const CGameControllerFeature& rhs)
{
  if (this != &rhs)
  {
    Reset();

    m_type       = rhs.m_type;
    m_strName    = rhs.m_strName;
    m_label      = rhs.m_label;
    m_geometry   = rhs.m_geometry ? rhs.m_geometry->Clone() : NULL;
    m_buttonType = rhs.m_buttonType;
  }
  return *this;
}

bool CGameControllerFeature::Deserialize(const TiXmlElement* pElement)
{
  Reset();

  if (!pElement)
    return false;

  std::string strType(pElement->Value());

  // Type
  m_type = CGameTranslator::TranslateFeatureType(strType);
  if (m_type == FEATURE_UNKNOWN)
  {
    CLog::Log(LOGERROR, "Invalid feature: <%s> ", pElement->Value());
    return false;
  }

  // Name
  m_strName = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_FEATURE_NAME);
  if (m_strName.empty())
  {
    CLog::Log(LOGERROR, "<%s> tag has no \"%s\" attribute", strType.c_str(), LAYOUT_XML_ATTR_FEATURE_NAME);
    return false;
  }

  // Label
  std::string strLabel = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_FEATURE_LABEL);
  if (m_type != FEATURE_KEY && strLabel.empty())
  {
    CLog::Log(LOGERROR, "<%s> tag has no \"%s\" attribute", strType.c_str(), LAYOUT_XML_ATTR_FEATURE_LABEL);
    return false;
  }
  std::istringstream(strLabel) >> m_label;

  // Geometry
  m_geometry = CreateGeometry(pElement);

  // Button type
  if (m_type == FEATURE_BUTTON)
  {
    std::string strButtonType = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_BUTTON_TYPE);
    if (strButtonType.empty())
    {
      CLog::Log(LOGERROR, "<%s> tag has no \"%s\" attribute", strType.c_str(), LAYOUT_XML_ATTR_BUTTON_TYPE);
      return false;
    }
    else
    {
      m_buttonType = CGameTranslator::TranslateButtonType(strButtonType);
      if (m_buttonType == BUTTON_UNKNOWN)
      {
        CLog::Log(LOGERROR, "<%s> tag - attribute \"%s\" is invalid: \"%s\"",
                  strType.c_str(), LAYOUT_XML_ATTR_BUTTON_TYPE, strButtonType.c_str());
        return false;
      }
    }
  }

  return true;
}

FeatureGeometry CGameControllerFeature::CreateGeometry(const TiXmlElement* pElement)
{
  FeatureGeometry geometry = NULL;

  std::string strGeometry;
  if (pElement && XMLUtils::GetString(pElement, LAYOUT_XML_ATTR_FEATURE_GEOMETRY, strGeometry))
  {
    switch (CGameTranslator::TranslateGeometry(strGeometry))
    {
      case GEOMETRY_RECTANGLE:
      {
        int x1, y1, x2, y2;

        // x1
        std::string strX1 = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_GEOMETRY_X1);
        if (strX1.empty())
        {
          CLog::Log(LOGERROR, "Geometry of type \"%s\" has no \"%s\" attribute",
                    strGeometry.c_str(), LAYOUT_XML_ATTR_GEOMETRY_X1);
          break;
        }
        std::istringstream(strX1) >> x1;

        // y1
        std::string strY1 = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_GEOMETRY_Y1);
        if (strY1.empty())
        {
          CLog::Log(LOGERROR, "Geometry of type \"%s\" has no \"%s\" attribute",
                    strGeometry.c_str(), LAYOUT_XML_ATTR_GEOMETRY_Y1);
          break;
        }
        std::istringstream(strY1) >> y1;

        // x2
        std::string strX2 = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_GEOMETRY_X2);
        if (strX2.empty())
        {
          CLog::Log(LOGERROR, "Geometry of type \"%s\" has no \"%s\" attribute",
                    strGeometry.c_str(), LAYOUT_XML_ATTR_GEOMETRY_X2);
          break;
        }
        std::istringstream(strX2) >> x2;

        // y2
        std::string strY2 = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_GEOMETRY_Y2);
        if (strY2.empty())
        {
          CLog::Log(LOGERROR, "Geometry of type \"%s\" has no \"%s\" attribute",
                    strGeometry.c_str(), LAYOUT_XML_ATTR_GEOMETRY_Y2);
          break;
        }
        std::istringstream(strY2) >> y2;

        geometry = new CRect(x1, y1, x2, y2);
        break;
      }
      case GEOMETRY_CIRCLE:
      {
        int x, y, r;

        // x
        std::string strX = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_GEOMETRY_X);
        if (strX.empty())
        {
          CLog::Log(LOGERROR, "Geometry of type \"%s\" has no \"%s\" attribute",
                    strGeometry.c_str(), LAYOUT_XML_ATTR_GEOMETRY_X);
          break;
        }
        std::istringstream(strX) >> x;

        // y
        std::string strY = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_GEOMETRY_Y);
        if (strY.empty())
        {
          CLog::Log(LOGERROR, "Geometry of type \"%s\" has no \"%s\" attribute",
                    strGeometry.c_str(), LAYOUT_XML_ATTR_GEOMETRY_Y);
          break;
        }
        std::istringstream(strY) >> y;

        // r
        std::string strR = XMLUtils::GetAttribute(pElement, LAYOUT_XML_ATTR_GEOMETRY_R);
        if (strR.empty())
        {
          CLog::Log(LOGERROR, "Geometry of type \"%s\" has no \"%s\" attribute",
                    strGeometry.c_str(), LAYOUT_XML_ATTR_GEOMETRY_R);
          break;
        }
        std::istringstream(strR) >> r;

        geometry = new CCircle(x, y, r);
        break;
      }
      default:
      {
        CLog::Log(LOGERROR, "<%s> tag - attribute \"%s\" is invalid: \"%s\"",
                  pElement->Value(), LAYOUT_XML_ATTR_FEATURE_GEOMETRY, strGeometry.c_str());
      }
    }
  }
  return geometry;
}
