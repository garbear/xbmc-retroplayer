/*
 *      Copyright (C) 2012-2014 Team XBMC
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

#include "addons/include/xbmc_game_types.h"
#include "cores/IPlayer.h"
#include "FileItem.h"
#include "games/GameClient.h"
#include "threads/Thread.h"

#include <stdint.h>
#include <string>

CRetroPlayerAudio;
CRetroPlayerVideo;

class CRetroPlayer : public IPlayer, public CThread
{
public:
  CRetroPlayer(IPlayerCallback& callback);
  virtual ~CRetroPlayer();

protected:
  virtual void Process();

private:
  /**
   * Dump game information (if any) to the debug log.
   */
  static void PrintGameInfo(const CFileItem &file) const;

  CRetroPlayerAudio*   m_audio; // Audio subsystem
  CRetroPlayerVideo*   m_video; // Video subsystem
  CFileItemPtr         m_file;
  GAME::GameClientPtr  m_gameClient;
  float                m_playSpeed; // Normal play speed is 1.0f
  CEvent               m_pauseEvent;
  CCriticalSection     m_critSection;
};
