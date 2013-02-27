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

#include <stdint.h>
#include <queue>

class IAEStream;

class CRetroPlayerAudio : public CThread
{
public:
  struct Packet
  {
    unsigned char * data;
    size_t          size; // Frame count * 2 (L+R) * sizeof(int16_t)
  };

  CRetroPlayerAudio();
  ~CRetroPlayerAudio();

  /**
   * Rev up the engines and start the thread.
   */
  unsigned int GoForth(double allegedSamplerate);

  /**
   * Send audio samples to be processed by this class. Data format is:
   * int16_t buf[4] = { l, r, l, r }; this would be 2 frames.
   */
  void SendAudioFrames(const int16_t *data, size_t frames);
  //void SendAudioFrames(int16_t left, size_t right);

  unsigned int GetSampleRate() const;
  double GetDelay() const;

protected:
  virtual void Process();

private:
  IAEStream          *m_pAudioStream;
  std::queue<Packet> m_packets;
  CCriticalSection   m_critSection;
  CEvent             m_streamReady;
  CEvent             m_packetReady;
  CEvent             m_pauseEvent;
};
