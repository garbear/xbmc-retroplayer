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
#pragma once

#include "threads/Thread.h"
#include "utils/RingBuffer.h"

#include <stdint.h>

class IAEStream;

class CRetroPlayerAudio : public CThread
{
public:
  CRetroPlayerAudio();
  ~CRetroPlayerAudio();

  /**
   * Rev up the engines and start the thread.
   */
  unsigned int GoForth(double allegedSamplerate, double framerate);

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
  void Flush() { if (m_bSingleFrames) m_packetReady.Set(); }

  /**
   * Accumulative audio delay. Does not include delay due to current packet, so
   * at 60fps this could be up to 17ms (~1/60) behind.
   */
  double GetDelay() const;

protected:
  virtual void Process();

private:
  IAEStream          *m_pAudioStream;
  CRingBuffer        m_buffer;
  bool               m_bSingleFrames;
  CCriticalSection   m_critSection;
  CEvent             m_packetReady;
};
