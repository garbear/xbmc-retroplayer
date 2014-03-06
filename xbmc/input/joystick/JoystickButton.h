#pragma once
/*
 *      Copyright (C) 2007-2014 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <vector>

namespace INPUT
{

class CJoystick;
class IInputHandler;

/**
 * On/off button.
 */
class CJoystickButton
{
public:
  CJoystickButton(CJoystick* joystick, unsigned int id);
  virtual ~CJoystickButton() { }

  void Activate();
  void Deactivate();
  
  void Attach(IInputHandler* inputHandler);
  void Detach(IInputHandler* inputHandler);

private:
  enum BUTTON_STATE
  {
    BUTTON_DEPRESSED,
    BUTTON_PRESSED
  };

  CJoystick*                  m_joystick;
  const unsigned int          m_id;
  const unsigned int          m_index;
  BUTTON_STATE                m_state;
  std::vector<IInputHandler*> m_inputHandlers;
};

}
