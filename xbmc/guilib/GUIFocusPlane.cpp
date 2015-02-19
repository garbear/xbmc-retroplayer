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

#include "GUIFocusPlane.h"

#include <cmath>

#define CIRCLE_THICKNESS  3.0f

// --- CFocusRendererGL --------------------------------------------------------

#if defined(HAS_GL) || HAS_GLES == 2

#include "cores/VideoRenderers/VideoShaders/VideoFilterShader.h"
#include "guilib/FrameBufferObject.h"
#include "utils/GLUtils.h"
#include "utils/MathUtils.h"
#include "utils/StringUtils.h"
#include "windowing/WindowingFactory.h"

#include "system_gl.h"

#define ROUND_TO_PIXEL(x) (float)(MathUtils::round_int(x))

class CFocusRendererGL : public IFocusRenderer
{
public:
  CFocusRendererGL(CGUIFocusPlane* control);
  virtual ~CFocusRendererGL(void) { Unload(); }

  virtual bool Load(void);
  virtual void Unload(void);
  virtual void RenderStart(void);
  virtual void RenderEnd(void);

private:
  CGUIFocusPlane* const           m_control;
  CFrameBufferObject              m_fbo;
  Shaders::BaseVideoFilterShader* m_pVideoFilterShader;
};

CFocusRendererGL::CFocusRendererGL(CGUIFocusPlane* control) :
    m_control(control),
    m_pVideoFilterShader(NULL)
{
}

bool CFocusRendererGL::Load(void)
{
  return true;

#if 0
  bool bTryGlsl = true;

#if defined(TARGET_POSIX) && !defined(TARGET_DARWIN)
  // with render method set to auto, don't try glsl on ati if we're on linux
  // it seems to be broken in a random way with every new driver release
  bTryGlsl = !StringUtils::StartsWithNoCase(g_Windowing.GetRenderVendor(), "ati");
#endif

  if (/* glCreateProgram && */ bTryGlsl)
  {
    unsigned int sourceWidth = g_graphicsContext.GetWidth();
    unsigned int sourceHeight = g_graphicsContext.GetHeight();
    if (m_fbo.Initialize() && m_fbo.CreateAndBindToTexture(GL_TEXTURE_2D, sourceWidth, sourceHeight, GL_RGBA))
    {
      m_pVideoFilterShader = new ConvolutionFilterShader(m_scalingMethod, m_nonLinStretch);
      if (m_pVideoFilterShader->CompileAndLink())
      {
        //SetTextureFilter(GL_LINEAR);
        //m_renderQuality = RQ_MULTIPASS;
        return true;
      }
    }
  }

  return false;
#endif
}

void CFocusRendererGL::Unload(void)
{
  m_fbo.Cleanup();
}

void CFocusRendererGL::RenderStart(void)
{
#if 0
  glDisable(GL_DEPTH_TEST);

  m_fbo.BeginRender();
  VerifyGLState();
#endif
}

void CFocusRendererGL::RenderEnd(void)
{
#if 0
  m_fbo.EndRender();

  glEnable(GL_TEXTURE_2D);
  glActiveTextureARB(GL_TEXTURE0);
  VerifyGLState();

  m_fbo.SetFiltering(GL_TEXTURE_2D, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  VerifyGLState();

  CRect renderRegion(0, 0, g_graphicsContext.GetWidth(), g_graphicsContext.GetHeight());
  CRect coords(0.0f, 1.0f, 1.0f, 0.0f);;

  glBegin(GL_QUADS);

  glMultiTexCoord2fARB(GL_TEXTURE0, coords.x1, coords.y1);
  glVertex4f(renderRegion.x1, renderRegion.y1, 0, 1.0f);

  glMultiTexCoord2fARB(GL_TEXTURE0, coords.x2, coords.y1);
  glVertex4f(renderRegion.x2, renderRegion.y1, 0, 1.0f);

  glMultiTexCoord2fARB(GL_TEXTURE0, coords.x2, coords.y2);
  glVertex4f(renderRegion.x2, renderRegion.y2, 0, 1.0f);

  glMultiTexCoord2fARB(GL_TEXTURE0, coords.x1, coords.y2);
  glVertex4f(renderRegion.x1, renderRegion.y2, 0, 1.0f);

  glEnd();
  VerifyGLState();

  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
  VerifyGLState();
#endif

  CPoint pos(m_control->GetXPosition(), m_control->GetYPosition());
  g_graphicsContext.SetOrigin(pos.x, pos.y);

  const unsigned int EDGES = 64;
  const CCircle focusArea(m_control->GetFocusedArea());

  glEnable(GL_LINE_SMOOTH);
  glBegin(GL_LINE_LOOP);

  for (unsigned int i = 0; i < EDGES; i++)
  {
    const float theta = i * 2 * (float)M_PI / EDGES;
    const float x = g_graphicsContext.ScaleFinalXCoord(focusArea.x, focusArea.y);
    const float y = g_graphicsContext.ScaleFinalYCoord(focusArea.x, focusArea.y);
    const float z = g_graphicsContext.ScaleFinalZCoord(focusArea.x, focusArea.y);
    const float r = focusArea.r * 1.25f; // Enlarge circle a bit

    glLineWidth(CIRCLE_THICKNESS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(ROUND_TO_PIXEL(x + std::cos(theta) * r), ROUND_TO_PIXEL(y + std::sin(theta) * r), ROUND_TO_PIXEL(z));
  }

  glEnd();
  glBegin(GL_LINE_LOOP);

  for (unsigned int i = 0; i < EDGES; i++)
  {
    const float theta = i * 2 * (float)M_PI / EDGES;
    const float x = g_graphicsContext.ScaleFinalXCoord(focusArea.x, focusArea.y);
    const float y = g_graphicsContext.ScaleFinalYCoord(focusArea.x, focusArea.y);
    const float z = g_graphicsContext.ScaleFinalZCoord(focusArea.x, focusArea.y);
    const float r = focusArea.r * 1.25f + 1.5f; // Enlarge circle a bit

    glLineWidth(CIRCLE_THICKNESS);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex3f(ROUND_TO_PIXEL(x + std::cos(theta) * r), ROUND_TO_PIXEL(y + std::sin(theta) * r), ROUND_TO_PIXEL(z));
  }

  glEnd();
  VerifyGLState();

  g_graphicsContext.RestoreOrigin();
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
  m_renderer = new CFocusRendererGL(this);
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

void CGUIFocusPlane::SetFocus(const CShape* focusArea)
{
  if (focusArea)
  {
    m_bFocused = true;
    m_focusArea = CCircle(focusArea->Center().x, focusArea->Center().y, focusArea->MaxRadius());
  }
}

void CGUIFocusPlane::Unfocus(void)
{
  m_bFocused = false;
}

void CGUIFocusPlane::Render(void)
{
  if (m_bFocused && m_renderer)
    m_renderer->RenderStart();

  CGUIControlGroup::Render();

  if (m_bFocused && m_renderer)
    m_renderer->RenderEnd();
}
