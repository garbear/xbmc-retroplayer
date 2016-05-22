/*
 *      Copyright (C) 2016 Team Kodi
 *      Copyright (C) 2016 Valve Corporation
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "cores/AudioEngine/Interfaces/AESink.h"

#define STEAM_LINK_SINK_NAME  "SteamLinkAudio"

namespace STEAMLINK
{

class CAESinkSteamLink : public IAESink
{
public:
  virtual const char* GetName() override { return STEAM_LINK_SINK_NAME; }

  CAESinkSteamLink();
  virtual ~CAESinkSteamLink();

  // implementation of IAESink
  virtual bool Initialize(AEAudioFormat &format, std::string &device) override;
  virtual void Deinitialize() override;
  virtual double GetCacheTotal() override;
  virtual unsigned int AddPackets(uint8_t **data, unsigned int frames, unsigned int offset) override;
  virtual void GetDelay(AEDelayStatus &status) override;
  virtual void Drain() override;

  static void EnumerateDevicesEx(AEDeviceInfoList &deviceInfoList);

private:
  // AE stuff
  AEAudioFormat m_format;

  // Steam Link stuff
  void* m_context;
  void* m_stream;
};

}
