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
#include "guilib/GUIControl.h"
#include "guilib/LocalizeStrings.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/XBMCTinyXML.h"

#include <algorithm>
#include <cstring>
#include <cstdlib>

using namespace ADDON;
using namespace GAME;

#define LAYOUT_XML_ROOT              "layout"

#define LAYOUT_XML_ELM_BUTTON        "button"

#define LAYOUT_XML_ATTR_LABEL        "label"
#define LAYOUT_XML_ATTR_IMAGE        "image"
#define LAYOUT_XML_ATTR_WIDTH        "width"
#define LAYOUT_XML_ATTR_HEIGHT       "height"
#define LAYOUT_XML_ATTR_PROMPT       "prompt"
#define LAYOUT_XML_ATTR_GEOMOETRY    "geometry"
#define LAYOUT_XML_ATTR_X1           "x1"
#define LAYOUT_XML_ATTR_Y1           "y1"
#define LAYOUT_XML_ATTR_X2           "x2"
#define LAYOUT_XML_ATTR_Y2           "y2"
#define LAYOUT_XML_ATTR_X            "x"
#define LAYOUT_XML_ATTR_Y            "y"
#define LAYOUT_XML_ATTR_RADIUS       "radius"
#define LAYOUT_XML_ATTR_RADIUS_X     "rx"
#define LAYOUT_XML_ATTR_RADIUS_Y     "ry"

#define LAYOUT_GEOMETRY_RECTANGLE    "rectangle"
#define LAYOUT_GEOMETRY_CIRCLE       "circle"
#define LAYOUT_GEOMETRY_ELLIPSE      "ellipse"
#define LAYOUT_GEOMETRY_DPAD         "dpad"


const GamePeripheralPtr CGamePeripheral::EmptyPtr;

CGamePeripheral::CGamePeripheral(const ADDON::AddonPtr& addon, CGUIControl* control) :
    m_addon(addon),
    m_width(0),
    m_height(0)
{
  if (control)
    m_controlRegion = control->CalcRenderRegion();
}

bool CGamePeripheral::Load(void)
{
  if (!m_addon)
    return false;

  std::string strLayoutXmlPath = m_addon->LibPath();

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

  const char* label = pRootElement->Attribute(LAYOUT_XML_ATTR_LABEL);
  if (!label)
  {
    CLog::Log(LOGERROR, "<%s> tag has no \"%s\" attribute", LAYOUT_XML_ROOT, LAYOUT_XML_ATTR_LABEL);
    return false;
  }

  const char* image = pRootElement->Attribute(LAYOUT_XML_ATTR_IMAGE);
  if (!image)
  {
    CLog::Log(LOGERROR, "<%s> tag has no \"%s\" attribute", LAYOUT_XML_ROOT, LAYOUT_XML_ATTR_IMAGE);
    return false;
  }

  const char* width = pRootElement->Attribute(LAYOUT_XML_ATTR_WIDTH);
  if (!width)
  {
    CLog::Log(LOGERROR, "<%s> tag has no \"%s\" attribute", LAYOUT_XML_ROOT, LAYOUT_XML_ATTR_WIDTH);
    return false;
  }

  int iWidth = TranslateInt(width);
  if (iWidth <= 0)
  {
    CLog::Log(LOGERROR, "<%s> tag invalid \"%s\" attribute: %s", LAYOUT_XML_ROOT, LAYOUT_XML_ATTR_WIDTH, width);
    return false;
  }

  const char* height = pRootElement->Attribute(LAYOUT_XML_ATTR_HEIGHT);
  if (!height)
  {
    CLog::Log(LOGERROR, "<%s> tag has no \"%s\" attribute", LAYOUT_XML_ROOT, LAYOUT_XML_ATTR_HEIGHT);
    return false;
  }

  int iHeight = TranslateInt(height);
  if (iHeight <= 0)
  {
    CLog::Log(LOGERROR, "<%s> tag invalid \"%s\" attribute: %s", LAYOUT_XML_ROOT, LAYOUT_XML_ATTR_HEIGHT, height);
    return false;
  }

  const TiXmlElement* pChild = pRootElement->FirstChildElement(LAYOUT_XML_ELM_BUTTON);
  while (pChild)
  {
    const char* buttonLabel = pChild->Attribute(LAYOUT_XML_ATTR_LABEL);
    if (!buttonLabel)
    {
      CLog::Log(LOGERROR, "<%s> tag has no \"%s\" attribute", LAYOUT_XML_ELM_BUTTON, LAYOUT_XML_ATTR_LABEL);
      return false;
    }

    std::string strButtonLabel = TranslateLabel(buttonLabel);
    if (strButtonLabel.empty())
    {
      CLog::Log(LOGERROR, "<%s> tag has invalid label: \"%s\"", LAYOUT_XML_ELM_BUTTON, buttonLabel);
      return false;
    }

    const char* prompt = pChild->Attribute(LAYOUT_XML_ATTR_PROMPT);
    if (!prompt)
      prompt = "";

    const char* geometry = pChild->Attribute(LAYOUT_XML_ATTR_GEOMOETRY);
    if (!geometry)
    {
      CLog::Log(LOGDEBUG, "Button \"%s\" has no \"%s\" attribute", strButtonLabel.c_str(), LAYOUT_XML_ATTR_GEOMOETRY);
      return false;
    }

    CCircle focusArea;

    if (std::strcmp(geometry, LAYOUT_GEOMETRY_RECTANGLE) == 0)
    {
      const char* x1 = pChild->Attribute(LAYOUT_XML_ATTR_X1);
      if (!x1)
      {
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is missing attribute \"%s\"",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_RECTANGLE, LAYOUT_XML_ATTR_X1);
        return false;
      }

      const char* y1 = pChild->Attribute(LAYOUT_XML_ATTR_Y1);
      if (!y1)
      {
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is missing attribute \"%s\"",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_RECTANGLE, LAYOUT_XML_ATTR_Y1);
        return false;
      }

      const char* x2 = pChild->Attribute(LAYOUT_XML_ATTR_X2);
      if (!x2)
      {
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is missing attribute \"%s\"",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_RECTANGLE, LAYOUT_XML_ATTR_X2);
        return false;
      }

      const char* y2 = pChild->Attribute(LAYOUT_XML_ATTR_Y2);
      if (!y2)
      {
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is missing attribute \"%s\"",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_RECTANGLE, LAYOUT_XML_ATTR_Y2);
        return false;
      }

      CRect rectangle(TranslateInt(x1), TranslateInt(y1), TranslateInt(x2), TranslateInt(y2));
      focusArea = rectangle.Circumcircle();
      if (focusArea.IsEmpty())
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is invalid: x1=%d, y1=%d, x2=%d, y2=%d",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_RECTANGLE,
                  TranslateInt(x1), TranslateInt(y1), TranslateInt(x2), TranslateInt(y2));
    }
    else if (std::strcmp(geometry, LAYOUT_GEOMETRY_CIRCLE) == 0)
    {
      const char* x = pChild->Attribute(LAYOUT_XML_ATTR_X);
      if (!x)
      {
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is missing attribute \"%s\"",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_CIRCLE, LAYOUT_XML_ATTR_X);
        return false;
      }

      const char* y = pChild->Attribute(LAYOUT_XML_ATTR_Y);
      if (!y)
      {
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is missing attribute \"%s\"",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_CIRCLE, LAYOUT_XML_ATTR_Y);
        return false;
      }

      const char* r = pChild->Attribute(LAYOUT_XML_ATTR_RADIUS);
      if (!r)
      {
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is missing attribute \"%s\"",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_CIRCLE, LAYOUT_XML_ATTR_RADIUS);
        return false;
      }

      focusArea = CCircle(TranslateInt(x), TranslateInt(y), TranslateInt(r));
      if (focusArea.IsEmpty())
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is invalid: x=%d, y=%d, r=%d",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_CIRCLE,
                  TranslateInt(x), TranslateInt(y), TranslateInt(r));
    }
    else if (std::strcmp(geometry, LAYOUT_GEOMETRY_ELLIPSE) == 0)
    {
      const char* x = pChild->Attribute(LAYOUT_XML_ATTR_X);
      if (!x)
      {
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is missing attribute \"%s\"",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_ELLIPSE, LAYOUT_XML_ATTR_X);
        return false;
      }

      const char* y = pChild->Attribute(LAYOUT_XML_ATTR_Y);
      if (!y)
      {
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is missing attribute \"%s\"",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_ELLIPSE, LAYOUT_XML_ATTR_Y);
        return false;
      }

      const char* rx = pChild->Attribute(LAYOUT_XML_ATTR_RADIUS_X);
      if (!rx)
      {
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is missing attribute \"%s\"",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_ELLIPSE, LAYOUT_XML_ATTR_RADIUS_X);
        return false;
      }

      const char* ry = pChild->Attribute(LAYOUT_XML_ATTR_RADIUS_Y);
      if (!ry)
      {
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is missing attribute \"%s\"",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_ELLIPSE, LAYOUT_XML_ATTR_RADIUS_Y);
        return false;
      }

      focusArea = CCircle(TranslateInt(x), TranslateInt(y), TranslateInt(std::max(rx, ry)));
      if (focusArea.IsEmpty())
        CLog::Log(LOGERROR, "Button \"%s\" with geometry \"%s\" is invalid: x=%d, y=%d, rx=%d, ry=%d",
                  strButtonLabel.c_str(), LAYOUT_GEOMETRY_ELLIPSE,
                  TranslateInt(x), TranslateInt(y), TranslateInt(rx), TranslateInt(ry));
    }
    else if (std::strcmp(geometry, LAYOUT_GEOMETRY_DPAD) == 0)
    {
      // TODO
      CLog::Log(LOGDEBUG, "Button \"%s\" with geometry \"%s\" is not implemented yet",
               strButtonLabel.c_str(), LAYOUT_GEOMETRY_DPAD);
      pChild = pChild->NextSiblingElement(LAYOUT_XML_ELM_BUTTON);
      continue;
    }
    else
    {
      CLog::Log(LOGERROR, "Button \"%s\" has invalid %s attribute: \"%s\"",
                strButtonLabel.c_str(), LAYOUT_XML_ATTR_GEOMOETRY, geometry);
      return false;
    }

    if (focusArea.IsEmpty())
    {
      CLog::Log(LOGERROR, "Button \"%s\" has invalid focus area: x=%d, y=%d, r=%d",
                strButtonLabel.c_str(), (int)focusArea.x, (int)focusArea.y, (int)focusArea.r);
      return false;
    }

    Button button = { strButtonLabel, Scale(focusArea, iWidth, iHeight) };
    if (!button.strLabel.empty())
      m_buttons.push_back(button);

    pChild = pChild->NextSiblingElement(LAYOUT_XML_ELM_BUTTON);
  }

  m_strControllerLabel = TranslateLabel(label);
  m_strImagePath = URIUtils::AddFileToFolder(URIUtils::GetDirectory(m_addon->LibPath()), image);
  m_width = iWidth;
  m_height = iHeight;

  if (!IsValid())
  {
    LogInvalid();
    return false;
  }

  CLog::Log(LOGDEBUG, "Loaded layout \"%s\" for %s with %u buttons",
            m_strControllerLabel.c_str(), m_addon->ID().c_str(), (unsigned int)m_buttons.size());

  return true;
}

bool CGamePeripheral::IsValid(void) const
{
  return m_addon &&
         !m_controlRegion.IsEmpty() &&
         !m_strControllerLabel.empty() &&
         !m_strImagePath.empty() &&
         m_width > 0 &&
         m_height > 0 &&
         !m_buttons.empty();
}

void CGamePeripheral::LogInvalid(void) const
{
  if (!m_addon)
    CLog::Log(LOGERROR, "Invalid peripheral");
  if (m_controlRegion.IsEmpty())
    CLog::Log(LOGERROR, "%s: Invalid control region: x1=%d, y1=%d, x2=%d, y2=%d", m_addon->ID().c_str(),
              (int)m_controlRegion.x1, (int)m_controlRegion.y1,
              (int)m_controlRegion.x2, (int)m_controlRegion.y2);
  if (m_strControllerLabel.empty())
    CLog::Log(LOGERROR, "%s: Missing layout label", m_addon->ID().c_str());
  if (m_strImagePath.empty())
    CLog::Log(LOGERROR, "%s: Missing image path", m_addon->ID().c_str());
  if (m_width <= 0)
    CLog::Log(LOGERROR, "%s: Invalid width: %d", m_addon->ID().c_str(), m_width);
  if (m_height <= 0)
    CLog::Log(LOGERROR, "%s: Invalid height: %d", m_addon->ID().c_str(), m_height);
  if (m_buttons.empty())
    CLog::Log(LOGERROR, "%s: Missing valid buttons", m_addon->ID().c_str());
}

CCircle CGamePeripheral::Scale(const CCircle& focusArea, float layoutWidth, float layoutHeight) const
{
  const CPoint offset = m_controlRegion.P1();
  const float scaleX = m_controlRegion.Width() / layoutWidth;
  const float scaleY = m_controlRegion.Height() / layoutHeight;

  return CCircle(offset.x + focusArea.x * scaleX, offset.y + focusArea.y * scaleY, focusArea.r * scaleX * scaleY);
}

std::string CGamePeripheral::TranslateLabel(const std::string& strLabel)
{
  if (StringUtils::IsNaturalNumber(strLabel))
    return m_addon->GetString(std::strtol(strLabel.c_str(), NULL, 10));
  return strLabel;
}

int CGamePeripheral::TranslateInt(const std::string& strInt)
{
  const char *s = strInt.c_str();
  char *p = NULL;
  errno = 0;
  long n = std::strtol(s, &p, 10);
  if (!errno && s != p)
    return (int)n;
  return 0;
}
