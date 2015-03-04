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

#include <cmath>

#define CIRCLE_THICKNESS  3.0f

#define DEG2RAD  (3.14159f / 180.0f)

// --- CFocusRendererGL --------------------------------------------------------

#if defined(HAS_GL) || HAS_GLES == 2

#include "cores/VideoRenderers/VideoShaders/VideoFilterShader.h"
#include "guilib/FrameBufferObject.h"
#include "utils/MathUtils.h"
#include "utils/StringUtils.h"
#include "windowing/WindowingFactory.h"

#include "system_gl.h"

#define ROUND_TO_PIXEL(x) (float)(MathUtils::round_int(x))

class CFocusRendererGL : public IFocusRenderer
{
public:
  CFocusRendererGL(void);
  virtual ~CFocusRendererGL(void) { Unload(); }

  virtual bool Load(void);
  virtual void Unload(void);
  virtual void RenderStart(const CPoint& origin, const CCircle& focusArea);
  virtual void RenderEnd(void);

private:
  bool                            m_bLoaded;
  CPoint                          m_origin;
  CCircle                         m_focusArea;
  CFrameBufferObject              m_fbo;
  Shaders::BaseVideoFilterShader* m_pVideoFilterShader;
};

CFocusRendererGL::CFocusRendererGL(void) :
    m_bLoaded(false),
    m_pVideoFilterShader(NULL)
{
}

bool CFocusRendererGL::Load(void)
{
  if (!m_bLoaded)
  {
    bool bTryGlsl = true;

#if defined(TARGET_POSIX) && !defined(TARGET_DARWIN)
    // with render method set to auto, don't try glsl on ati if we're on linux
    // it seems to be broken in a random way with every new driver release
    bTryGlsl = !StringUtils::StartsWithNoCase(g_Windowing.GetRenderVendor(), "ati");
#endif

    if (/* glCreateProgram && */ bTryGlsl)
    {
      m_bLoaded = true;

      /* TODO
      unsigned int sourceWidth = 1280;
      unsigned int sourceHeight = 720;
      if (m_fbo.Initialize() && m_fbo.CreateAndBindToTexture(GL_TEXTURE_2D, sourceWidth, sourceHeight, GL_RGBA))
      {
        m_pVideoFilterShader = new ConvolutionFilterShader(m_scalingMethod, m_nonLinStretch);
        if (m_pVideoFilterShader->CompileAndLink())
        {
          SetTextureFilter(GL_LINEAR);
          //m_renderQuality = RQ_MULTIPASS;
          m_bLoaded = true;
        }
      }
      */
    }
  }

  return m_bLoaded;
}

void CFocusRendererGL::Unload(void)
{
  if (m_bLoaded)
  {
    m_fbo.Cleanup();
    m_bLoaded = false;
  }
}

void CFocusRendererGL::RenderStart(const CPoint& origin, const CCircle& focusArea)
{
  if (m_bLoaded)
  {
    m_origin = origin;
    m_focusArea = focusArea;
  }
}

void CFocusRendererGL::RenderEnd(void)
{
  if (m_bLoaded)
  {
    g_graphicsContext.SetOrigin(m_origin.x, m_origin.y);

    glEnable(GL_LINE_SMOOTH);

    glBegin(GL_LINE_LOOP);

    const unsigned int EDGES = 64;

    for (unsigned int i = 0; i < EDGES; i++)
    {
      const float theta = i * 2 * (float)M_PI / EDGES;

      const float x = g_graphicsContext.ScaleFinalXCoord(m_focusArea.x, m_focusArea.y);
      const float y = g_graphicsContext.ScaleFinalYCoord(m_focusArea.x, m_focusArea.y);
      const float z = g_graphicsContext.ScaleFinalZCoord(m_focusArea.x, m_focusArea.y);

      const float r = m_focusArea.r * 1.25f; // Enlarge circle a bit

      glLineWidth(CIRCLE_THICKNESS);
      glColor3f(1.0f, 1.0f, 1.0f);
      glVertex3f(ROUND_TO_PIXEL(x + std::cos(theta) * r), ROUND_TO_PIXEL(y + std::sin(theta) * r), ROUND_TO_PIXEL(z));
    }

    for (unsigned int i = 0; i < EDGES; i++)
    {
      const float theta = i * 2 * (float)M_PI / EDGES;

      const float x = g_graphicsContext.ScaleFinalXCoord(m_focusArea.x, m_focusArea.y);
      const float y = g_graphicsContext.ScaleFinalYCoord(m_focusArea.x, m_focusArea.y);
      const float z = g_graphicsContext.ScaleFinalZCoord(m_focusArea.x, m_focusArea.y);

      const float r = m_focusArea.r * 1.25f + 1.5f; // Enlarge circle a bit

      glLineWidth(CIRCLE_THICKNESS);
      glColor3f(0.0f, 0.0f, 0.0f);
      glVertex3f(ROUND_TO_PIXEL(x + std::cos(theta) * r), ROUND_TO_PIXEL(y + std::sin(theta) * r), ROUND_TO_PIXEL(z));
    }

    glEnd();

    g_graphicsContext.RestoreOrigin();
  }
}

#endif

// --- CGUIFocusPlane ----------------------------------------------------------

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

#if defined(HAS_GL) || HAS_GLES == 2
  m_renderer = new CFocusRendererGL;
#else
  m_renderer = NULL;
#endif

  if (m_renderer && !m_renderer->Load())
  {
    delete m_renderer;
    m_renderer = NULL;
  }
}

CGUIFocusPlane::~CGUIFocusPlane(void)
{
  delete m_renderer;
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

void CGUIFocusPlane::Render(void)
{
  if (m_bFocused && m_renderer)
    m_renderer->RenderStart(GetPosition(), m_focusArea);

  CGUIControlGroup::Render();

  if (m_bFocused && m_renderer)
    m_renderer->RenderEnd();
}
