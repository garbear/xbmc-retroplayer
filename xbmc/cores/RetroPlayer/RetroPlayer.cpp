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
#include "addons/AddonDatabase.h"
#include "addons/AddonInstaller.h"
#include "addons/AddonManager.h"
#include "cores/dvdplayer/DVDClock.h"
#include "cores/VideoRenderers/RenderManager.h"
#include "dialogs/GUIDialogContextMenu.h"
#include "dialogs/GUIDialogOK.h"
#include "games/GameManager.h"
#include "games/tags/GameInfoTag.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/Key.h"
#include "settings/GUISettings.h"
#include "URL.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

#define PLAYSPEED_PAUSED    0
#define PLAYSPEED_NORMAL    1000
#define REWIND_SCALE        4 // 2x rewind is 1/2 speed of play

using namespace ADDON;

CRetroPlayer *CRetroPlayer::m_retroPlayer = NULL;

// Callback installation
CGameClient::DataReceiver CRetroPlayer::m_callbacks(OnVideoFrame,
                                                    OnAudioSample,
                                                    OnAudioSampleBatch,
                                                    OnInputState,
                                                    OnSetPixelFormat,
                                                    OnSetKeyboardCallback);

retro_keyboard_event_t CRetroPlayer::m_keyboardCallback = NULL;

CRetroPlayer::CRetroPlayer(IPlayerCallback& callback)
  : IPlayer(callback), CThread("RetroPlayer"), m_playSpeed(PLAYSPEED_NORMAL)
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
    CloseFile();

  // Get game info tag (from a mutable file item, if necessary)
  const GAME_INFO::CGameInfoTag *tag = file.GetGameInfoTag();
  CFileItem temp;
  if (!tag)
  {
    temp = file;
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

  // Now we need to see how many game clients contend for this file
  CStdStringArray candidates;
  CGameManager::Get().GetGameClientIDs(file, candidates);
  if (candidates.empty())
  {
    // No compatible game clients. Enable just-in-time game client installation
    // by asking the user if they would like to download and install a client
    // and then use it to play the game.
    if (!InstallGameClient(file, m_gameClient))
      return false;
  }
  else if (candidates.size() == 1)
  {
    AddonPtr addon;
    CAddonMgr::Get().GetAddon(candidates[0], addon, ADDON_GAMEDLL);
    m_gameClient = boost::dynamic_pointer_cast<CGameClient>(addon);
  }
  else
  {
    if (!ChooseAddon(file, candidates, m_gameClient))
      return false;
  }

  if (!m_gameClient)
  {
    CLog::Log(LOGERROR, "RetroPlayer: No game client");
    // "Playback failed"
    // "One or more items failed to play."
    // "Check the log file for details."
    CGUIDialogOK::ShowAndGetInput(16026, 16027, 16029, 0);
    return false;
  }

  // Load the DLL and retrieve system info from the game client
  if (!m_gameClient->Init())
  {
    CLog::Log(LOGERROR, "RetroPlayer: Failed to init game client %s", m_gameClient->ID().c_str());
    return false;
  }

  CLog::Log(LOGINFO, "RetroPlayer: Using game client %s at version %s", m_gameClient->GetClientName().c_str(),
    m_gameClient->GetClientVersion().c_str());

  // We need to store a pointer to ourself before sending the callbacks to the game client
  m_retroPlayer = this;
  if (!m_gameClient->OpenFile(file, m_callbacks))
  {
    CLog::Log(LOGERROR, "RetroPlayer: Error opening file");
    CStdString errorOpening;
    errorOpening.Format(g_localizeStrings.Get(13329).c_str(), file.GetAsUrl().GetFileNameWithoutPath().c_str());
    CGUIDialogOK::ShowAndGetInput(m_gameClient->Name(), errorOpening, 0, 0); // Error opening %s
    m_gameClient.reset();
    return false;
  }

  // Validate the reported framerate
  if (m_gameClient->GetFrameRate() < 5.0 || m_gameClient->GetFrameRate() > 100.0)
  {
    CLog::Log(LOGERROR, "RetroPlayer: Game client reported invalid framerate: %f", m_gameClient->GetFrameRate());
    m_gameClient.reset();
    return false;
  }

  // Success. We use m_file.GetPath() to check if a file is playing in IsPlaying()
  m_file = file;
  m_file.SetPath(m_gameClient->GetFilePath());
  m_PlayerOptions = options;

  g_renderManager.PreInit();
  Create();
  CLog::Log(LOGDEBUG, "RetroPlayer: File opened successfully");
  return true;
}

bool CRetroPlayer::InstallGameClient(CFileItem file, GameClientPtr &result) const
{
  // If an explicit game client was specified, try to download that
  if (file.HasProperty("gameclient"))
  {
    // First, make sure the game client isn't installed
    CStdString id(file.GetProperty("gameclient").asString());
    CLog::Log(LOGDEBUG, "RetroPlayer: Trying to install game client %s", id.c_str());
    AddonPtr addon;
    bool installed = CAddonMgr::Get().GetAddon(id, addon, ADDON_GAMEDLL, false);
    if (!installed || !addon)
    {
      // Now make sure it exists in a remote repository
      CStdStringArray candidates;
      GameClientPtr gc;
      CAddonDatabase database;
      database.Open();
      if (database.GetAddon(id, addon) && (gc = boost::dynamic_pointer_cast<CGameClient>(addon)) &&
          gc->CanOpen(file))
      {
        CLog::Log(LOGDEBUG, "RetroPlayer: Installing game client %s", id.c_str());
        addon.reset();
        if (CAddonInstaller::Get().PromptForInstall(id, addon) && addon && addon->Type() == ADDON_GAMEDLL)
        {
          result = boost::dynamic_pointer_cast<CGameClient>(addon);
          return true;
        }
      }
    }
    file.ClearProperty("gameclient"); // don't want this to interfere later on
  }

  // First, ask the user if they would like to install a game client or go to
  // the add-on manager
  CContextButtons choices;
  choices.Add(0, 24026); // Install emulator
  choices.Add(1, 24049); // Add-on manager

  int btnid = CGUIDialogContextMenu::ShowAndGetChoice(choices);
  if (btnid == 0) // Install emulator
  {
    CContextButtons emuChoices;

    VECADDONS addons;
    CStdStringArray candidates;
    CAddonDatabase database;
    database.Open();
    database.GetAddons(addons);
    for (VECADDONS::const_iterator itRemote = addons.begin(); itRemote != addons.end(); itRemote++)
    {
      if (!(*itRemote)->IsType(ADDON_GAMEDLL))
        continue;
      GameClientPtr gc = boost::dynamic_pointer_cast<CGameClient>(*itRemote);
      // Require extensions to be provided by the game client to filter ones
      // that are probably unneccessary
      if (gc && !gc->GetConfig().extensions.empty() && gc->CanOpen(file))
      {
        AddonPtr addon;
        // If the game client is already installed and enabled, exclude it from the list
        if (CAddonMgr::Get().GetAddon(gc->ID(), addon, ADDON_GAMEDLL, true))
          continue;
        // If GetAddon() returns false, but the add-on still exists, then the
        // add-on is disabled; include it and mark it as such
        if (addon) // NAME (disabled)
          emuChoices.Add(candidates.size(), gc->Name() + " (" + g_localizeStrings.Get(1223) + ")");
        else
        {
          // Don't include broken remote add-ons
          if (!(*itRemote)->Props().broken.empty())
            continue;
          emuChoices.Add(candidates.size(), gc->Name());
        }
        candidates.push_back(gc->ID());
      }
    }

    if (candidates.empty())
    {
      CLog::Log(LOGDEBUG, "RetroPlayer: No compatible game clients for installation");
      // "Playback failed"
      // "No compatible emulators found for file:"
      // "FILENAME"
      CGUIDialogOK::ShowAndGetInput(16026, 16023, URIUtils::GetFileName(file.GetPath()), 0);
      return false;
    }
    else
    {
      int btnid2 = CGUIDialogContextMenu::ShowAndGetChoice(emuChoices);
      if (btnid2 < 0 || btnid2 >= (int)candidates.size())
      {
        CLog::Log(LOGDEBUG, "RetroPlayer: User canceled game client installation selection");
        return false;
      }
      else
      {
        AddonPtr addon;
        CStdString id(candidates[btnid2]);
        if (CAddonMgr::Get().GetAddon(id, addon, ADDON_GAMEDLL, true))
        {
          // We continued earlier, shouldn't be here now
          CLog::Log(LOGDEBUG, "RetroPlayer: Game client %s already installed, using it", id.c_str());
        }
        else if (addon)
        {
          CLog::Log(LOGDEBUG, "RetroPlayer: Game client %s installed but disabled, enabling it", id.c_str());
          // TODO: Prompt the user to enable it
          CAddonDatabase database;
          database.Open();
          database.DisableAddon(id, false, false);
          CAddonMgr::Get().GetAddon(id, addon);
        }
        else
        {
          CLog::Log(LOGDEBUG, "RetroPlayer: Installing game client %s", id.c_str());
          if (!(CAddonInstaller::Get().PromptForInstall(id, addon) && addon && addon->Type() == ADDON_GAMEDLL))
          {
            CLog::Log(LOGDEBUG, "RetroPlayer: Game client installation canceled/failed");
            // "id"
            // "Installation failed"
            // "Check the log file for details."
            CGUIDialogOK::ShowAndGetInput(id.c_str(), 114, 16029, 0);
            return false;
          }
        }
        result = boost::dynamic_pointer_cast<CGameClient>(addon);
        return true;
      }
    }
  }
  else if (btnid == 1) // Add-on manager
  {
    // Queue the file so that if a compatible game client is installed, the
    // user will be asked to launch the file.
    CGameManager::Get().QueueFile(file);
    CLog::Log(LOGDEBUG, "RetroPlayer: User chose to go to the add-on manager");
    CStdStringArray params;
    params.push_back("addons://all/xbmc.gameclient");
    g_windowManager.ActivateWindow(WINDOW_ADDON_BROWSER, params);
    return false;
  }
  else
  {
    CLog::Log(LOGDEBUG, "RetroPlayer: User canceled game client selection");
    return false;
  }

  // We shouldn't fall through, but if so, this will trigger a failed open
  result = GameClientPtr();
  return true;
}

bool CRetroPlayer::ChooseAddon(const CFileItem &file, const CStdStringArray &addonIds, GameClientPtr &result) const
{
  CLog::Log(LOGDEBUG, "RetroPlayer: Multiple clients found: %s", StringUtils::JoinString(addonIds, ", ").c_str());
  std::vector<GameClientPtr> clients;
  CContextButtons choices;
  for (unsigned int i = 0; i < addonIds.size(); i++)
  {
    AddonPtr addon;
    // TODO: Sort by add-on name
    CAddonMgr::Get().GetAddon(addonIds[i], addon, ADDON_GAMEDLL);
    GameClientPtr client = boost::dynamic_pointer_cast<CGameClient>(addon);
    if (client)
    {
      clients.push_back(client);
      choices.Add(i, client->Name());
    }
  }

  // Choice to go to the add-on manager
  choices.Add(choices.size(), 24025); // "Manage emulators..."

  int btnid = CGUIDialogContextMenu::ShowAndGetChoice(choices);
  if (0 <= btnid && btnid < (int)clients.size())
  {
    CLog::Log(LOGDEBUG, "RetroPlayer: Using %s", clients[btnid]->ID().c_str());
    result = clients[btnid];
    return true;
  }
  else if (btnid == (int)clients.size())
  {
    // Queue the file so that if a compatible game client is installed, the
    // user will be asked to launch the file.
    CGameManager::Get().QueueFile(file);

    CLog::Log(LOGDEBUG, "RetroPlayer: User chose to go to the add-on manager");
    CStdStringArray params;
    params.push_back("addons://all/xbmc.gameclient");
    g_windowManager.ActivateWindow(WINDOW_ADDON_BROWSER, params);
  }
  else
    CLog::Log(LOGDEBUG, "RetroPlayer: User cancelled game client selection");

  return false;
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
    m_gameClient->RunFrame();

    // If the game client uses single frame audio, render those now
    m_audio.Flush();

    // Slow down (increase nextpts) if we're playing catchup after stalling
    if (nextpts < CDVDClock::GetAbsoluteClock())
      nextpts = CDVDClock::GetAbsoluteClock();

    double realFrameTime = frametime * PLAYSPEED_NORMAL /
      (m_playSpeed > PLAYSPEED_PAUSED ? m_playSpeed : -m_playSpeed / REWIND_SCALE);

    // Slow down to 0.5x (an extra frame) if the audio is delayed
    //if (m_audio.GetDelay() * 1000 > g_guiSettings.GetInt("games.audiodelay"))
    //  nextpts += realFrameTime;

    CDVDClock::WaitAbsoluteClock(nextpts);
    nextpts += realFrameTime;
  }

  m_bStop = true;
  m_gameClient->CloseFile();

  m_video.StopThread(true);
  m_audio.StopThread(true);
}

/* static */
void CRetroPlayer::OnVideoFrame(const void *data, unsigned width, unsigned height, size_t pitch)
{
  // Verify all game client data. You don't know where that code's been.
  if (data && width && height && pitch)
    m_retroPlayer->m_video.SendVideoFrame(data, width, height, pitch);
}

/* static */
void CRetroPlayer::OnAudioSample(int16_t left, int16_t right)
{
  m_retroPlayer->m_audio.SendAudioFrame(left, right);
}

/* static */
size_t CRetroPlayer::OnAudioSampleBatch(const int16_t *data, size_t frames)
{
  if (data && frames && m_retroPlayer->m_playSpeed == PLAYSPEED_NORMAL)
    m_retroPlayer->m_audio.SendAudioFrames(data, frames);
  return frames;
}

/* static */
int16_t CRetroPlayer::OnInputState(unsigned port, unsigned device, unsigned index, unsigned id)
{
  return 0;
}

/* static */
void CRetroPlayer::OnSetPixelFormat(retro_pixel_format pixelFormat)
{
  m_retroPlayer->m_video.SetPixelFormat(pixelFormat);
}

/* static */
void CRetroPlayer::OnSetKeyboardCallback(retro_keyboard_event_t callback)
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

