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

#include "JoystickTypes.h"

class CJoystickTranslator
{
public:
  static const char*  HatDirectionToString(HatDirection dir);

  static JoystickInputType GetInputType(JoystickFeatureID featureId);

  /*!
   * \brief Get the button key, as defined in guilib/Key.h, for the specified
   *        joystick feature/direction
   *
   * A direction vector of the feature's position can be used to obtain keys
   * for analog stick directions (e.g. "rightthumbstickup").
   *
   * Ties are resolved in the clockwise direction. A right thumb stick at (0.5, 0.5)
   * will resolve to "rightthumbstickright".
   *
   * \param id        The joystick feature ID
   * \param x         The x component of the direction vector being queried
   * \param y         The y component of the direction vector being queried
   * \param z         The z component of the direction vector being queried
   *
   * \return True if the event was handled otherwise false
   */
  static unsigned int GetButtonKeyID(JoystickFeatureID id, float x = 0.0f,
                                                           float y = 0.0f,
                                                           float z = 0.0f);
};
