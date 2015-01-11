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
  /*!
   * \brief Translate a hat direction to a string representation
   *
   * \param dir The hat direction
   *
   * \return A capitalized string representation, or "RELEASED" if the hat is centered.
   */
  static const char* HatDirectionToString(HatDirection dir);

  /*!
   * \brief Get the semi-axis direction containing the specified position
   *
   * \param position The position
   *
   * \return Positive, negative, or unknown if position is 0
   */
  static SemiAxisDirection PositionToSemiAxisDirection(float position);

  /*!
   * \brief Get the closest cardinal direction to the given vector
   *
   * Ties are resolved in the clockwise direction: (0.5, 0.5) will resolve to
   * Right.
   *
   * \param x  The x component of the vector
   * \param y  The y component of the vector
   *
   * \return The closest cardinal directon (up, down, right or left), or unknown
   *         if x and y are both 0.
   */
  static CardinalDirection VectorToCardinalDirection(float x, float y);
};
