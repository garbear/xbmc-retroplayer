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

class CRetroPlayerAudio : public CThread
{
public:
  CRetroPlayerAudio();
  ~CRetroPlayerAudio();

  struct Packet
  {
    const int16_t *data;
    size_t         frames;
    Packet() { data = NULL; frames = 0; }
    Packet(const int16_t *data, size_t frames) : data(data), frames(frames) { }
    Packet(const Packet &frame) { *this = frame; }
    Packet &operator=(const Packet &packet)
    {
      if (this != &packet) { data = packet.data; frames = packet.frames; }
      return *this;
    }
  };

  /**
   * Rev up the engines and start the thread.
   */
  void GoForth(int samplerate);

  /**
   * Send audio samples to be processed by this class. Data format is:
   * int16_t buf[4] = { l, r, l, r }; this would be 2 frames.
   */
  void SendAudioFrames(const int16_t *data, size_t frames);

  void Tickle() { m_packetReady.Set(); }

  void Pause() { m_bPaused = true; }
  void UnPause() { m_bPaused = false; m_pauseEvent.Set(); }

protected:
  virtual void Process();
  //virtual void OnExit();

private:
  std::queue<Packet> m_packets;
  CEvent             m_packetReady;
  CEvent             m_pauseEvent;
  CCriticalSection   m_critSection;
  int                m_samplerate;
  bool               m_bPaused;
};
