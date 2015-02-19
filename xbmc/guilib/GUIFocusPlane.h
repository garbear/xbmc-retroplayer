/*
 *      Copyright (C) 2014-2015 Team XBMC
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

#include "Geometry.h"
#include "GUIControlGroup.h"

class IFocusRenderer
{
public:
  virtual ~IFocusRenderer(void) { }

  virtual bool Load(void) = 0;
  virtual void Unload(void) = 0;
  virtual void RenderStart(void) = 0;
  virtual void RenderEnd(void) = 0;
};

class CGUIFocusPlane : public CGUIControlGroup
{
public:
  CGUIFocusPlane(void);
  CGUIFocusPlane(int parentID, int controlID, float posX, float posY, float width, float height);
  CGUIFocusPlane(const CGUIFocusPlane &from);
  virtual ~CGUIFocusPlane(void);
  void Initialize(void);
  virtual CGUIFocusPlane *Clone() const { return new CGUIFocusPlane(*this); }

  bool           IsFocused(void) const      { return m_bFocused; }
  const CCircle& GetFocusedArea(void) const { return m_focusArea; }

  void SetFocus(const CShape* focusArea);
  void Unfocus(void);

  // implementation of CGUIControl
  virtual void Render();

protected:
  void LoadShaders(void);

private:
  bool    m_bFocused;
  CCircle m_focusArea;

  IFocusRenderer* m_renderer;

  /*
  void* m_costmap;
  void* m_effectMask;
  void* m_start[8];
  void* m_goal[4];
  */
};
