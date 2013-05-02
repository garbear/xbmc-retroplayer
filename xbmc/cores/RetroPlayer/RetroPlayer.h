/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include "RetroPlayerVideo.h"
#include "RetroPlayerAudio.h"
#include "RetroPlayerInput.h"
#include "cores/IPlayer.h"
#include "FileItem.h"
#include "games/GameClient.h"
#include "threads/Thread.h"

#include <stdint.h>

class CRetroPlayer : public IPlayer, public CThread
{
public:
  CRetroPlayer(IPlayerCallback& callback);
  virtual ~CRetroPlayer();

  virtual bool OpenFile(const CFileItem& file, const CPlayerOptions& options);
  virtual bool CloseFile();

  bool InstallGameClient(CFileItem file, ADDON::GameClientPtr &result) const;
  bool ChooseAddon(const CFileItem &file, const CStdStringArray &clients, ADDON::GameClientPtr &result) const;

  virtual bool OnAction(const CAction &action);

  // Upon successful open, m_file is set to the opened file
  virtual bool IsPlaying() const { return !m_bStop && !m_file.GetPath().empty(); }
  virtual void Pause();
  virtual bool IsPaused() const { return m_playSpeed == 0; }

  const CStdString &GetFilePath() { return m_file.GetPath(); }
  CStdString GetGameClient() { return m_gameClient ? m_gameClient->ID() : ""; }

  virtual bool HasVideo() const { return true; }
  virtual bool HasAudio() const { return true; }

  CRetroPlayerInput &GetInput() { return m_input; }

  virtual void GetAudioInfo(CStdString& strAudioInfo) { strAudioInfo = "CRetroPlayer:GetAudioInfo"; }
  virtual void GetVideoInfo(CStdString& strVideoInfo) { strVideoInfo = "CRetroPlayer:GetVideoInfo"; }
  virtual void GetGeneralInfo(CStdString& strGeneralInfo) { strGeneralInfo = "CRetroPlayer:GetGeneralInfo"; }

  //virtual CStdString GetAudioCodecName() { return ""; }
  //virtual CStdString GetVideoCodecName() { return ""; }
  //virtual int  GetAudioBitrate() { return 0; }
  //virtual int  GetChannels() { return 0; }
  //virtual int  GetBitsPerSample() { return 0; }
  //virtual int  GetSampleRate() { return 0; }

  virtual void Update(bool bPauseDrawing = false) { m_video.Update(bPauseDrawing); }
  //virtual void GetVideoRect(CRect& SrcRect, CRect& DestRect) { }
  //virtual void GetVideoAspectRatio(float& fAR) { fAR = 1.0f; }
  //virtual int  GetPictureWidth() { return 0; }
  //virtual int  GetPictureHeight() { return 0; }
  //virtual bool GetStreamDetails(CStreamDetails &details) { return false; }

  //virtual int  GetAudioStreamCount() { return 0; }
  //virtual int  GetAudioStream() { return -1; }
  //virtual void GetAudioStreamName(int iStream, CStdString &strStreamName) { }
  //virtual void SetAudioStream(int iStream) { }
  //virtual void GetAudioStreamLanguage(int iStream, CStdString &strLanguage) { }

  //virtual void  SetAVDelay(float fValue = 0.0f) { return; }
  //virtual float GetAVDelay() { return 0.0f;};

  virtual void ToFFRW(int iSpeed = 0);
  // In the future, a "back buffer" will store game history to enable rewinding
  // Braid-style. The size of this back buffer will be known, from which the
  // time can be computed, or perhaps a set time like 30s from which the buffer
  // size is computed. Regardless, the "duration" of the item will be equal to
  // this buffer, and will allow seeking and rewinding. Fast-forwarding will
  // simply play the game at a faster speed, and will cause this buffer to fill
  // faster. When the buffer is full, the progress bar will reach 100% (30s),
  // which will look like a track being finished, but instead of exiting the
  // game will continue to play. We will need these functions then:
  // CanSeek()
  // Seek()
  // SeekPercentage()
  // GetPercentage()
  // SeekTime()
  // GetTime()
  // GetTotalTime()
  // GetActualFPS()

  // Allows FF. (RW might not be possible depending on game.)
  virtual bool CanSeek() { return true; }
  virtual void Seek(bool bPlus = true, bool bLargeStep = false);
  virtual void SeekPercentage(float fPercent = 0);
  virtual float GetPercentage();
  virtual void SeekTime(int64_t iTime = 0);
  virtual int64_t GetTime();
  virtual int64_t GetTotalTime();


protected:
  virtual void Process();

private:
  // Static functions used to send and receive data from the game clients
  static void    OnVideoFrame(const void *data, unsigned width, unsigned height, size_t pitch);
  static void    OnAudioSample(int16_t left, int16_t right);
  static size_t  OnAudioSampleBatch(const int16_t *data, size_t frames);
  static int16_t OnInputState(unsigned port, unsigned device, unsigned index, unsigned id);
  static void    OnSetPixelFormat(retro_pixel_format pixelFormat); // retro_pixel_format defined in libretro.h
  static void    OnSetKeyboardCallback(retro_keyboard_event_t callback); // retro_keyboard_event_t defined in libretro.h

  // So that the static functions above may invoke audio, video and input callbacks
  static CRetroPlayer *m_retroPlayer;
  static ADDON::CGameClient::DataReceiver m_callbacks;
  static retro_keyboard_event_t m_keyboardCallback; // unused

  CRetroPlayerVideo    m_video;
  CRetroPlayerAudio    m_audio;
  CRetroPlayerInput    m_input;
  ADDON::GameClientPtr m_gameClient;

  CFileItem            m_file;
  CPlayerOptions       m_PlayerOptions;
  int                  m_playSpeed; // Normal play speed is PLAYSPEED_NORMAL (1000)
  CEvent               m_pauseEvent;
};
