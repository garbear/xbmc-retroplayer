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
#pragma once

#include "cores/VideoPlayer/VideoPlayerVideo.h"

namespace STEAMLINK
{

class CSteamLinkVideo : public CVideoPlayerVideo
{
public:
  CSteamLinkVideo(CDVDClock* pClock,
                  CDVDOverlayContainer* pOverlayContainer,
                  CDVDMessageQueue& parent,
                  CRenderManager& renderManager,
                  CProcessInfo &processInfo);
  virtual ~CSteamLinkVideo();

  // implementation of IDVDStreamPlayer via CVideoPlayerVideo
  virtual bool OpenStream(CDVDStreamInfo &hint) override;
  virtual void CloseStream(bool bWaitForBuffers) override;
  virtual void SendMessage(CDVDMsg* pMsg, int priority = 0) override;
  virtual void FlushMessages() override;
  virtual bool IsInited() const override;
  virtual bool AcceptsData() const override;
  virtual bool IsStalled() const override;

  // implementation of IDVDStreamPlayerVideo via CVideoPlayerVideo
  virtual bool StepFrame() override;
  virtual void Flush(bool sync) override;
  virtual void WaitForBuffers() override;
  virtual bool HasData() const override;
  virtual int  GetLevel() const override;
  virtual void EnableSubtitle(bool bEnable) override;
  virtual bool IsSubtitleEnabled() override;
  virtual void EnableFullscreen(bool bEnable) override;
  virtual double GetDelay() override;
  virtual void SetDelay(double delay) override;
  virtual double GetSubtitleDelay() override;
  virtual void SetSubtitleDelay(double delay) override;
  virtual double GetCurrentPts() override;
  virtual double GetOutputDelay() override;
  virtual std::string GetPlayerInfo() override;
  virtual int GetVideoBitrate() override;
  virtual std::string GetStereoMode() override;
  virtual void SetSpeed(int iSpeed) override;
  virtual int  GetDecoderBufferSize() override;
  virtual int  GetDecoderFreeSpace() override;
  virtual bool IsEOS() override;
  virtual bool SubmittedEOS() const override;

private:
  /*!
   * Flag to enable Steam Link video. Set this to true when opening the stream
   * to bypass CVideoPlayerVideo.
   */
  bool m_bSteamLinkVideo;
};

}
