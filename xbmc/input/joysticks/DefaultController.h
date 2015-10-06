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

#include "input/joysticks/IJoystickInputHandler.h"
#include "input/joysticks/JoystickTypes.h"

#include <vector>

class IButtonKeyHandler;
class IButtonSequence;

/*!
 * \ingroup joysticks
 *
 * \brief Implementation of IJoystickInputHandler for the default controller
 *
 * \sa IJoystickInputHandler
 */
class CDefaultController : public IJoystickInputHandler
{
public:
  CDefaultController(void);

  virtual ~CDefaultController(void);

  // implementation of IJoystickInputHandler
  virtual std::string ControllerID(void) const;
  virtual InputType GetInputType(const JoystickFeature& feature) const;
  virtual bool OnButtonPress(const JoystickFeature& feature, bool bPressed);
  virtual bool OnButtonMotion(const JoystickFeature& feature, float magnitude);
  virtual bool OnAnalogStickMotion(const JoystickFeature& feature, float x, float y);
  virtual bool OnAccelerometerMotion(const JoystickFeature& feature, float x, float y, float z);

private:
  /*!
   * \brief Get the button key, as defined in guilib/Key.h, for the specified
   *        joystick feature/direction
   *
   * \return The button key ID
   */
  static unsigned int GetButtonKeyID(const JoystickFeature& feature, CardinalDirection dir = DirectionUnknown);

  /*!
   * \brief Return a vector of the four cardinal directions
   */
  static const std::vector<CardinalDirection>& GetDirections(void);

  IButtonKeyHandler* const m_handler;
  IButtonSequence*         m_easterEgg;
};
