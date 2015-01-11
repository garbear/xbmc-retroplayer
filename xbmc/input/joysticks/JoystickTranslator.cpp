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

JoystickFeatureType CJoystickTranslator::GetFeatureType(JoystickFeatureID featureId)
{
  switch (featureId)
  {
    case JoystickIDButtonA:           return JoystickDigitalButton;
    case JoystickIDButtonB:           return JoystickDigitalButton;
    case JoystickIDButtonX:           return JoystickDigitalButton;
    case JoystickIDButtonY:           return JoystickDigitalButton;
    case JoystickIDButtonStart:       return JoystickDigitalButton;
    case JoystickIDButtonBack:        return JoystickDigitalButton;
    case JoystickIDButtonGuide:       return JoystickDigitalButton;
    case JoystickIDButtonLeftBumper:  return JoystickDigitalButton;
    case JoystickIDButtonRightBumper: return JoystickDigitalButton;
    case JoystickIDButtonLeftStick:   return JoystickDigitalButton;
    case JoystickIDButtonRightStick:  return JoystickDigitalButton;
    case JoystickIDButtonUp:          return JoystickDigitalButton;
    case JoystickIDButtonRight:       return JoystickDigitalButton;
    case JoystickIDButtonDown:        return JoystickDigitalButton;
    case JoystickIDButtonLeft:        return JoystickDigitalButton;
    case JoystickIDTriggerLeft:       return JoystickAnalogButton;
    case JoystickIDTriggerRight:      return JoystickAnalogButton;
    case JoystickIDAnalogStickLeft:   return JoystickAnalogStick;
    case JoystickIDAnalogStickRight:  return JoystickAnalogStick;
    case JoystickIDAccelerometer:     return JoystickAccelerometer;
    default:
      break;
  }
  return JoystickDigitalButton;
}
