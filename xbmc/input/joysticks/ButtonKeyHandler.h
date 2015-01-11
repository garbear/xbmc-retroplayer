/*
 *      Copyright (C) 2015 Team XBMC
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

#include "input/joysticks/IButtonKeyHandler.h"
#include "threads/Timer.h"

#include <vector>

class CButtonKeyHandler : public IButtonKeyHandler, public ITimerCallback
{
public:
  CButtonKeyHandler(void);

  virtual ~CButtonKeyHandler(void) { }

  // implementation of IButtonKeyHandler
  virtual InputType GetInputType(unsigned int buttonKeyId) const;
  virtual void OnDigitalButtonKey(unsigned int buttonKeyId, bool bPressed);
  virtual void OnAnalogButtonKey(unsigned int buttonKeyId, float magnitude);

  // implementation of ITimerCallback
  virtual void OnTimeout(void);

private:
  bool ProcessButtonPress(unsigned int buttonKeyId);
  void ProcessButtonRelease(unsigned int buttonKeyId);
  bool IsHeld(unsigned int buttonKeyId) const;

  void StartHoldTimer(unsigned int buttonKeyId);
  void ClearHoldTimer(void);

  bool SendDigitalAction(unsigned int buttonKeyId, unsigned int holdTimeMs = 0);
  bool SendAnalogAction(unsigned int buttonKeyId, float magnitude);

  CTimer                    m_holdTimer; // TODO: This creates a new thread every button press!
  unsigned int              m_lastButtonPress;
  std::vector<unsigned int> m_pressedButtons;
};
