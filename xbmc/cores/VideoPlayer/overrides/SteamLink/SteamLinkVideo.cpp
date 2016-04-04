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
#include "cores/VideoPlayer/DVDClock.h"
//#include "cores/VideoPlayer/DVDStreamInfo.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"

// Steam Link video API
#include "SLVideo.h"

//#include <cstring>
#include <ios>     // for std::fixed
#include <iomanip> // for std::setprecision
#include <sstream>
//#include <unistd.h> // for usleep()

using namespace STEAMLINK;

#define MESSAGE_QUEUE_TIMEOUT_MS  100 // TODO

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
  if (m_bSteamLinkVideo)
    CloseStream(true);

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

  return DVD_NOPTS_VALUE;
}

double CSteamLinkVideo::GetOutputDelay()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::GetOutputDelay();

  return 0.0;
}

std::string CSteamLinkVideo::GetPlayerInfo()
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::GetPlayerInfo();

  std::ostringstream s;
  s << "Mb/s: " << std::fixed << std::setprecision(2) << m_videoStats.GetBitrate() / (1024.0*1024.0);
  return s.str();
}

void CSteamLinkVideo::SetSpeed(int iSpeed)
{
  if (!m_bSteamLinkVideo)
    return CVideoPlayerVideo::SetSpeed(iSpeed);

  // TODO
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

  m_videoStats.Start();

  while (!m_bStop)
  {
    CDVDMsg* pMsg;
    MsgQueueReturnCode ret = m_messageQueue.Get(&pMsg, MESSAGE_QUEUE_TIMEOUT_MS);

    if (MSGQ_IS_ERROR(ret))
    {
      CLog::Log(LOGERROR, "SteamLinkVideo: Got MSGQ_ABORT or MSGO_IS_ERROR returned true");
      break;
    }
    else if (ret == MSGQ_TIMEOUT)
    {
      continue;
    }

    if (pMsg->IsType(CDVDMsg::DEMUXER_PACKET))
    {
      DemuxPacket* pPacket = ((CDVDMsgDemuxerPacket*)pMsg)->GetPacket();
      bool bPacketDrop     = ((CDVDMsgDemuxerPacket*)pMsg)->GetPacketDrop();

      // TODO
    }

    pMsg->Release();
  }
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



/*
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
*/
