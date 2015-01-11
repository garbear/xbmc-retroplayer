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

/*!
 * \ingroup joysticks
 * \brief Cardinal directions
 */
enum CardinalDirection
{
  DirectionUnknown,
  DirectionUp,
  DirectionDown,
  DirectionRight,
  DirectionLeft,
};

/*!
 * \ingroup joysticks
 * \brief States in which a hat (directional pad) can be
 */
enum HatDirection
{
  HatDirectionNone      = 0x0,    /*!< @brief no directions are pressed */
  HatDirectionUp        = 0x1,    /*!< @brief only up is pressed */
  HatDirectionDown      = 0x2,    /*!< @brief only down is pressed */
  HatDirectionRight     = 0x4,    /*!< @brief only right is pressed */
  HatDirectionLeft      = 0x8,    /*!< @brief only left is pressed */
  HatDirectionLeftUp    = HatDirectionLeft  | HatDirectionUp,
  HatDirectionLeftDown  = HatDirectionLeft  | HatDirectionDown,
  HatDirectionRightUp   = HatDirectionRight | HatDirectionUp,
  HatDirectionRightDown = HatDirectionRight | HatDirectionDown,
};

/*!
 * \ingroup joysticks
 * \brief Directions in which a semiaxis can point
 */
enum SemiAxisDirection
{
  SemiAxisDirectionNegative = -1,  // semiaxis lies in the interval [-1.0, 0.0]
  SemiAxisDirectionUnknown  =  0,  // semiaxis is unknown or invalid
  SemiAxisDirectionPositive =  1,  // semiaxis lies in the interval [0.0, 1.0]
};
