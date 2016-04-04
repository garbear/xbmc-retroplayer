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

  // implementation of IDVDStreamPlayerVideo via CVideoPlayerVideo
  virtual void Flush(bool sync) override;
  virtual double GetCurrentPts() override;
  virtual double GetOutputDelay() override;
  virtual std::string GetPlayerInfo() override;
  virtual void SetSpeed(int iSpeed) override;

protected:
  // implementation of CTread via CVideoPlayerVideo
  virtual void OnStartup() override;
  virtual void OnExit() override;
  virtual void Process() override;

private:
  /*!
   * Flag to enable Steam Link video. Set this to true when opening the stream
   * to bypass CVideoPlayerVideo.
   */
  bool m_bSteamLinkVideo;

  // Steam Link functions
  void GetDisplayResolution(int &iWidth, int &iHeight);
  bool BeginFrame(int nFrameSize);
  bool WriteFrameData(void *pData, int nDataSize);
  bool SubmitFrame();

  // Steam Link data
  void* m_context;
  void* m_stream;
};

}
