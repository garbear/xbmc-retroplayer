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
   */
  unsigned int GoForth(double allegedSamplerate);

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
  /**
   * Operate several std::vector buffers in a ring-like fashion. This avoids
   * the problem of blocking on a single buffer, and also allows the buffer
   * queue to get backed up several video frames.
   */
  class AudioMultiBuffer
  {
  public:
    AudioMultiBuffer() : m_pos(0) { }
    // A Window is a collection of audio data, usually corresponding to a frame of video
    struct Window
    {
      Window() : frames(0) { }
      std::vector<unsigned char> window;
      // Number of audio frames in the window
      unsigned int frames;
    };
    // Return the queued buffer containing the most recent data. Null if no data
    Window *GetWindow();
    // Return the next unused buffer. Null if AudioMultiBuffer is full
    Window *GetTheNextWindow(); // GetNextWindow() is reserved on Windows
    // Set the size of all buffers
    void ResizeAll(unsigned int size);
    // Delcare a buffer unused
    void Free(Window *window) { window->frames = 0; }

  private:
    // If buffer gets backed up, start dropping audio after 4 windows
    static const unsigned int WINDOW_COUNT = 4;
    Window windows[WINDOW_COUNT];
    // Index of the current window to be returned by GetWindow()
    int m_pos;
  };

  IAEStream            *m_pAudioStream;
  // Process() is greedy and will try to keep m_buffer drained
  AudioMultiBuffer     m_buffer;
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
