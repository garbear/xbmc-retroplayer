/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "GUIControllerLayout.h"
#include "games/ControllerLayout.h"
#include "utils/log.h"

using namespace GAME;

CGUIControllerLayout::CGUIControllerLayout(int parentID, int controlID, float posX, float posY, float width, float height)
  : CGUIImage(parentID, controlID, posX, posY, width, height, CTextureInfo())
{
  ControlType = GUICONTROL_CONTROLLERLAYOUT;
}

CGUIControllerLayout::CGUIControllerLayout(const CGUIControllerLayout &from)
  : CGUIImage(from)
{
  ControlType = GUICONTROL_CONTROLLERLAYOUT;
}

void CGUIControllerLayout::Render(void)
{
  CGUIImage::Render();
  // TODO: Render pressed buttons
}

void CGUIControllerLayout::ActivateLayout(const ControllerLayoutPtr& layout)
{
  if (layout)
  {
    m_currentLayout = layout;

    // TODO: Sometimes this fails no window init
    SetFileName(m_currentLayout->ImagePath());
  }
}
