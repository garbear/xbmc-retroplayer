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
#pragma once

#include "cores/AudioEngine/Utils/AEChannelData.h"

#include <stdint.h>

class IAEStream;

class CRetroPlayerAudio
{
public:
  CRetroPlayerAudio(void);
  ~CRetroPlayerAudio(void) { Cleanup(); }

  bool Start(AEDataFormat format, double samplerate);
  void Stop(void) { Cleanup(); }

  unsigned int AudioFrames(AEDataFormat format, unsigned int frames, const uint8_t* data);

  /**
   * Accumulative audio delay. Does not include delay due to current packet, so
   * at 60fps this could be up to 17ms (~1/60) behind. Accuracy is also subject
   * to accuracy found in AE GetDelay() functions.
   */
  double GetDelay() const;

  unsigned int GetSampleRate() const;

private:
  void Cleanup(void);

  /**
   * Given a desired samplerate, this will choose an appropriate sample rate
   * depending on the user's hardware.
   * @param  samplerate - the desired samplerate
   * @return the chosen samplerate
   */
  static unsigned int FindSampleRate(double samplerate);

  IAEStream* m_pAudioStream;
};
