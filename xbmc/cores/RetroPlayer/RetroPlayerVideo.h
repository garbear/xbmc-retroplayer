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
#pragma once

#include "games/libretro/libretro.h"
#include "threads/Thread.h"
#include "threads/CriticalSection.h"

class CRetroPlayerVideo : public CThread
{
public:
  struct Frame
  {
    const void * data;
    unsigned int width;
    unsigned int height;
    size_t       pitch;
    bool         uploaded;
  };

  CRetroPlayerVideo();
  ~CRetroPlayerVideo();

  /**
   * Send a video frame to be rendered by this class. The pixel format is
   * specified by m_pixelFormat.
   */
  void SendVideoFrame(const void *data, unsigned width, unsigned height, size_t pitch);
  
  void LoadTexture() const { m_uploadEvent.Set(); }

  /**
   * Returns the name of the texture loaded in the texture manager.
   */
  const CStdString &GetCurrentTexture() const { return m_currentTexture; }

  /**
   * Set the m_pixelFormat to match the format used by the game client. If this
   * function is not called, m_pixelFormat defaults to RETRO_PIXEL_FORMAT_0RGB1555.
   * For a list of valid pixel formats, see libretro.h.
   */
  void SetPixelFormat(retro_pixel_format pixelFormat);
  
  virtual void StopThread(bool bWait = true);

protected:
  virtual void Process();

private:
  Frame              m_queuedFrame;
  CStdString         m_currentTexture;
  unsigned int       m_pixelFormat;
  CCriticalSection   m_critSection;
  mutable CEvent     m_uploadEvent;
};
