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

CRetroPlayerAudio::CRetroPlayerAudio()
  : CThread("RetroPlayerAudio"),
    m_samplerate(0),
    m_bPaused(false)
{
}

CRetroPlayerAudio::~CRetroPlayerAudio()
{
  StopThread();
}

void CRetroPlayerAudio::GoForth(int samplerate)
{
  m_samplerate = samplerate;
  Create();
}

void CRetroPlayerAudio::Process()
{
  CLog::Log(LOGINFO, "RetroPlayerAudio: Creating audio stream, sample rate = %d", m_samplerate);
  static enum AEChannel map[3] = {AE_CH_FL, AE_CH_FR, AE_CH_NULL};
  IAEStream *audioStream = CAEFactory::MakeStream(AE_FMT_S16NE, m_samplerate, m_samplerate,
      CAEChannelInfo(map), AESTREAM_AUTOSTART);

  if (!audioStream)
  {
    CLog::Log(LOGERROR, "RetroPlayerAudio: Failed to create audio stream");
    return;
  }

  Packet packet;

  while (!m_bStop)
  {
    /*
    if (m_bPaused)
    {
      audioStream->Pause();
      m_pauseEvent.Wait();
      if (m_bStop)
        break;
      audioStream->Resume();
    }
    */

    { // Enter critical section
      CSingleLock lock(m_critSection);
      if (!m_packets.size())
      {
        lock.Leave();
        m_packetReady.WaitMSec(17); // ~1 video frame @ 60fps
        // If event timed out, we might be done, so check m_bStop again
        continue;
      }
      packet = m_packets.front();
      m_packets.pop();
    } // End critical section

    unsigned char* data = reinterpret_cast<unsigned char*>(const_cast<int16_t*>(packet.data));
    DWORD frameSize = 2 * sizeof(uint16_t); // L (2) + R (2)
    DWORD size = packet.frames * frameSize;
    double secondsPerByte = 1.0 / (m_samplerate * frameSize);

    // Calculate a timeout when this definitely should be done
    double timeout;
    timeout  = DVD_SEC_TO_TIME(audioStream->GetDelay() + size * secondsPerByte);
    timeout += DVD_SEC_TO_TIME(1.0);
    timeout += CDVDClock::GetAbsoluteClock();

    DWORD  copied;
    do
    {
      copied = audioStream->AddData(data, size);
      data += copied;
      size -= copied;

      if (size < frameSize)
        break;

      if (copied == 0 && timeout < CDVDClock::GetAbsoluteClock())
      {
        CLog::Log(LOGERROR, "RetroPlayerAudio: Timeout adding data to renderer");
        break;
      }

      Sleep(1);
    } while (!m_bStop);

    // if we have more data left, save it for the next call to this function
    if (size > 0 && !m_bStop)
      CLog::Log(LOGNOTICE, "RetroPlayerAudio: %d bytes left over after rendering, discarding", size);

    // Clean up the data allocated in CRetroPlayer::OnAudioSampleBatch()
    delete[] packet.data;
  }

  if (audioStream)
    CAEFactory::FreeStream(audioStream);
}

void CRetroPlayerAudio::SendAudioFrames(const int16_t *data, size_t frames)
{
  if (IsRunning())
  {
    CSingleLock lock(m_critSection);
    m_packets.push(Packet(data, frames));
    m_packetReady.Set();
  }
  else
  {
    // Packets were discarded, so make sure we clean up after CRetroPlayer::OnAudioSampleBatch()
    delete[] data;
  }
}
