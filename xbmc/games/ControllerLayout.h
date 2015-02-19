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

#include "GameTypes.h"
#include "addons/Addon.h"
#include "guilib/Geometry.h"

#include <string>
#include <vector>

class CGUIControl;

namespace GAME
{

struct Button
{
  std::string strLabel;
  CCircle     focusArea;
};

class CControllerLayout
{
public:
  CControllerLayout(const ADDON::AddonPtr& peripheral, CGUIControl* control);
  ~CControllerLayout(void) {  }

  static ControllerLayoutPtr EmptyPtr;

  bool Load(void);

  ADDON::AddonPtr            Addon(void) const     { return m_peripheral; }
  const std::string&         Label(void) const     { return m_strControllerLabel; }
  const std::string&         ImagePath(void) const { return m_strImagePath; }
  const std::vector<Button>& Buttons(void) const   { return m_buttons; }

private:
  bool IsValid(void) const;
  void LogInvalid(void) const;

  CCircle Scale(const CCircle& focusArea, float layoutWidth, float layoutHeight) const;

  std::string TranslateLabel(const std::string& strLabel);
  static int TranslateInt(const std::string& strInt);

  ADDON::AddonPtr     m_peripheral;
  CRect               m_controlRegion;
  std::string         m_strControllerLabel;
  std::string         m_strImagePath;
  int                 m_width;
  int                 m_height;
  std::vector<Button> m_buttons;
};

}
