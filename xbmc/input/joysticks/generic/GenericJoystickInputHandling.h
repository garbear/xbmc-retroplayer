/*
 *      Copyright (C) 2014-2015 Team XBMC
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
 * \ingroup joysticks_generic
 *
 * \brief Generic implementation of IJoystickDriverHandler to translate raw
 *        actions into physical joystick features using the provided button map
 */
class CGenericJoystickInputHandling : public IJoystickDriverHandler
{
public:
  CGenericJoystickInputHandling(IJoystickInputHandler* handler, IJoystickButtonMap* buttonMap);

  virtual ~CGenericJoystickInputHandling(void);

  // implementation of IJoystickDriverHandler
  virtual bool OnButtonMotion(unsigned int buttonIndex, bool bPressed);
  virtual bool OnHatMotion(unsigned int hatIndex, HatDirection direction);
  virtual bool OnAxisMotion(unsigned int axisIndex, float position);
  virtual void ProcessAxisMotions(void);

private:
  bool ProcessHatDirection(int index, HatDirection oldDir, HatDirection newDir, HatDirection targetDir);

  void OnPress(const std::string& feature);
  void OnRelease(const std::string& feature);

  void StartDigitalRepeating(const std::string& feature);
  void StopDigitalRepeating(const std::string& feature);

  float GetAxisState(int axisIndex) const;

  IJoystickInputHandler* const m_handler;
  IJoystickButtonMap* const    m_buttonMap;
  std::vector<char>            m_buttonStates; // std::vector is specialized for <bool>
  std::vector<HatDirection>    m_hatStates;
  std::vector<float>           m_axisStates;
  std::vector<std::string>     m_featuresWithMotion;
  std::vector<std::string>     m_repeatingFeatures;
};
