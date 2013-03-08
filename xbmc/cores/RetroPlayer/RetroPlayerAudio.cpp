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

#include <vector>

#define FRAMESIZE (2 * sizeof(uint16_t)) // L (2) + R (2)
#define BUFFER_VIDEO_FRAMES 10 // Buffer can hold audio for 10 video frames max

CRetroPlayerAudio::CRetroPlayerAudio()
  : CThread("RetroPlayerAudio"), m_pAudioStream(NULL), m_bSingleFrames(false)
{
}

CRetroPlayerAudio::~CRetroPlayerAudio()
{
  StopThread();
}

unsigned int CRetroPlayerAudio::GoForth(double allegedSamplerate, double framerate)
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
    // Create a ring buffer of 10 fames to hold our audio packets. We should
    // dispatch a packet every frame, but this gives us some room to overflow.
    const double frameTime = 1.0 / (framerate * m_pAudioStream->GetSampleRate() / allegedSamplerate);
    const unsigned int bufferSize = (unsigned int)(BUFFER_VIDEO_FRAMES * frameTime * m_pAudioStream->GetSampleRate() * FRAMESIZE);

    m_buffer.Create(bufferSize);

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

  std::vector<unsigned char> data; // data.size() is max buffer size
  unsigned int size; // current packet size

  while (!m_bStop)
  {
    {
      CSingleLock lock(m_critSection);

      size = m_buffer.getMaxReadSize();
      if (size == 0)
      {
        lock.Leave();
        m_packetReady.WaitMSec(17); // ~1 video frame @ 60fps
        // If event timed out, we might be done, so check m_bStop again
        continue;
      }

      // Ensure we have enough space by resizing to the largest data packet observed
      if (data.size() < size)
        data.resize(size);
      m_buffer.ReadData(reinterpret_cast<char*>(data.data()), size);
    }

    unsigned char* dataPtr = data.data();

    // Calculate a timeout when this definitely should be done
    double timeout;
    const double secondsPerByte = 1.0 / (m_pAudioStream->GetSampleRate() * FRAMESIZE);
    timeout  = DVD_SEC_TO_TIME(m_pAudioStream->GetDelay() + size * secondsPerByte);
    timeout += DVD_SEC_TO_TIME(1.0);
    timeout += CDVDClock::GetAbsoluteClock();

    // Keep track of how much data has been added to the stream
    unsigned int copied;
    do
    {
      // Fast-forward packet data on successful add
      copied = m_pAudioStream->AddData(dataPtr, size);
      dataPtr += copied;
      size -= copied;

      // Test for incomplete frames remaining
      if (size < FRAMESIZE)
        break;

      if (copied == 0 && timeout < CDVDClock::GetAbsoluteClock())
      {
        CLog::Log(LOGERROR, "RetroPlayerAudio: Timeout adding data to audio renderer");
        break;
      }

      Sleep(1);
    } while (!m_bStop);

    // Discard extra data
    if (size > 0 && !m_bStop)
      CLog::Log(LOGNOTICE, "RetroPlayerAudio: %u bytes left over after rendering, discarding", size);
  }

  {
    CSingleLock lock(m_critSection);
    m_bStop = true;
    m_buffer.Destroy();
  }
  CAEFactory::FreeStream(m_pAudioStream);
  m_pAudioStream = NULL;
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
    m_buffer.WriteData(reinterpret_cast<const char*>(data), frames * FRAMESIZE);
    m_packetReady.Set();
  }
}

void CRetroPlayerAudio::SendAudioFrame(int16_t left, int16_t right)
{
  CSingleLock lock(m_critSection);

  if (!m_bSingleFrames)
    m_bSingleFrames = true;

  if (!m_bStop && IsRunning())
  {
    m_buffer.WriteData(reinterpret_cast<const char*>(&left), sizeof(left));
    m_buffer.WriteData(reinterpret_cast<const char*>(&right), sizeof(right));
    // m_packetReady.Set() is called in Flush()
  } 
}
