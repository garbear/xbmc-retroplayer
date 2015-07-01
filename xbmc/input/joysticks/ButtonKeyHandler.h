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
#include "threads/CriticalSection.h"
#include "threads/Event.h"
#include "threads/Thread.h"

#include <vector>

class CButtonKeyHandler : public IButtonKeyHandler, protected CThread
{
public:
  CButtonKeyHandler(void);

  virtual ~CButtonKeyHandler(void);

  // implementation of IButtonKeyHandler
  virtual InputType GetInputType(unsigned int buttonKeyId) const;
  virtual void OnDigitalButtonKey(unsigned int buttonKeyId, bool bPressed);
  virtual void OnAnalogButtonKey(unsigned int buttonKeyId, float magnitude);

protected:
  // implementation of CThread
  virtual void Process(void);

private:
  enum BUTTON_STATE
  {
    STATE_UNPRESSED,
    STATE_BUTTON_PRESSED,
    STATE_BUTTON_HELD,
  };

  bool ProcessButtonPress(unsigned int buttonKeyId);
  void ProcessButtonRelease(unsigned int buttonKeyId);
  bool IsHeld(unsigned int buttonKeyId) const;

  static bool SendDigitalAction(unsigned int buttonKeyId, unsigned int holdTimeMs = 0);
  static bool SendAnalogAction(unsigned int buttonKeyId, float magnitude);

  BUTTON_STATE              m_state;
  unsigned int              m_lastButtonPress;
  std::vector<unsigned int> m_pressedButtons;
  CEvent                    m_pressEvent;
  CCriticalSection          m_digitalMutex;
};
