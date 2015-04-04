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

#include "RetroPlayerVideo.h"
#include "cores/dvdplayer/DVDClock.h" // for DVD_NOPTS_VALUE
#include "cores/dvdplayer/DVDCodecs/DVDCodecUtils.h"
#include "cores/dvdplayer/DVDCodecs/Video/DVDVideoCodec.h"
#include "cores/VideoRenderers/RenderFlags.h"
#include "cores/VideoRenderers/RenderManager.h"
#include "cores/FFmpeg.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "system_gl.h"
#include "cores/RetroPlayer/RetroGl.h"

#include "libswscale/swscale.h"

// 1 second should be a good failsafe if the event isn't triggered
#define WAIT_TIMEOUT_MS  1000

CRetroPlayerVideo::CRetroPlayerVideo(void)
  : CThread("RetroPlayerVideo"),
    m_framerate(0.0),
    m_format(AV_PIX_FMT_NONE),
    m_picture(NULL),
    m_swsContext(NULL),
    m_bFrameReady(false),
    m_renderpic(NULL)
{
}

void CRetroPlayerVideo::Cleanup(void)
{
  if (m_swsContext)
  {
    sws_freeContext(m_swsContext);
    m_swsContext = NULL;
  }

  if (m_picture)
  {
    CDVDCodecUtils::FreePicture(m_picture);
    m_picture = NULL;
  }
}

void CRetroPlayerVideo::Start(double framerate)
{
  if (!IsRunning())
  {
    m_framerate = framerate;
    Create();
  }
}

void CRetroPlayerVideo::Stop(void)
{
  StopThread(false);
  m_frameReadyEvent.Set();
}

bool CRetroPlayerVideo::VideoFrame(const uint8_t* data, unsigned int size, unsigned int width, unsigned int height, AVPixelFormat format)
{
  if (!m_bStop && !IsFrameReady())
  {
    if (Configure(width, height, format))
    {
      ColorspaceConversion(data, size, width, height, *m_picture);
      SetFrameReady(true);
      return true;
    }
    else
    {
      Stop();
    }
  }

  return false;
}

bool CRetroPlayerVideo::VideoFrame(AVPixelFormat format, unsigned int size, unsigned int width, unsigned int height, CRetroGlRenderPicture* picture)
{
  if (!m_bStop && !IsFrameReady())
  {
    m_renderpic = picture;
    m_renderpic->texWidth = width;
    m_renderpic->texHeight = height;

    if (Configure(width, height, format))
    {
      ColorspaceConversion(0, size, width, height, *m_picture);
      SetFrameReady(true);
      return true;
    }
    else
    {
      Stop();
    }
  }

  return false;
}

void CRetroPlayerVideo::Process(void)
{
  while (!m_bStop)
  {
    if (AbortableWait(m_frameReadyEvent, WAIT_TIMEOUT_MS) == WAIT_INTERRUPTED)
      break;

    if (IsFrameReady())
    {
      const double sleepTime = 0; // TODO: How is this calculated in DVDPlayer?
      int buffer = g_renderManager.WaitForBuffer(m_bStop, std::max(DVD_TIME_TO_MSEC(sleepTime) + 500, 50));

      if (buffer < 0)
      {
        // There was a timeout waiting for buffer, drop the frame
      }
      else
      {
        int index = g_renderManager.AddVideoPicture(*m_picture);
        if (index < 0)
        {
          // Video device might not be done yet, drop the frame
        }
        else
        {
          g_renderManager.FlipPage(m_bStop);
        }
      }

      SetFrameReady(false);
    }
  }

  Cleanup();
}

bool CRetroPlayerVideo::Configure(unsigned int width, unsigned int height, AVPixelFormat format)
{
  if (!g_renderManager.IsConfigured() ||
      m_format           != format    ||
      m_picture          == NULL      ||
      m_picture->iWidth  != width     ||
      m_picture->iHeight != height)
  {
    // Determine RenderManager flags
    unsigned int flags = CONF_FLAGS_YUVCOEF_BT601 | // color_matrix = 4
                         CONF_FLAGS_FULLSCREEN;      // Allow fullscreen

    CLog::Log(LOGDEBUG, "RetroPlayerVideo: Change configuration: %dx%d, %4.2f fps", width, height, m_framerate);

    int orientation = 0; // (90 = 5, 180 = 2, 270 = 7), if we ever want to use RETRO_ENVIRONMENT_SET_ROTATION

    ERenderFormat render_format;
    if (m_renderpic)
      render_format         = RENDER_FMT_RETROGL;
    else
      render_format         = RENDER_FMT_YUV420P; // PIX_FMT_YUV420P


    if (!g_renderManager.Configure(width, height, width, height, (float)m_framerate,
                                   flags, render_format, 0, orientation))
    {
      CLog::Log(LOGERROR, "RetroPlayerVideo: Failed to configure renderer");
      return false;
    }

    Cleanup();

    m_swsContext = sws_getContext(width, height, format,
                                  width, height, PIX_FMT_YUV420P,
                                  SWS_FAST_BILINEAR | SwScaleCPUFlags(),
                                  NULL, NULL, NULL);


    m_picture = CDVDCodecUtils::AllocatePicture(width, height);

    m_picture->dts            = DVD_NOPTS_VALUE;
    m_picture->pts            = DVD_NOPTS_VALUE;
    m_picture->format         = render_format;
    m_picture->color_range    = 0; // *not* CONF_FLAGS_YUV_FULLRANGE
    m_picture->color_matrix   = 4; // CONF_FLAGS_YUVCOEF_BT601
    m_picture->iFlags         = DVP_FLAG_ALLOCATED;
    m_picture->iDisplayWidth  = width;
    m_picture->iDisplayHeight = height;
    m_picture->iDuration      = 1.0 / m_framerate;
    m_format = format;
  }

  if (m_renderpic)
      m_picture->retro = m_renderpic;

  return true;
}

void CRetroPlayerVideo::ColorspaceConversion(const uint8_t* data, unsigned int size, unsigned int width, unsigned int height, DVDVideoPicture &output)
{
  if (!m_renderpic && data)
  {
    const unsigned int stride = size / height;

    if (stride != 0)
    {
      uint8_t* dataMutable = const_cast<uint8_t*>(data);

      uint8_t* src[] =       { dataMutable,         0,                   0,                   0 };
      int      srcStride[] = { (int)stride,         0,                   0,                   0 };
      uint8_t* dst[] =       { output.data[0],      output.data[1],      output.data[2],      0 };
      int      dstStride[] = { output.iLineSize[0], output.iLineSize[1], output.iLineSize[2], 0 };

      sws_scale(m_swsContext, src, srcStride, 0, height, dst, dstStride);
    }
  }
}

bool CRetroPlayerVideo::IsFrameReady(void)
{
  CSingleLock lock(m_frameReadyMutex);
  return m_bFrameReady;
}

void CRetroPlayerVideo::SetFrameReady(bool bReady)
{
  CSingleLock lock(m_frameReadyMutex);

  m_bFrameReady = bReady;
  if (m_bFrameReady)
    m_frameReadyEvent.Set();
}
