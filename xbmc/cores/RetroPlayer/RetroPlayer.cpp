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

#include "RetroPlayer.h"
#include "cores/dvdplayer/DVDClock.h"
#include "cores/RetroPlayer/RetroPlayerDialogs.h"
#include "cores/VideoRenderers/RenderManager.h"
#include "dialogs/GUIDialogOK.h"
#include "games/tags/GameInfoTag.h"
#include "guilib/Key.h"
#include "settings/Settings.h"
#include "threads/SystemClock.h" // Should auto-save tracking be in GameClient.cpp?
#include "URL.h"
#include "utils/log.h"

#include <assert.h>

#define PLAYSPEED_PAUSED    0
#define PLAYSPEED_NORMAL    1000
#define REWIND_SCALE        4 // 2x rewind is 1/2 speed of play

using namespace ADDON;
using namespace GAMES;
using namespace std;

CRetroPlayer::CRetroPlayer(IPlayerCallback& callback)
  : IPlayer(callback), CThread("RetroPlayer"), m_keyboardCallback(NULL), m_playSpeed(PLAYSPEED_NORMAL)
{
}

CRetroPlayer::~CRetroPlayer()
{
  CloseFile();
}

bool CRetroPlayer::OpenFile(const CFileItem& file, const CPlayerOptions& options)
{
  CLog::Log(LOGINFO, "RetroPlayer: Opening: %s", file.GetPath().c_str());
  m_bStop = false;

  if (IsRunning())
  {
    // If the same file was provided, load the appropriate save state
    if (m_gameClient && file.GetPath().Equals(m_file.GetPath()))
    {
      if (!file.m_startSaveState.empty())
        return m_gameClient->Load(file.m_startSaveState);
      else
        return m_gameClient->AutoLoad();
    }
    CloseFile();
  }

  PrintGameInfo(file);

  // Resolve the file into the appropriate game client. This will open dialogs
  // for user input if necessary.
  GameClientPtr gameClient;
  if (!CRetroPlayerDialogs::GetGameClient(file, gameClient))
    return false;
  assert((bool)gameClient);

  // Load the DLL and retrieve system info from the game client
  if (!gameClient->Init())
  {
    CLog::Log(LOGERROR, "RetroPlayer: Failed to init game client %s", gameClient->ID().c_str());
    return false;
  }

  CLog::Log(LOGINFO, "RetroPlayer: Using game client %s at version %s", gameClient->GetClientName().c_str(),
    gameClient->GetClientVersion().c_str());

  // We need to store a pointer to ourself before sending the callbacks to the game client
  if (!gameClient->OpenFile(file, this))
  {
    CLog::Log(LOGERROR, "RetroPlayer: Error opening file");
    CStdString errorOpening;
    errorOpening.Format(g_localizeStrings.Get(13329).c_str(), file.GetAsUrl().GetFileNameWithoutPath().c_str());
    CGUIDialogOK::ShowAndGetInput(gameClient->Name(), errorOpening, 0, 0); // Error opening %s
    return false;
  }

  // Validate the reported framerate
  if (gameClient->GetFrameRate() < 5.0 || gameClient->GetFrameRate() > 100.0)
  {
    CLog::Log(LOGERROR, "RetroPlayer: Game client reported invalid framerate: %f", gameClient->GetFrameRate());
    return false;
  }

  // Success. We use m_file.GetPath() to check if a file is playing in IsPlaying()
  m_gameClient = gameClient;
  m_file = file;
  m_file.SetPath(m_gameClient->GetFilePath());
  m_PlayerOptions = options;
  
  g_renderManager.PreInit();
  Create();
  CLog::Log(LOGDEBUG, "RetroPlayer: File opened successfully");
  return true;
}

void CRetroPlayer::PrintGameInfo(const CFileItem &file) const
{
  // Get game info tag (from a mutable file item, if necessary)
  const GAME_INFO::CGameInfoTag *tag = file.GetGameInfoTag();
  if (!tag)
  {
    CFileItem temp = file;
    if (temp.LoadGameTag())
      tag = temp.GetGameInfoTag();
  }

  // Dump discovered information to the debug log
  if (tag)
  {
    CLog::Log(LOGDEBUG, "RetroPlayer: ---------------------------------------");
    CLog::Log(LOGDEBUG, "RetroPlayer: Game tag loaded");
    CLog::Log(LOGDEBUG, "RetroPlayer: URL: %s", tag->GetURL().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Platform: %s", tag->GetPlatform().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Title: %s", tag->GetTitle().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Game Code: %s", tag->GetID().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Region: %s", tag->GetRegion().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Publisher: %s", tag->GetPublisher().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Format: %s", tag->GetFormat().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Cartridge Type: %s", tag->GetCartridgeType().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: ---------------------------------------");
  }
}

bool CRetroPlayer::CloseFile()
{
  CLog::Log(LOGDEBUG, "RetroPlayer: Closing file");
  if (m_gameClient)
    m_gameClient->CloseFile();

  m_playSpeed = PLAYSPEED_NORMAL;

  // Set the abort request so that other threads can finish up
  m_bStop = true;
  m_file = CFileItem();

  // Set m_video.m_bStop to false before triggering the event
  m_video.StopThread(false);
  m_pauseEvent.Set();

  // Wait for the main thread to finish up. Since this main thread cleans up
  // all other resources and threads we are done after the StopThread call.
  StopThread(true);

  g_renderManager.UnInit();

  CLog::Log(LOGDEBUG, "RetroPlayer: File closed");
  return true;
}

bool CRetroPlayer::OnAction(const CAction &action)
{
  if (!IsPlaying())
    return false;

  switch (action.GetID())
  {
  case ACTION_SAVE:
    return m_gameClient->AutoSave();
  case ACTION_SAVE1:
  case ACTION_SAVE2:
  case ACTION_SAVE3:
  case ACTION_SAVE4:
  case ACTION_SAVE5:
  case ACTION_SAVE6:
  case ACTION_SAVE7:
  case ACTION_SAVE8:
  case ACTION_SAVE9:
    return m_gameClient->Save(action.GetID() - ACTION_SAVE1 + 1);
  case ACTION_LOAD:
    if (m_playSpeed <= 0)
      ToFFRW(1);
    return m_gameClient->AutoLoad();
  case ACTION_LOAD1:
  case ACTION_LOAD2:
  case ACTION_LOAD3:
  case ACTION_LOAD4:
  case ACTION_LOAD5:
  case ACTION_LOAD6:
  case ACTION_LOAD7:
  case ACTION_LOAD8:
  case ACTION_LOAD9:
    if (m_playSpeed <= 0)
      ToFFRW(1);
    return m_gameClient->Load(action.GetID() - ACTION_LOAD1 + 1);
  }

  return false;
}

void CRetroPlayer::Process()
{
  // Calculate the framerate (how often RunFrame() should be called)
  double framerate = m_gameClient->GetFrameRate();

  // We want to sync the video clock to the audio. The creation of the audio
  // thread will return the decided-upon sample rate.
  unsigned int samplerate = 0;
  double allegedSamplerate = m_gameClient->GetSampleRate();

  // No sound if invalid sample rate
  if (allegedSamplerate > 0)
  {
    // The audio thread will return the sample rate decided by the audio stream
    samplerate = m_audio.GoForth(allegedSamplerate);
    if (samplerate)
    {
      CLog::Log(LOGDEBUG, "RetroPlayer: Created audio stream with sample rate %u from reported rate of %f",
        samplerate, (float)allegedSamplerate);

      // If audio is playing, use that as the reference clock and adjust our framerate accordingly
      double oldFramerate = framerate; // for logging purposes
      framerate *= samplerate / allegedSamplerate;
      CLog::Log(LOGDEBUG, "RetroPlayer: Frame rate changed from %f to %f", (float)oldFramerate, (float)framerate);
    }
    else
      CLog::Log(LOGERROR, "RetroPlayer: Error creating audio stream with sample rate %f", (float)allegedSamplerate);
  }
  else
    CLog::Log(LOGERROR, "RetroPlayer: Error, invalid sample rate %f, continuing without sound", (float)allegedSamplerate);

  // Got our final framerate. Record it back in our game client so that our
  // savestate buffer can be resized accordingly.
  m_gameClient->SetFrameRate(framerate);

  m_video.GoForth(framerate, m_PlayerOptions.fullscreen);

  unsigned int saveTimer = XbmcThreads::SystemClockMillis();

  const double frametime = 1000 * 1000 / framerate; // microseconds
  double nextpts = CDVDClock::GetAbsoluteClock() + frametime;
  CLog::Log(LOGDEBUG, "RetroPlayer: Beginning loop de loop");
  while (!m_bStop)
  {
    if (m_playSpeed == PLAYSPEED_PAUSED)
    {
      // No need to pause audio or video, the absence of frames will pause it
      // 1s should be a good failsafe if the event isn't triggered (shouldn't happen)
      m_pauseEvent.WaitMSec(1000);

      // Reset the clock
      nextpts = CDVDClock::GetAbsoluteClock() + frametime;

      continue;
    }
    else if (m_playSpeed < PLAYSPEED_PAUSED)
    {
      // Need to rewind 2 frames, so that RunFrame() will update the screen
      m_gameClient->RewindFrames(2);
    }

    // Run the game client for the next frame
    if (!m_gameClient->RunFrame())
      break;

    // If the game client uses single frame audio, render those now
    m_audio.Flush();

    if (CSettings::Get().GetBool("gamesgeneral.autosave") &&
      XbmcThreads::SystemClockMillis() - saveTimer > 30000) // every 30 seconds
    {
      m_gameClient->AutoSave();
      saveTimer = XbmcThreads::SystemClockMillis();
    }

    // Slow down (increase nextpts) if we're playing catchup after stalling
    if (nextpts < CDVDClock::GetAbsoluteClock())
      nextpts = CDVDClock::GetAbsoluteClock();

    double realFrameTime = frametime * PLAYSPEED_NORMAL /
      (m_playSpeed > PLAYSPEED_PAUSED ? m_playSpeed : -m_playSpeed / REWIND_SCALE);

    // Slow down to 0.5x (an extra frame) if the audio is delayed
    //if (m_audio.GetDelay() * 1000 > CSettings::Get().GetInt("gamesgeneral.audiodelay"))
    //  nextpts += realFrameTime;

    CDVDClock::WaitAbsoluteClock(nextpts);
    nextpts += realFrameTime;
  }

  m_bStop = true;

  // Save the game before the video cuts out
  m_gameClient->CloseFile();

  m_video.StopThread(true);
  m_audio.StopThread(true);
}

void CRetroPlayer::VideoFrame(const void *data, unsigned width, unsigned height, size_t pitch)
{
  // Verify all game client data. You don't know where that code's been.
  if (data && width && height && pitch)
    m_video.SendVideoFrame(data, width, height, pitch);
}

void CRetroPlayer::AudioSample(int16_t left, int16_t right)
{
  m_audio.SendAudioFrame(left, right);
}

size_t CRetroPlayer::AudioSampleBatch(const int16_t *data, size_t frames)
{
  if (data && frames && m_playSpeed == PLAYSPEED_NORMAL)
    m_audio.SendAudioFrames(data, frames);
  return frames;
}

int16_t CRetroPlayer::GetInputState(unsigned port, unsigned device, unsigned index, unsigned id)
{
  return m_input.GetInput(port, device, index, id);
}

void CRetroPlayer::SetPixelFormat(LIBRETRO::retro_pixel_format pixelFormat)
{
  m_video.SetPixelFormat(pixelFormat);
}

void CRetroPlayer::SetKeyboardCallback(LIBRETRO::retro_keyboard_event_t callback)
{
  m_keyboardCallback = callback;
}

void CRetroPlayer::Pause()
{
  if (m_playSpeed == PLAYSPEED_PAUSED)
  {
    m_playSpeed = PLAYSPEED_NORMAL;
    m_pauseEvent.Set();
  }
  else
    m_playSpeed = PLAYSPEED_PAUSED;
}

void CRetroPlayer::ToFFRW(int iSpeed /* = 0 */)
{
  bool unpause = (m_playSpeed == PLAYSPEED_PAUSED && iSpeed != PLAYSPEED_PAUSED);
  m_playSpeed = iSpeed * PLAYSPEED_NORMAL;
  if (unpause)
    m_pauseEvent.Set();
}

void CRetroPlayer::Seek(bool bPlus, bool bLargeStep)
{
  if (bPlus) // Cannot seek forward in time.
    return;

  if (!m_gameClient)
    return;

  int seek_seconds = bLargeStep ? 10 : 1; // Seem like good values, probably depends on max rewind, needs testing
  m_gameClient->RewindFrames((unsigned int)(seek_seconds * m_gameClient->GetFrameRate()));
}

void CRetroPlayer::SeekPercentage(float fPercent)
{
  if (!m_gameClient || !m_gameClient->GetMaxFrames())
    return; // Rewind not supported for game.

  if (fPercent < 0.0f)
    fPercent = 0.0f;
  else if (fPercent > 100.0f)
    fPercent = 100.0f;

  int max_buffer     = m_gameClient->GetMaxFrames();
  int current_buffer = m_gameClient->GetAvailableFrames();

  int target_buffer  = (int)(max_buffer * fPercent / 100.0f);
  int rewind_frames  = current_buffer - target_buffer;

  if (rewind_frames > 0)
    m_gameClient->RewindFrames(rewind_frames);
}

float CRetroPlayer::GetPercentage()
{
  if (!m_gameClient || !m_gameClient->GetMaxFrames())
    return 0.0f;

  return (100.0f * m_gameClient->GetAvailableFrames()) / m_gameClient->GetMaxFrames();
}

void CRetroPlayer::SeekTime(int64_t iTime)
{
  if (!m_gameClient)
    return;

  // Avoid SIGFPE
  int64_t totalTime = GetTotalTime();
  SeekPercentage(totalTime > 0 ? 100.0f * iTime / totalTime : 0.0f);
}

int64_t CRetroPlayer::GetTime()
{
  if (!m_gameClient || !m_gameClient->GetFrameRate())
    return 0;

  int current_buffer = m_gameClient->GetAvailableFrames();
  return 1000 * current_buffer / m_gameClient->GetFrameRate(); // Millisecs
}

int64_t CRetroPlayer::GetTotalTime()
{
  if (!m_gameClient || !m_gameClient->GetFrameRate())
    return 0;

  int max_buffer = m_gameClient->GetMaxFrames();
  return 1000 * max_buffer / m_gameClient->GetFrameRate(); // Millisecs
}

