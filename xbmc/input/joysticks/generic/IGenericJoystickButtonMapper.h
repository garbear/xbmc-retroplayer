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

#include "input/joysticks/IJoystickInputHandling.h"
#include "input/joysticks/JoystickTypes.h"

#include <string>

/*!
 * \ingroup joysticks_generic
 * \brief Interface defining methods to perform button mapping
 */
class IGenericJoystickButtonMapper : public IJoystickInputHandling
{
public:
  IGenericJoystickButtonMapper(IJoystickActionHandler *handler, const std::string &strName)
    : m_strJoystickName(strName)
  {
    RegisterHandler(handler);
  }

  virtual ~IGenericJoystickButtonMapper() { }

  /*!
   * \brief A button's value has changed
   *
   * \param id        The raw button ID
   * \param bPressed  The new value of the button
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnRawButtonPress(unsigned int id, bool bPressed) = 0;

  /*!
   * \brief A hat's value has changed
   *
   * \param id        The raw hat ID
   * \param direction The new value of the hat
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnRawHatMotion(unsigned int id, HatDirection direction) = 0;

  /*!
   * \brief A axis's value has changed
   *
   * \param id        The raw axis ID
   * \param position  The new value of the axis in the interval [-1, 1], inclusive
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnRawAxisMotion(unsigned int id, float position) = 0;

protected:
  /*!
   * \brief Name of the joystick
   */
  std::string m_strJoystickName;
};
