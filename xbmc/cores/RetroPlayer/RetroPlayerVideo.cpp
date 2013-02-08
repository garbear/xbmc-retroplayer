/*
 *      Copyright (C) 2012 Garrett Brown
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "RetroPlayerVideo.h"
#include "guilib/GraphicContext.h"
#include "guilib/Texture.h"
#include "guilib/TextureManager.h"
#include "guilib/XBTF.h"
#include "utils/log.h"

CRetroPlayerVideo::CRetroPlayerVideo()
  : CThread("RetroPlayerVideo"),
    m_queuedFrame(), // TODO: will this be value-initialized without this line?
    m_pixelFormat(XB_FMT_RGB1555) // Libretro has this as the default pixel format
{
}

CRetroPlayerVideo::~CRetroPlayerVideo()
{
  StopThread();
}

void CRetroPlayerVideo::Process()
{
  while (!m_bStop)
  {
    m_uploadEvent.Wait();
    if (m_bStop)
      break;

    CTexture *texture = NULL;

    // First lock: block SendVideoFrame() to grab the texture data
    {
      CSingleLock lock(m_critSection);

      if (!m_queuedFrame.data || m_queuedFrame.uploaded)
        continue;

      texture = new CTexture();
      if (!texture)
        continue;
      texture->LoadFromMemory(m_queuedFrame.width, m_queuedFrame.height, m_queuedFrame.pitch,
        m_pixelFormat, /* hasAlpha = */ false, reinterpret_cast<const unsigned char*>(m_queuedFrame.data));

      m_queuedFrame.uploaded = true;
    }
    
    // Second lock: block the renderer to load the texture to the texture manager
    {
      CSingleLock lock(g_graphicsContext);

      if (!m_currentTexture.empty())
        g_TextureManager.ReleaseTexture(m_currentTexture);

      m_currentTexture = g_TextureManager.LoadFromTexture(texture);

      // Inc the reference counter so the texture will remain available and any
      // calls to GetCurrentTexture() will return a valid texture name
      if (!m_currentTexture.empty())
        g_TextureManager.GetTexture(m_currentTexture);
    }
  }

  delete[] m_queuedFrame.data;
  m_queuedFrame.data = NULL;

  if (!m_currentTexture.empty())
  {
    g_TextureManager.ReleaseTexture(m_currentTexture);
    m_currentTexture.clear();
  }
}

void CRetroPlayerVideo::StopThread(bool bWait /* = true */)
{
  m_bStop = true;
  m_uploadEvent.Set();
  CThread::StopThread(bWait);
}

void CRetroPlayerVideo::SendVideoFrame(const void *data, unsigned width, unsigned height, size_t pitch)
{
  if (IsRunning())
  {
    CSingleLock lock(m_critSection);

    // If frames are the same size, we can avoid an extra allocation
    if (!m_queuedFrame.data || pitch * height != m_queuedFrame.pitch * m_queuedFrame.height)
    {
      delete[] m_queuedFrame.data;
      m_queuedFrame.data = new unsigned char[pitch * height];
      if (!m_queuedFrame.data)
        return;
    }

    memcpy(const_cast<void*>(m_queuedFrame.data), data, pitch * height);
    m_queuedFrame.width    = width;
    m_queuedFrame.height   = height;
    m_queuedFrame.pitch    = pitch;
    m_queuedFrame.uploaded = false;
  }
}

void CRetroPlayerVideo::SetPixelFormat(retro_pixel_format pixelFormat)
{
  switch (pixelFormat)
  {
    case RETRO_PIXEL_FORMAT_XRGB8888:
      //m_pixelFormat = XB_FMT_RGB8;
      m_pixelFormat = XB_FMT_A8R8G8B8;
      break;
    case RETRO_PIXEL_FORMAT_RGB565:
      m_pixelFormat = XB_FMT_R5G6B5;
      break;
    case RETRO_PIXEL_FORMAT_0RGB1555:
    default:
      m_pixelFormat = XB_FMT_RGB1555;
      break;
  }
}
