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

#include "GameTranslator.h"
#include "GameDefinitions.h"

using namespace GAME;

static const char* TranslateFeatureType(FeatureType type)
{
  switch (type)
  {
    case FEATURE_BUTTON:           return LAYOUT_XML_ELM_BUTTON;
    case FEATURE_ANALOG_STICK:     return LAYOUT_XML_ELM_ANALOG_STICK;
    case FEATURE_ACCELEROMETER:    return LAYOUT_XML_ELM_ACCELEROMETER;
    case FEATURE_KEY:              return LAYOUT_XML_ELM_KEY;
    case FEATURE_RELATIVE_POINTER: return LAYOUT_XML_ELM_RELATIVE_POINTER;
    case FEATURE_ABSOLUTE_POINTER: return LAYOUT_XML_ELM_ABSOLUTE_POINTER;
    default:
      break;
  }
  return "";
}

FeatureType CGameTranslator::TranslateFeatureType(const std::string& strType)
{
  if (strType == LAYOUT_XML_ELM_BUTTON)           return FEATURE_BUTTON;
  if (strType == LAYOUT_XML_ELM_ANALOG_STICK)     return FEATURE_ANALOG_STICK;
  if (strType == LAYOUT_XML_ELM_ACCELEROMETER)    return FEATURE_ACCELEROMETER;
  if (strType == LAYOUT_XML_ELM_KEY)              return FEATURE_KEY;
  if (strType == LAYOUT_XML_ELM_RELATIVE_POINTER) return FEATURE_RELATIVE_POINTER;
  if (strType == LAYOUT_XML_ELM_ABSOLUTE_POINTER) return FEATURE_ABSOLUTE_POINTER;

  return FEATURE_UNKNOWN;
}

static const char* TranslateButtonType(ButtonType type)
{
  switch (type)
  {
    case BUTTON_DIGITAL: return "digital";
    case BUTTON_ANALOG:  return "analog";
    default:
      break;
  }
  return "";
}

ButtonType CGameTranslator::TranslateButtonType(const std::string& strType)
{
  if (strType == "digital") return BUTTON_DIGITAL;
  if (strType == "analog")  return BUTTON_ANALOG;

  return BUTTON_UNKNOWN;
}

const char* CGameTranslator::TranslateGeometry(FeatureGeometryType geometry)
{
  switch (geometry)
  {
    case GEOMETRY_RECTANGLE: return "rectangle";
    case GEOMETRY_CIRCLE:    return "circle";
    default:
      break;
  }
  return "none";
}

FeatureGeometryType CGameTranslator::TranslateGeometry(const std::string& strGeometry)
{
  if (strGeometry == "rectangle") return GEOMETRY_RECTANGLE;
  if (strGeometry == "circle")    return GEOMETRY_CIRCLE;

  return GEOMETRY_NONE;
}
