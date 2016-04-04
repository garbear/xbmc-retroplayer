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
  m_bSteamLinkVideo(false),
  m_context(nullptr),
  m_stream(nullptr)
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
  m_bSteamLinkVideo = false;

  if (!hint.software)
  {
    switch (hint.codec)
    {
      case AV_CODEC_ID_H264:
        m_bSteamLinkVideo = true;
        break;
      default:
        break;
    }
  }

  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::OpenStream(hint);

  CloseStream(true);

  bool bSuccess = false;

  CSLVideoContext* context = SLVideo_CreateContext();
  if (context)
  {
    CSLVideoStream* stream = SLVideo_CreateStream(context, k_ESLVideoFormatH264);
    if (stream)
    {
      bSuccess = true;
      m_context = context;
      m_stream = stream;

      if (g_advancedSettings.CanLogComponent(LOGVIDEO))
      {
        int width = 0;
        int height = 0;
        GetDisplayResolution(width, height);
        CLog::Log(LOGDEBUG, "SteamLinkVideo Display resolution = %d x %d", width, height);
      }
    }
    else
    {
      CLog::Log(LOGERROR, "SteamLinkVideo Failed to create stream");
      SLVideo_FreeContext(context);
    }
  }
  else
  {
    CLog::Log(LOGERROR, "SteamLinkVideo Failed to create context");
  }

  return bSuccess;
}

void CSteamLinkVideo::CloseStream(bool bWaitForBuffers)
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::CloseStream(bWaitForBuffers);

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

void CSteamLinkVideo::Flush(bool sync)
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::Flush(sync);

  // TODO
  m_messageQueue.Flush();
  //m_messageQueue.Put(new CDVDMsgBool(CDVDMsg::GENERAL_FLUSH, sync), 1);
  //m_bAbortOutput = true;
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

void CSteamLinkVideo::OnStartup()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::OnStartup();
}

void CSteamLinkVideo::OnExit()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::OnExit();
}

void CSteamLinkVideo::Process()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::Process();

  // TODO
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
        CLog::Log(LOGERROR, "SteamLinkVideo Error submitting frame", GetName());
    }
    else
    {
      CLog::Log(LOGERROR, "SteamLinkVideo Tried to write more data than expected", GetName()); // TODO
    }
  }
  else
  {
    CLog::Log(LOGERROR, "SteamLinkVideo Failed to begin frame", GetName());
  }

  return ret;
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
