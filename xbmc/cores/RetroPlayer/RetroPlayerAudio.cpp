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

#include "system.h"
#include "RetroPlayerAudio.h"
#include "cores/AudioEngine/AEFactory.h"
#include "cores/AudioEngine/Interfaces/AEStream.h"
#include "cores/AudioEngine/Utils/AEConvert.h"
#include "cores/dvdplayer/DVDClock.h"
#include "utils/log.h"

// Temporary - notify users about unsupported samplerate
#include "dialogs/GUIDialogOK.h"

#include <vector>

// Pre-convert audio to float to avoid additional buffer in AE stream
#define FRAMESIZE  (2 * sizeof(float)) // L (float) + R (float)

CRetroPlayerAudio::CRetroPlayerAudio()
  : CThread("RetroPlayerAudio"), m_pAudioStream(NULL), m_bSingleFrames(false),
    m_singleFrameSamples(0), m_bFlushSingleFrames(false)
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

  unsigned int samplerate = 0;

  // List comes from AESinkALSA.cpp
  static unsigned int sampleRateList[] = {5512, 8000, 11025, 16000, 22050, 32000, 44100, 48000, 0};
  for (unsigned int *rate = sampleRateList; ; rate++)
  {
    if (*(rate + 1) == 0)
    {
      // Reached the end of our list
      samplerate = *rate;
      break;
    }
    if ((unsigned int)allegedSamplerate < (*rate + *(rate + 1)) / 2)
    {
      // allegedSamplerate is between this rate and the next, so use this rate
      samplerate = *rate;
      break;
    }
  }

  CLog::Log(LOGINFO, "RetroPlayerAudio: Creating audio stream, sample rate hint = %u", samplerate);
  static enum AEChannel map[3] = {AE_CH_FL, AE_CH_FR, AE_CH_NULL};
  m_pAudioStream = CAEFactory::MakeStream(AE_FMT_FLOAT, samplerate, samplerate, CAEChannelInfo(map),
      AESTREAM_AUTOSTART | AESTREAM_LOW_LATENCY);

  if (!m_pAudioStream)
  {
    CLog::Log(LOGERROR, "RetroPlayerAudio: Failed to create audio stream");
    return 0;
  }
  else if (samplerate != m_pAudioStream->GetSampleRate())
  {
    // For real-time (ish) audio, we need to avoid resampling
    CLog::Log(LOGERROR, "RetroPlayerAudio: sink sample rate (%u) doesn't match", m_pAudioStream->GetSampleRate());
    // Temporary: Notify the user via GUI box
    CStdString msg;
    msg.Format("Sample rate not supported by audio device: %uHz", samplerate);
    CGUIDialogOK::ShowAndGetInput(257, msg.c_str(), "Continuing without sound", 0);
    return 0;
  }
  else
  {
    Create();
    return samplerate;
  }
}

void CRetroPlayerAudio::Process()
{
  if (!m_pAudioStream)
  {
    CLog::Log(LOGERROR, "RetroPlayerAudio: Process() called with no audio stream!");
    return;
  }

  AudioMultiBuffer::Window *window;

  while (!m_bStop)
  {
    {
      CSingleLock lock(m_critSection);

      // Greedy - try to keep m_buffer drained
      if (!(window = m_buffer.GetWindow()))
      {
        lock.Leave();
        m_packetReady.WaitMSec(17); // ~1 video frame @ 60fps
        // If event timed out, we might be done, so check m_bStop again
        continue;
      }
    }

    // Calculate a timeout when this definitely should be done
    double timeout;
    timeout  = DVD_SEC_TO_TIME(m_pAudioStream->GetDelay() + window->frames / m_pAudioStream->GetSampleRate());
    timeout += DVD_SEC_TO_TIME(1.0);
    timeout += CDVDClock::GetAbsoluteClock();

    // Keep track of how much data has been added to the stream
    unsigned char  *dataPtr = window->window.data();
    unsigned int   size     = window->frames * FRAMESIZE;
    unsigned int   copied;
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

    m_buffer.Free(window);
  }

  m_bStop = true;

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

  AudioMultiBuffer::Window *window = m_buffer.GetTheNextWindow();
  if (window)
  {
    if (window->window.size() < frames * FRAMESIZE)
      window->window.resize(frames * FRAMESIZE);

    // Batch-convert our audio samples to floats here using AE converters. We
    // do this to avoid conversion in AE, which reqiures another buffer and
    // another source of delay.
    uint8_t *source = reinterpret_cast<uint8_t*>(const_cast<int16_t*>(data));
    float   *dest   = reinterpret_cast<float*>(window->window.data());
    static CAEConvert::AEConvertToFn convertFn = CAEConvert::ToFloat(AE_FMT_S16NE);

    convertFn(source, frames * 2, dest); // L + R
    window->frames = frames;

    m_packetReady.Set();
  }
}

// TODO: Use parameters from RetroPlayer
// We don't want to flush intra-frame while the game is processing (game
// clients might use threads, for example). We also want to minimize
// m_singleFrameBuffer.
#define SAMPLERATE     32000
#define FPS            60
#define SAFETYFACTOR   4 // video frames
#define BUFFERSAMPLES  (SAMPLERATE * 2 * SAFETYFACTOR / FPS)

void CRetroPlayerAudio::SendAudioFrame(int16_t left, int16_t right)
{
  if (!m_bSingleFrames)
  {
    m_bSingleFrames = true;
    CLog::Log(LOGNOTICE, "RetroPlayer (Audio): Using single-frame audio");
    m_singleFrameBuffer.resize(BUFFERSAMPLES * sizeof(int16_t));
  }

  int16_t *dest = m_singleFrameBuffer.data() + m_singleFrameSamples;
  *dest = left;
  *(dest + 1) = right;

  m_singleFrameSamples += 2;

  if (m_bFlushSingleFrames || m_singleFrameSamples >= BUFFERSAMPLES)
  {
    m_bFlushSingleFrames = false;
    SendAudioFrames(m_singleFrameBuffer.data(), m_singleFrameSamples / 2);
    m_singleFrameSamples = 0;
  }
}

CRetroPlayerAudio::AudioMultiBuffer::Window *CRetroPlayerAudio::AudioMultiBuffer::GetWindow()
{
  // Return the window indicated by the m_pos index if it has frames
  if (windows[m_pos].frames != 0)
  {
    // Advance m_pos so that GetTheNextWindow() starts at the correct window
    m_pos = (m_pos + 1) % WINDOW_COUNT;
    return &windows[m_pos];
  }
  return NULL;
}

CRetroPlayerAudio::AudioMultiBuffer::Window *CRetroPlayerAudio::AudioMultiBuffer::GetTheNextWindow()
{
  // Start at the current window, and progress until we find one with no frames
  for (unsigned int i = m_pos; i < m_pos + WINDOW_COUNT; i++)
  {
    if (windows[i % WINDOW_COUNT].frames == 0)
      return &windows[i % WINDOW_COUNT];
  }
  return NULL;
}

void CRetroPlayerAudio::AudioMultiBuffer::ResizeAll(unsigned int size)
{
  // Start at the current window, and progress until we find one with no frames
  for (unsigned int i = 0; i < WINDOW_COUNT; i++)
    windows[i].window.resize(size);
}
