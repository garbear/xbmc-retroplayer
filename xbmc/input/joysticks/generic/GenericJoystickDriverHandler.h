/*
 *      Copyright (C) 2014 Team XBMC
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

#include "input/joysticks/IJoystickDriverHandler.h"
#include "input/joysticks/JoystickTypes.h"

#include <vector>

class IJoystickInputHandler;
class IJoystickButtonMap;

/*!
 * \brief Generic implementation of IJoystickDriverHandler to translate raw
 *        actions into physical joystick features using the provided button map
 */
class CGenericJoystickDriverHandler : public IJoystickDriverHandler
{
public:
  CGenericJoystickDriverHandler(IJoystickInputHandler* handler, IJoystickButtonMap* buttonMap);

  virtual ~CGenericJoystickDriverHandler(void);

  // implementation of IJoystickDriverHandler
  virtual bool OnButtonMotion(unsigned int buttonIndex, bool bPressed);
  virtual bool OnHatMotion(unsigned int hatIndex, HatDirection direction);
  virtual bool OnAxisMotion(unsigned int axisIndex, float position);
  virtual void ProcessAxisMotions(void);

private:
  bool ProcessHatDirection(int index, HatDirection oldDir, HatDirection newDir, HatDirection targetDir);

  float GetAxisState(int axisIndex) const;

  IJoystickInputHandler* const m_handler;
  IJoystickButtonMap* const    m_buttonMap;
  std::vector<char>            m_buttonStates; // std::vector is specialized for <bool>
  std::vector<HatDirection>    m_hatStates;
  std::vector<float>           m_axisStates;
  std::vector<unsigned int>    m_featuresWithMotion;
};
