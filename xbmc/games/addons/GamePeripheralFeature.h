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
#pragma once

#include "games/GameTypes.h"
#include "guilib/Geometry.h"

#include <string>

class TiXmlElement;

namespace GAME
{

typedef CShape* FeatureGeometry;

class CGamePeripheralFeature
{
public:
  CGamePeripheralFeature(void) : m_geometry(NULL) { Reset(); }
  CGamePeripheralFeature(const CGamePeripheralFeature& other) : m_geometry(NULL) { *this = other; }
  ~CGamePeripheralFeature(void) { Reset(); }

  void Reset(void);

  CGamePeripheralFeature& operator=(const CGamePeripheralFeature& rhs);

  FeatureType        Type(void) const       { return m_type; }
  const std::string& Name(void) const       { return m_strName; }
  unsigned int       Label(void) const      { return m_label; }
  FeatureGeometry    Geometry(void) const   { return m_geometry; }
  enum ButtonType    ButtonType(void) const { return m_buttonType; }

  bool Deserialize(const TiXmlElement* pElement);

private:
  static FeatureGeometry CreateGeometry(const TiXmlElement* pElement);

  FeatureType     m_type;
  std::string     m_strName;
  unsigned int    m_label;
  FeatureGeometry m_geometry;
  enum ButtonType m_buttonType;
};

}
