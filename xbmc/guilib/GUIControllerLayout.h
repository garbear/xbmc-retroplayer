/*
 *      Copyright (C) 2014 Team XBMC
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

#include "GUIImage.h"
#include "games/GameTypes.h"

class CGUIControllerLayout : public CGUIImage
{
public:
  CGUIControllerLayout(int parentID, int controlID, float posX, float posY, float width, float height);
  CGUIControllerLayout(const CGUIControllerLayout &from);
  virtual ~CGUIControllerLayout(void) { }
  virtual CGUIControllerLayout *Clone() const { return new CGUIControllerLayout(*this); }

  virtual void Render();

  void ActivateLayout(const GAME::ControllerLayoutPtr& layout);

private:
  GAME::ControllerLayoutPtr m_currentLayout;
};
