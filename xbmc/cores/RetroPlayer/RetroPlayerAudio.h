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

#include "threads/Thread.h"

#include <boost/shared_array.hpp>
#include <queue>
#include <stdint.h>
#include <vector>

class IAEStream;

class CRetroPlayerAudio : public CThread
{
public:
  CRetroPlayerAudio();
  ~CRetroPlayerAudio();

  /**
   * Rev up the engines and start the thread.
   * @param  samplerate - the desired samplerate
   * @return the chosen samplerate, or 0 if failure
   */
  unsigned int GoForth(double samplerate);

  /**
   * Send audio samples to be processed by this class. Data format is:
   * int16_t buf[4] = { l, r, l, r }; this would be 2 frames.
   */
  void SendAudioFrames(const int16_t *data, size_t frames);

  /**
   * Send a single frame. Frames are cached until Flush() is called, so make
   * sure to call Flush() after every call to CGameClient::RunFrame().
   */
  void SendAudioFrame(int16_t left, int16_t right);

  /**
   * This function is only necessary if single frame audio is used, as
   * SendAudioFrames() sets the event after each multi-sample packet. If no
   * single frames have been observed (no call to SendAudioFrame()), this has
   * no effect.
   */
  void Flush() { m_bFlushSingleFrames = true; }

  /**
   * Accumulative audio delay. Does not include delay due to current packet, so
   * at 60fps this could be up to 17ms (~1/60) behind. Accuracy is also subject
   * to accuracy found in AE GetDelay() functions.
   */
  double GetDelay() const;

protected:
  virtual void Process();

private:
  struct Packet
  {
    boost::shared_array<uint8_t> data;
    size_t                       size;
  };

  /**
   * Given a desired samplerate, this will choose an appropriate sample rate
   * depending on the user's hardware.
   * @param  samplerate - the desired samplerate
   * @return the chosen samplerate
   */
  unsigned int SelectSampleRate(double samplerate);

  IAEStream            *m_pAudioStream;
  // Process() is greedy and will try to keep m_buffer drained
  std::queue<Packet>   m_packets;
  // Set to true if SendAudioFrame() is called
  bool                 m_bSingleFrames;
  std::vector<int16_t> m_singleFrameBuffer;
  // Count of samples in m_singleFrameBuffer
  unsigned int         m_singleFrameSamples;
  // Set to true if Flush() is called, reset inside SendAudioFrame()
  bool                 m_bFlushSingleFrames;
  CCriticalSection     m_critSection;
  CEvent               m_packetReady;
};
