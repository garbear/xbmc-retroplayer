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

#include "RetroPlayerAudio.h"
#include "cores/AudioEngine/AEFactory.h"
#include "cores/AudioEngine/Interfaces/AEStream.h"
#include "utils/log.h"

// Temporary - notify users about unsupported samplerate
#include "dialogs/GUIDialogOK.h"
#include "utils/StringUtils.h"

CRetroPlayerAudio::CRetroPlayerAudio()
  : m_pAudioStream(NULL)
{
}

void CRetroPlayerAudio::Cleanup()
{
  if (m_pAudioStream)
  {
    CAEFactory::FreeStream(m_pAudioStream);
    m_pAudioStream = NULL;
  }
}

bool CRetroPlayerAudio::Start(AEDataFormat format, double samplerate)
{
  if (m_pAudioStream == NULL)
  {
    const unsigned int newsamplerate = FindSampleRate(samplerate);

    CLog::Log(LOGINFO, "RetroPlayerAudio: Creating audio stream, sample rate hint = %u", newsamplerate);
    static enum AEChannel map[3] = { AE_CH_FL, AE_CH_FR, AE_CH_NULL };
    m_pAudioStream = CAEFactory::MakeStream(format, newsamplerate, newsamplerate, CAEChannelInfo(map), AESTREAM_AUTOSTART);

    if (!m_pAudioStream)
    {
      CLog::Log(LOGERROR, "RetroPlayerAudio: Failed to create audio stream");
      return false;
    }
    else if (newsamplerate != m_pAudioStream->GetSampleRate())
    {
      // For real-time (ish) audio, we need to avoid resampling
      CLog::Log(LOGERROR, "RetroPlayerAudio: sink sample rate (%u) doesn't match", m_pAudioStream->GetSampleRate());

      // Temporary: Notify the user via GUI box
      std::string msg = StringUtils::Format("Sample rate not supported by audio device: %uHz", m_pAudioStream->GetSampleRate());
      CGUIDialogOK::ShowAndGetInput(257, msg.c_str(), "Continuing without sound", 0);

      Cleanup();

      return false;
    }
  }

  return true;
}

unsigned int CRetroPlayerAudio::AudioFrames(AEDataFormat format, unsigned int frames, const uint8_t* data)
{
  // TODO: reconfigure if format changes

  unsigned int framesCopied = 0;

  if (m_pAudioStream)
  {
    uint8_t* dataMutable = const_cast<uint8_t*>(data);
    framesCopied = m_pAudioStream->AddData(&dataMutable, 0, frames);
  }

  return framesCopied;
}

double CRetroPlayerAudio::GetDelay() const
{
  return m_pAudioStream ? m_pAudioStream->GetDelay() : 0.0;
}

unsigned int CRetroPlayerAudio::GetSampleRate() const
{
  return m_pAudioStream ? m_pAudioStream->GetSampleRate() : 0;
}

unsigned int CRetroPlayerAudio::FindSampleRate(double samplerate)
{
  // List comes from AESinkALSA.cpp
  static unsigned int sampleRateList[] = {5512, 8000, 11025, 16000, 22050, 32000, 44100, 48000, 0};
  for (unsigned int *rate = sampleRateList; ; rate++)
  {
    if (*(rate + 1) == 0)
    {
      // Reached the end of our list
      return *rate;
    }
    if ((unsigned int)samplerate < (*rate + *(rate + 1)) / 2)
    {
      // samplerate is between this rate and the next, so use this rate
      return *rate;
    }
  }
}
