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

#include "input/IKeyboardHandler.h"

class IJoystickDriverHandler;

/*!
 * \ingroup joysticks_generic
 *
 * \brief Generic implementation of IKeyboardHandler to handle a keyboard as a
 *        joystick with many buttons.
 */
class CGenericJoystickKeyboardHandler : public IKeyboardHandler
{
public:
  CGenericJoystickKeyboardHandler(IJoystickDriverHandler* handler);

  virtual ~CGenericJoystickKeyboardHandler(void) { }

  // implementation of IKeyboardHandler
  virtual bool OnKeyPress(const CKey& key);
  virtual void OnKeyRelease(const CKey& key);

private:
  static unsigned int GetButtonIndex(const CKey& key);

  IJoystickDriverHandler* const m_handler;
};
