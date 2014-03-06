#pragma once
/*
 *      Copyright (C) 2007-2013 Team XBMC
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

#include "JoystickButton.h"
#include "JoystickHat.h"
#include "JoystickAxis.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#define JACTIVE_NONE            0x00000000
#define JACTIVE_BUTTON          0x00000001
#define JACTIVE_HAT             0x00000002
#define JACTIVE_AXIS            0x00000004
#define JACTIVE_HAT_UP          0x01
#define JACTIVE_HAT_RIGHT       0x02
#define JACTIVE_HAT_DOWN        0x04
#define JACTIVE_HAT_LEFT        0x08

#define GAMEPAD_BUTTON_COUNT    32
#define GAMEPAD_HAT_COUNT       4
#define GAMEPAD_AXIS_COUNT      6

class CAction;

namespace INPUT
{

class CJoystick;
typedef boost::shared_ptr<CJoystick> JoystickPtr;
typedef std::vector<JoystickPtr>     JoystickArray;

class CJoystick
{
protected:
  CJoystick(const std::string& strName, unsigned int id, unsigned int buttonCount, unsigned int hatCount, unsigned int axisCount);

public:
  virtual ~CJoystick();

  virtual void Update() = 0;

  unsigned int GetID() const { return m_id; }
  void SetID(unsigned int id) { m_id = id; }

  const std::string& GetName() const { return m_name; }

private:
  static bool IsGameControl(int actionID);

  const std::string             m_name;
  unsigned int                  m_id;
  std::vector<CJoystickButton*> m_buttons;
  std::vector<CJoystickHat*>    m_hats;
  std::vector<CJoystickAxis*>   m_axes;
};

}
