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

#include "AddonJoystickButtonMap.h"

using namespace PERIPHERALS;

CAddonJoystickButtonMap::CAddonJoystickButtonMap(const PERIPHERALS::PeripheralAddonPtr& addon, unsigned int index)
 : m_addon(addon),
   m_index(index)
{
}

bool CAddonJoystickButtonMap::Load(void)
{
  if (!m_addon->GetJoystickFeatures(m_index, m_features))
    return false;

  std::map<CJoystickDriverPrimitive, JoystickFeatureID> actionMap = GetActionMap(m_features);
  m_actions.swap(actionMap);

  return true;
}

std::map<CJoystickDriverPrimitive, JoystickFeatureID> CAddonJoystickButtonMap::GetActionMap(const JoystickFeatureMap& features)
{
  std::map<CJoystickDriverPrimitive, JoystickFeatureID> actionMap;

  for (JoystickFeatureMap::const_iterator it = features.begin(); it != features.end(); ++it)
  {
    const JoystickFeatureID              id      = it->first;
    const ADDON::JoystickFeature* const feature = it->second.get();

    switch (feature->Type())
    {
    case JOYSTICK_DRIVER_TYPE_BUTTON:
    {
      const ADDON::DriverButton* button = static_cast<const ADDON::DriverButton*>(feature);
      actionMap[CJoystickDriverPrimitive(button->Index())] = id;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
    {
      const ADDON::DriverHat* hat = static_cast<const ADDON::DriverHat*>(feature);
      actionMap[CJoystickDriverPrimitive(hat->Index(), ToHatDirection(hat->Direction()))] = id;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
    {
      const ADDON::DriverSemiAxis* semiaxis = static_cast<const ADDON::DriverSemiAxis*>(feature);
      actionMap[CJoystickDriverPrimitive(semiaxis->Index(), ToSemiAxisDirection(semiaxis->Direction()))] = id;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
    {
      const ADDON::DriverAnalogStick* analogStick = static_cast<const ADDON::DriverAnalogStick*>(feature);
      actionMap[CJoystickDriverPrimitive(analogStick->XIndex(), SemiAxisDirectionPositive)] = id;
      actionMap[CJoystickDriverPrimitive(analogStick->XIndex(), SemiAxisDirectionNegative)] = id;
      actionMap[CJoystickDriverPrimitive(analogStick->YIndex(), SemiAxisDirectionPositive)] = id;
      actionMap[CJoystickDriverPrimitive(analogStick->YIndex(), SemiAxisDirectionNegative)] = id;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
    {
      const ADDON::DriverAccelerometer* accelerometer = static_cast<const ADDON::DriverAccelerometer*>(feature);
      actionMap[CJoystickDriverPrimitive(accelerometer->XIndex(), SemiAxisDirectionPositive)] = id;
      actionMap[CJoystickDriverPrimitive(accelerometer->XIndex(), SemiAxisDirectionNegative)] = id;
      actionMap[CJoystickDriverPrimitive(accelerometer->YIndex(), SemiAxisDirectionPositive)] = id;
      actionMap[CJoystickDriverPrimitive(accelerometer->YIndex(), SemiAxisDirectionNegative)] = id;
      actionMap[CJoystickDriverPrimitive(accelerometer->ZIndex(), SemiAxisDirectionPositive)] = id;
      actionMap[CJoystickDriverPrimitive(accelerometer->ZIndex(), SemiAxisDirectionNegative)] = id;
      break;
    }

    default:
      break;
    }
  }

  return actionMap;
}

JoystickFeatureID CAddonJoystickButtonMap::GetFeature(const CJoystickDriverPrimitive& source)
{
  JoystickFeatureID id = JoystickIDButtonUnknown;

  std::map<CJoystickDriverPrimitive, JoystickFeatureID>::const_iterator it = m_actions.find(source);
  if (it != m_actions.end())
    id = it->second;

  return id;
}

bool CAddonJoystickButtonMap::GetDriverPrimitive(JoystickFeatureID id, CJoystickDriverPrimitive& button)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(id);
  if (it != m_features.end())
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    switch (feature->Type())
    {
    case JOYSTICK_DRIVER_TYPE_BUTTON:
    {
      const ADDON::DriverButton* driverButton = static_cast<const ADDON::DriverButton*>(feature);
      button = CJoystickDriverPrimitive(driverButton->Index());
      retVal = true;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
    {
      const ADDON::DriverHat* driverHat = static_cast<const ADDON::DriverHat*>(feature);
      const HatDirection dir = ToHatDirection(driverHat->Direction());
      button = CJoystickDriverPrimitive(driverHat->Index(), dir);
      retVal = true;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
    {
      const ADDON::DriverSemiAxis* driverSemiAxis = static_cast<const ADDON::DriverSemiAxis*>(feature);
      const SemiAxisDirection dir = ToSemiAxisDirection(driverSemiAxis->Direction());
      button = CJoystickDriverPrimitive(driverSemiAxis->Index(), dir);
      retVal = true;
      break;
    }

    default:
      break;
    }
  }

  return retVal;
}

bool CAddonJoystickButtonMap::GetAnalogStick(JoystickFeatureID id, 
    int& horizIndex, bool& horizInverted,
    int& vertIndex,  bool& vertInverted)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(id);
  if (it != m_features.end())
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    switch (feature->Type())
    {
    case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
    {
      const ADDON::DriverAnalogStick* driverAnalogStick = static_cast<const ADDON::DriverAnalogStick*>(feature);
      horizIndex    = driverAnalogStick->XIndex();
      horizInverted = driverAnalogStick->XInverted();
      vertIndex     = driverAnalogStick->YIndex();
      vertInverted  = driverAnalogStick->YInverted();
      retVal        = true;
      break;
    }

    default:
      break;
    }
  }

  return retVal;
}

bool CAddonJoystickButtonMap::GetAccelerometer(JoystickFeatureID id,
    int& xIndex, bool& xInverted,
    int& yIndex, bool& yInverted,
    int& zIndex, bool& zInverted)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(id);
  if (it != m_features.end())
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    switch (feature->Type())
    {
    case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
    {
      const ADDON::DriverAccelerometer* driverAccelerometer = static_cast<const ADDON::DriverAccelerometer*>(feature);
      xIndex    = driverAccelerometer->XIndex();
      xInverted = driverAccelerometer->XInverted();
      yIndex    = driverAccelerometer->YIndex();
      yInverted = driverAccelerometer->YInverted();
      zIndex    = driverAccelerometer->ZIndex();
      zInverted = driverAccelerometer->ZInverted();
      retVal    = true;
      break;
    }

    default:
      break;
    }
  }

  return retVal;
}

HatDirection CAddonJoystickButtonMap::ToHatDirection(JOYSTICK_DRIVER_HAT_DIRECTION driverDirection)
{
  switch (driverDirection)
  {
  case JOYSTICK_DRIVER_HAT_LEFT:   return HatDirectionLeft;
  case JOYSTICK_DRIVER_HAT_RIGHT:  return HatDirectionRight;
  case JOYSTICK_DRIVER_HAT_UP:     return HatDirectionUp;
  case JOYSTICK_DRIVER_HAT_DOWN:   return HatDirectionDown;
  default:                         return HatDirectionNone;
  }
}

SemiAxisDirection CAddonJoystickButtonMap::ToSemiAxisDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION dir)
{
  switch (dir)
  {
  case JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE:  return SemiAxisDirectionNegative;
  case JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE:  return SemiAxisDirectionPositive;
  default:                                           return SemiAxisDirectionUnknown;
  }
}
