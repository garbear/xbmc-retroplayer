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

#include "JoystickTranslator.h"

const char* CJoystickTranslator::HatDirectionToString(HatDirection dir)
{
  switch (dir)
  {
    case HatDirectionLeft:      return "LEFT";
    case HatDirectionRight:     return "RIGHT";
    case HatDirectionUp:        return "UP";
    case HatDirectionDown:      return "DOWN";
    case HatDirectionLeftUp:    return "UP LEFT";
    case HatDirectionLeftDown:  return "DOWN LEFT";
    case HatDirectionRightUp:   return "UP RIGHT";
    case HatDirectionRightDown: return "DOWN RIGHT";
    case HatDirectionNone:
    default:
      break;
  }
  return "RELEASED";
}

SemiAxisDirection CJoystickTranslator::PositionToSemiAxisDirection(float position)
{
  if      (position > 0) return SemiAxisDirectionPositive;
  else if (position < 0) return SemiAxisDirectionNegative;

  return SemiAxisDirectionUnknown;
}

CardinalDirection CJoystickTranslator::VectorToCardinalDirection(float x, float y)
{
  if      (y >= x && y >  -x) return DirectionUp;
  else if (y <  x && y >= -x) return DirectionRight;
  else if (y <= x && y <  -x) return DirectionDown;
  else if (y >  x && y <= -x) return DirectionLeft;

  return DirectionUnknown;
}
