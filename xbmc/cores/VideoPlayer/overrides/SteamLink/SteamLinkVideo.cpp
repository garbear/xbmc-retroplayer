/*
 *      Copyright (C) 2016 Team Kodi
 *      Copyright (C) 2016 Valve Corporation
 *      http://kodi.tv
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "SteamLinkVideo.h"
//#include "DVDVideoCodec.h"
//#include "cores/VideoPlayer/DVDClock.h"
//#include "cores/VideoPlayer/DVDStreamInfo.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"

// Steam Link video API
#include "SLVideo.h"

//#include <cstring>
//#include <unistd.h> // for usleep()

using namespace STEAMLINK;

namespace
{
  void LogFunction(void *pContext, ESLVideoLog eLogLevel, const char *pszMessage)
  {
    // TODO: Problem that log messages end with a newline?
    switch (eLogLevel)
    {
    case k_ESLVideoLogDebug:
      CLog::Log(LOGDEBUG, "%s", pszMessage);
      break;
    case k_ESLVideoLogInfo:
      CLog::Log(LOGINFO, "%s", pszMessage);
      break;
    case k_ESLVideoLogWarning:
      CLog::Log(LOGWARNING, "%s", pszMessage);
      break;
    case k_ESLVideoLogError:
      CLog::Log(LOGERROR, "%s", pszMessage);
      break;
    default:
      break;
    }
  }
}

CSteamLinkVideo::CSteamLinkVideo(CDVDClock* pClock,
                                 CDVDOverlayContainer* pOverlayContainer,
                                 CDVDMessageQueue& parent,
                                 CRenderManager& renderManager,
                                 CProcessInfo &processInfo) :
  CVideoPlayerVideo(pClock, pOverlayContainer, parent, renderManager, processInfo),
  m_bSteamLinkVideo(false)
{
  // TODO: Refcount to allow logging with multiple instances
  SLVideo_SetLogLevel(g_advancedSettings.CanLogComponent(LOGVIDEO) ? k_ESLVideoLogDebug : k_ESLVideoLogError);
  SLVideo_SetLogFunction(LogFunction, nullptr);
}

CSteamLinkVideo::~CSteamLinkVideo()
{
  SLVideo_SetLogFunction(nullptr, nullptr);
}

bool CSteamLinkVideo::OpenStream(CDVDStreamInfo &hint)
{
  m_bSteamLinkVideo = false; // TODO

  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::OpenStream(hint);

  return false;
}

void CSteamLinkVideo::CloseStream(bool bWaitForBuffers)
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::CloseStream(bWaitForBuffers);

  return false;
}

void CSteamLinkVideo::SendMessage(CDVDMsg* pMsg, int priority = 0)
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::SendMessage(pMsg, priority);

  return false;
}

void CSteamLinkVideo::FlushMessages()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::FlushMessages();

  return false;
}

bool CSteamLinkVideo::IsInited() const
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::IsInited();

  return false;
}

bool CSteamLinkVideo::AcceptsData() const
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::AcceptsData();

  return false;
}

bool CSteamLinkVideo::IsStalled() const
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::IsStalled();

  return false;
}

bool CSteamLinkVideo::StepFrame()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::StepFrame();

  return false;
}

void CSteamLinkVideo::Flush(bool sync)
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::Flush(sync);

  return false;
}

void CSteamLinkVideo::WaitForBuffers()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::WaitForBuffers();

  return false;
}

bool CSteamLinkVideo::HasData() const
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::HasData();

  return false;
}

int CSteamLinkVideo::GetLevel() const
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::GetLevel();

  return false;
}

void CSteamLinkVideo::EnableSubtitle(bool bEnable)
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::EnableSubtitle(bEnable);

  return false;
}

bool CSteamLinkVideo::IsSubtitleEnabled()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::IsSubtitleEnabled();

  return false;
}

void CSteamLinkVideo::EnableFullscreen(bool bEnable)
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::EnableFullscreen(bEnable);

  return false;
}

double CSteamLinkVideo::GetDelay()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::GetDelay();

  return false;
}

void CSteamLinkVideo::SetDelay(double delay)
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::SetDelay(delay);

  return false;
}

double CSteamLinkVideo::GetSubtitleDelay()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::GetSubtitleDelay();

  return false;
}

void CSteamLinkVideo::SetSubtitleDelay(double delay)
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::SetSubtitleDelay(delay);

  return false;
}

double CSteamLinkVideo::GetCurrentPts()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::GetCurrentPts();

  return false;
}

double CSteamLinkVideo::GetOutputDelay()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::GetOutputDelay();

  return false;
}

std::string CSteamLinkVideo::GetPlayerInfo()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::GetPlayerInfo();

  return false;
}

int CSteamLinkVideo::GetVideoBitrate()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::GetVideoBitrate();

  return false;
}

std::string CSteamLinkVideo::GetStereoMode()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::GetStereoMode();

  return false;
}

void CSteamLinkVideo::SetSpeed(int iSpeed)
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::SetSpeed(iSpeed);

  return false;
}

int CSteamLinkVideo::GetDecoderBufferSize()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::GetDecoderBufferSize();

  return false;
}

int CSteamLinkVideo::GetDecoderFreeSpace()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::GetDecoderFreeSpace();

  return false;
}

bool CSteamLinkVideo::IsEOS()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::IsEOS();

  return false;
}

bool CSteamLinkVideo::SubmittedEOS() const
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::SubmittedEOS();

  return false;
}









bool CSteamLinkVideo::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
  if (hints.software)
    return false;

  Dispose();

  CSLVideoContext* context = SLVideo_CreateContext();
  if (context)
  {
    CSLVideoStream* stream = nullptr;

    switch (hints.codec)
    {
      case AV_CODEC_ID_H264:
        stream = SLVideo_CreateStream(context, k_ESLVideoFormatH264);
        break;
      default:
        if (g_advancedSettings.CanLogComponent(LOGVIDEO))
          CLog::Log(LOGDEBUG, "%s: Codec not supported", GetName());
        break;
    }

    if (stream)
    {
      // Success
      m_context = context;
      m_stream = stream;

      if (g_advancedSettings.CanLogComponent(LOGVIDEO))
      {
        int width = 0;
        int height = 0;
        GetDisplayResolution(width, height);

        CLog::Log(LOGDEBUG, "%s: Display resolution = %d x %d", GetName(), width, height);
      }
    }
    else
    {
      CLog::Log(LOGERROR, "%s: Failed to create stream", GetName());
      SLVideo_FreeContext(context);
    }
  }
  else
  {
    CLog::Log(LOGERROR, "%s: Failed to create context", GetName());
  }

  return m_context != nullptr;
}

void CSteamLinkVideo::Dispose()
{
  if (m_stream)
  {
    SLVideo_FreeStream(static_cast<CSLVideoStream*>(m_stream));
    m_stream = nullptr;
  }
  if (m_context)
  {
    SLVideo_FreeContext(static_cast<CSLVideoContext*>(m_context));
    m_context = nullptr;
  }
}

int CSteamLinkVideo::Decode(uint8_t *pData, int iSize, double dts, double pts)
{
  if (!pData || iSize == 0)
    return 0;

  int ret = VC_ERROR;

  if (BeginFrame(iSize))
  {
    if (WriteFrameData(pData, iSize))
    {
      if (SubmitFrame())
      {
        ret = VC_PICTURE;

        /*
        if (pts == DVD_NOPTS_VALUE)
        {
          CLog::Log(LOGDEBUG, "SteamLinkVideo: No pts");
          ret = VC_BUFFER;
        }
        else
        {
          CLog::Log(LOGDEBUG, "SteamLinkVideo: Picture!!!!!!!!!!!!!");
        }

        /*
        if (CDVDClock::GetAbsoluteClock() < pts)
        {
          CDVDClock::WaitAbsoluteClock(pts * DVD_TIME_BASE);
          ret = VC_PICTURE;
        }
        else
        {
          ret = VC_BUFFER;
        }

        /*
        const double nowSec = CDVDClock::GetAbsoluteClock() / DVD_TIME_BASE;
        const double ptsSec = pts;// / DVD_TIME_BASE;
        if (nowSec <= ptsSec)
        {
          CLog::Log(LOGDEBUG, "SteamLinkVideo: Sleeping %u ms and showing", (unsigned int)((ptsSec - nowSec) * 1000));
          CDVDClock::WaitAbsoluteClock(pts * DVD_TIME_BASE);
          ret = VC_PICTURE;
        }
        else
        {
          CLog::Log(LOGDEBUG, "SteamLinkVideo: Buffering");
          ret = VC_BUFFER;
        }
        */
      }
      else
        CLog::Log(LOGERROR, "%s: Error submitting frame", GetName());
    }
    else
    {
      CLog::Log(LOGERROR, "%s: Tried to write more data than expected", GetName()); // TODO
    }
  }
  else
  {
    CLog::Log(LOGERROR, "%s: Failed to begin frame", GetName());
  }

  return ret;
}

void CSteamLinkVideo::Reset(void)
{
  // TODO
}

bool CSteamLinkVideo::GetPicture(DVDVideoPicture *pDvdVideoPicture)
{
  int width = 0;
  int height = 0;
  GetDisplayResolution(width, height);

  std::memset(pDvdVideoPicture, 0, sizeof(*pDvdVideoPicture));

  pDvdVideoPicture->dts = DVD_NOPTS_VALUE;
  pDvdVideoPicture->pts = DVD_NOPTS_VALUE;
  pDvdVideoPicture->iWidth = width;
  pDvdVideoPicture->iHeight= height;
  pDvdVideoPicture->iDisplayWidth = width;
  pDvdVideoPicture->iDisplayHeight= height;
  pDvdVideoPicture->format = RENDER_FMT_BYPASS;

  return true;
}

void CSteamLinkVideo::GetDisplayResolution(int &iWidth, int &iHeight)
{
  SLVideo_GetDisplayResolution(static_cast<CSLVideoContext*>(m_context), &iWidth, &iHeight);
}

bool CSteamLinkVideo::BeginFrame(int nFrameSize)
{
  return SLVideo_BeginFrame(static_cast<CSLVideoStream*>(m_stream), nFrameSize) == 0;
}

bool CSteamLinkVideo::WriteFrameData(void *pData, int nDataSize)
{
  return SLVideo_WriteFrameData(static_cast<CSLVideoStream*>(m_stream), pData, nDataSize) == 0;
}

bool CSteamLinkVideo::SubmitFrame()
{
  return SLVideo_SubmitFrame(static_cast<CSLVideoStream*>(m_stream)) == 0;
}
