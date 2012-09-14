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

#include "linux/PlatformDefs.h"
#include "cores/VideoRenderers/RenderManager.h"
#include "DllSwScale.h"
#include "games/libretro/libretro.h"
#include "threads/Thread.h"

#include <stdint.h>
#include <queue>

struct SwsContext;

class CRetroPlayerVideo : public CThread
{
public:
  struct Frame
  {
    const void * data;
    unsigned int width;
    unsigned int height;
    size_t       pitch;
    Frame() { data = NULL; width = 0; height = 0; pitch = 0; }
    Frame(const void *data, unsigned width, unsigned height, size_t pitch)
      : data(data), width(width), height(height), pitch(pitch) { }
    Frame(const Frame &frame) { *this = frame; }
    Frame &operator=(const Frame &frame)
    {
      if (this != &frame) { data = frame.data; width = frame.width; height = frame.height; pitch = frame.pitch; }
      return *this;
    }
  };

  CRetroPlayerVideo();
  ~CRetroPlayerVideo();

  /**
   * Begin doing what a RetroPlayerVideo does best.
   */
  void GoForth(double framerate);

  /**
   * Send a video frame to be rendered by this class. The pixel format is
   * specified by m_pixelFormat.
   */
  void SendVideoFrame(const void *data, unsigned width, unsigned height, size_t pitch);

  /**
   * Set the m_pixelFormat to match the format used by the game client. If this
   * function is not called, m_pixelFormat defaults to RETRO_PIXEL_FORMAT_0RGB1555.
   * For a list of valid pixel formats, see libretro.h.
   */
  void SetPixelFormat(retro_pixel_format pixelFormat) { m_pixelFormat = pixelFormat; }

  void EnableFullscreen(bool bEnable) { m_bAllowFullscreen = bEnable; }

  void Update(bool bPauseDrawing) { g_renderManager.Update(bPauseDrawing); }

  void Pause() { m_bPaused = true; }
  void UnPause() { m_bPaused = false; m_pauseEvent.Set(); }

  /**
   * This is called immediately after the game client's RunFrame(). Inside
   * RunFrame(), a callback is invoked that calls SendVideoFrame(). Therefore,
   * once RunFrame() exits, a frame is waiting to be rendered and this method
   * wakes the thread to process the frame.
   */
  void Tickle() { m_frameReady.Set(); }

protected:
  virtual void Process();
  //virtual void OnExit();

private:
  bool CheckConfiguration(const DVDVideoPicture &picture);

  std::queue<Frame>  m_frames;
  CEvent             m_frameReady;
  CEvent             m_pauseEvent;
  CCriticalSection   m_critSection;
  retro_pixel_format m_pixelFormat;
  DllSwScale         m_dllSwScale;
  SwsContext        *m_swsContext;

  bool               m_bAllowFullscreen;
  double             m_framerate;
  // Output variables are used to store the current output configuration. Each
  // frame the picture's configuration is compared to the output configuration,
  // and a discrepancy will reconfigure the render manager, as well as saving
  // the new state to these variables.
  unsigned int       m_outputWidth;
  unsigned int       m_outputHeight;
  double             m_outputFramerate;
  bool               m_bPaused;
};
