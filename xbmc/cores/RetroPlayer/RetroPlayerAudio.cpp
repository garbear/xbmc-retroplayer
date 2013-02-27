/*
 *      Copyright (C) 2012 Garrett Brown
 *      Copyright (C) 2012 Team XBMC
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

#include "system.h"
#include "RetroPlayerAudio.h"
#include "cores/AudioEngine/AEFactory.h"
#include "cores/AudioEngine/Interfaces/AEStream.h"
#include "cores/dvdplayer/DVDClock.h"
#include "utils/log.h"

CRetroPlayerAudio::CRetroPlayerAudio() : CThread("RetroPlayerAudio"), m_pAudioStream(NULL)
{
}

CRetroPlayerAudio::~CRetroPlayerAudio()
{
  StopThread();
}

unsigned int CRetroPlayerAudio::GoForth(double allegedSamplerate)
{
  if (m_pAudioStream)
    { CAEFactory::FreeStream(m_pAudioStream); m_pAudioStream = NULL; }

  // Reported sample rates look like 32040.5 or 31997.22. Round up to nearest
  // 10 or so, cast to int, and let AE figure out the most appropriate samplerate.
  const unsigned int roundupto = 10;
  const unsigned int samplerate = ((unsigned int)allegedSamplerate / roundupto + 1) * roundupto;

  CLog::Log(LOGINFO, "RetroPlayerAudio: Creating audio stream, sample rate hint = %u", samplerate);
  static enum AEChannel map[3] = {AE_CH_FL, AE_CH_FR, AE_CH_NULL};
  m_pAudioStream = CAEFactory::MakeStream(AE_FMT_S16NE, samplerate, samplerate, CAEChannelInfo(map), AESTREAM_AUTOSTART);

  if (!m_pAudioStream)
  {
    CLog::Log(LOGERROR, "RetroPlayerAudio: Failed to create audio stream");
    return 0;
  }
  else
  {
    Create();
    return m_pAudioStream->GetSampleRate();
  }
}

void CRetroPlayerAudio::Process()
{
  if (!m_pAudioStream)
  {
    CLog::Log(LOGERROR, "RetroPlayerAudio: Process() called with no audio stream!");
    return;
  }

  Packet packet = {};

  while (!m_bStop)
  {
    {
      CSingleLock lock(m_critSection);

      if (m_packets.empty())
      {
        lock.Leave();
        m_packetReady.WaitMSec(17); // ~1 video frame @ 60fps
        // If event timed out, we might be done, so check m_bStop again
        continue;
      }

      packet = m_packets.front();
      m_packets.pop();
    }

    // So we can clean up packet.data later
    unsigned char* data = packet.data;

    // Calculate some inherent properties of the sound data
    const DWORD  frameSize      = 2 * sizeof(uint16_t); // L (2) + R (2)
    const double secondsPerByte = 1.0 / (m_pAudioStream->GetSampleRate() * frameSize);

    // Calculate a timeout when this definitely should be done
    double timeout;
    timeout  = DVD_SEC_TO_TIME(m_pAudioStream->GetDelay() + packet.size * secondsPerByte);
    timeout += DVD_SEC_TO_TIME(1.0);
    timeout += CDVDClock::GetAbsoluteClock();

    // Keep track of how much data has been added to the stream
    DWORD copied;
    do
    {
      // Fast-forward packet data on successful add
      copied = m_pAudioStream->AddData(data, packet.size);
      data += copied;
      packet.size -= copied;

      // Test for incomplete frames remaining
      if (packet.size < frameSize)
        break;

      if (copied == 0 && timeout < CDVDClock::GetAbsoluteClock())
      {
        CLog::Log(LOGERROR, "RetroPlayerAudio: Timeout adding data to audio renderer");
        break;
      }

      Sleep(1);
    } while (!m_bStop);

    // Discard extra data
    if (packet.size > 0 && !m_bStop)
      CLog::Log(LOGNOTICE, "RetroPlayerAudio: %u bytes left over after rendering, discarding", (unsigned int)packet.size);

    // Clean up the data allocated in CRetroPlayer::OnAudioSampleBatch()
    delete[] packet.data;
    data = NULL;
  }

  m_bStop = true;
  { CAEFactory::FreeStream(m_pAudioStream); m_pAudioStream = NULL; }
}

unsigned int CRetroPlayerAudio::GetSampleRate() const
{
  return !m_bStop && m_pAudioStream ? m_pAudioStream->GetSampleRate() : 0;
}

double CRetroPlayerAudio::GetDelay() const
{
  return !m_bStop && m_pAudioStream ? m_pAudioStream->GetDelay() : 0.0;
}

void CRetroPlayerAudio::SendAudioFrames(const int16_t *data, size_t frames)
{
  CSingleLock lock(m_critSection);

  if (!m_bStop && IsRunning())
  {
    Packet packet;
    packet.size = frames * 2 * sizeof(int16_t);
    packet.data = new unsigned char[packet.size];

    if (packet.data)
    {
      memcpy(packet.data, data, packet.size);
      m_packets.push(packet);
      m_packetReady.Set();
    }
  } 
}
