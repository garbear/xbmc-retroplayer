/*
 *      Copyright (C) 2012-2015 Team XBMC
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

#include "threads/Thread.h"

#include "libavutil/pixfmt.h"

#include <stdint.h>

struct DVDVideoPicture;
struct SwsContext;

class CRetroPlayerVideo : protected CThread
{
public:
  CRetroPlayerVideo(void);
  virtual ~CRetroPlayerVideo(void) { Cleanup(); }

  void Start(double framerate);
  void Stop(void);

  bool VideoFrame(AVPixelFormat format, unsigned int width, unsigned int height, const uint8_t* data);

protected:
  virtual void Process(void);

private:
  void Cleanup(void);

  bool Configure(AVPixelFormat format, unsigned int width, unsigned int height);
  
  void ColorspaceConversion(AVPixelFormat format, unsigned int width, unsigned int height, const uint8_t* data, DVDVideoPicture &output);

  bool IsFrameReady(void);
  void SetFrameReady(bool bReady);
  
  static unsigned int GetPitch(AVPixelFormat format, unsigned int width);

  double            m_framerate;
  AVPixelFormat     m_format;
  DVDVideoPicture*  m_picture;
  SwsContext*       m_swsContext;
  bool              m_bFrameReady;
  CCriticalSection  m_frameReadyMutex;
  CEvent            m_frameReadyEvent;
};
