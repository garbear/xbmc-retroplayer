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

#include "GUIFocusPlane.h"
#include "utils/MathUtils.h"
#include "system_gl.h"

#include <cmath>

#define DEG2RAD  (3.14159f / 180.0f)

CGUIFocusPlane::CGUIFocusPlane(void)
{
  Initialize();
  ControlType = GUICONTROL_FOCUSPLANE;
}

CGUIFocusPlane::CGUIFocusPlane(int parentID, int controlID, float posX, float posY, float width, float height)
  : CGUIControlGroup(parentID, controlID, posX, posY, width, height)
{
  Initialize();
  ControlType = GUICONTROL_FOCUSPLANE;
}

CGUIFocusPlane::CGUIFocusPlane(const CGUIFocusPlane &from)
  : CGUIControlGroup(from)
{
  Initialize();
  ControlType = GUICONTROL_FOCUSPLANE;
}

void CGUIFocusPlane::Initialize(void)
{
  m_bFocused = false;
}

void CGUIFocusPlane::SetFocus(const CCircle& focusArea)
{
  m_bFocused = true;
  m_focusArea = focusArea;
}

void CGUIFocusPlane::SetFocus(const CRect& focusArea)
{
  m_bFocused = true;
  m_focusArea = focusArea.Circumcircle();
}

void CGUIFocusPlane::Unfocus(void)
{
  m_bFocused = false;
}

#define ROUND_TO_PIXEL(x) (float)(MathUtils::round_int(x))

void CGUIFocusPlane::Render(void)
{
  if (!m_bFocused)
  {
    CGUIControlGroup::Render();
  }
  else
  {
    CGUIControlGroup::Render();

    CPoint pos(GetPosition());
    g_graphicsContext.SetOrigin(pos.x, pos.y);

#ifdef HAS_GL

    glBegin(GL_LINE_LOOP);

    const unsigned int EDGES = 32;

    for (unsigned int i = 0; i < EDGES; i++)
    {
      const float theta = i * 2 * (float)M_PI / EDGES;

      const float x = g_graphicsContext.ScaleFinalXCoord(m_focusArea.x, m_focusArea.y);
      const float y = g_graphicsContext.ScaleFinalYCoord(m_focusArea.x, m_focusArea.y);
      const float z = g_graphicsContext.ScaleFinalZCoord(m_focusArea.x, m_focusArea.y);

      const float r = m_focusArea.r * 1.25f; // Enlarge circle a bit

      glVertex3f(ROUND_TO_PIXEL(x + std::cos(theta) * r), ROUND_TO_PIXEL(y + std::sin(theta) * r), ROUND_TO_PIXEL(z));
    }

    glEnd();

#endif

    g_graphicsContext.RestoreOrigin();
  }
}
