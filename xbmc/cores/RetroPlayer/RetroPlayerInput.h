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

#include "games/libretro/libretro.h"
#include "guilib/Key.h"
#include "windowing/XBMC_events.h"
#include "threads/Thread.h"

#include <stdint.h>

class CRetroPlayerInput
{
public:
  CRetroPlayerInput();
  ~CRetroPlayerInput() { Finish(); }

  void Begin();
  void Finish();

  /**
   * Called by the game client to query gamepad states.
   * \param port   The player #. Player 1 is port 0.
   * \param device The fundamental device abstraction. This can be changed from
   *               the default by calling CGameClient::SetDevice().
   * \param index  Only used for analog devices (RETRO_DEVICE_ANALOG)
   * \param id     The button ID being queried.
   * \return       Although this returns int16_t, it seems game clients cast
   *               to bool. Therefore, 0 = not pressed, 1 = pressed.
   */
  int16_t GetInput(unsigned port, unsigned device, unsigned index, unsigned id);

  /**
   * Called by CApplication with an action containing the ID of the button
   * pressed. Note: the ID needs to be translated before modifying the state
   * structure.
   */
  void OnAction(const CAction &action);

  /**
   * Marks a key as released.
   */
  void ProcessKeyUp(XBMC_Event &newEvent);

  /**
   * Called by CApplication when a key is released, and forwards said key to
   * ProcessKeyUp(). Precondition: Begin() must have been called.
   */
  static void OnKeyUp(XBMC_Event &newEvent);

  static CRetroPlayerInput *m_self;

private:
  int TranslateActionID(int id) const;

  // RETRO_DEVICE_ID_JOYPAD_R3 is the last key in libretro.h
  int16_t m_joypadState[ACTION_JOYPAD_CONTROL_END - ACTION_GAME_CONTROL_START + 1];
  bool    m_bActive;

  CCriticalSection m_statesGuard;
};
