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

#include "AddonJoystickButtonMapRO.h"
#include "addons/include/kodi_peripheral_utils.hpp"

using namespace PERIPHERALS;

CAddonJoystickButtonMapRO::CAddonJoystickButtonMapRO(CPeripheral* device, const PeripheralAddonPtr& addon, const std::string& strControllerId)
  : m_device(device),
    m_addon(addon),
    m_strControllerId(strControllerId)
{
}

bool CAddonJoystickButtonMapRO::Load(void)
{
  m_features.clear();
  m_driverMap.clear();

  if (m_addon && m_addon->GetButtonMap(m_device, m_strControllerId, m_features))
  {
    m_driverMap = ToDriverMap(m_features);
    return true;
  }

  return false;
}

CAddonJoystickButtonMapRO::DriverMap CAddonJoystickButtonMapRO::ToDriverMap(const JoystickFeatureMap& features)
{
  DriverMap driverMap;

  for (JoystickFeatureMap::const_iterator it = features.begin(); it != features.end(); ++it)
  {
    const JoystickFeaturePtr& feature = it->second;

    switch (feature->Type())
    {
    case JOYSTICK_DRIVER_TYPE_BUTTON:
    {
      const ADDON::DriverButton* button = static_cast<const ADDON::DriverButton*>(feature.get());
      driverMap[CJoystickDriverPrimitive(button->Index())] = it->first;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
    {
      const ADDON::DriverHat* hat = static_cast<const ADDON::DriverHat*>(feature.get());
      driverMap[CJoystickDriverPrimitive(hat->Index(), ToHatDirection(hat->Direction()))] = it->first;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
    {
      const ADDON::DriverSemiAxis* semiaxis = static_cast<const ADDON::DriverSemiAxis*>(feature.get());
      driverMap[CJoystickDriverPrimitive(semiaxis->Index(), ToSemiAxisDirection(semiaxis->Direction()))] = it->first;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
    {
      const ADDON::DriverAnalogStick* analogStick = static_cast<const ADDON::DriverAnalogStick*>(feature.get());
      driverMap[CJoystickDriverPrimitive(analogStick->XIndex(), SemiAxisDirectionPositive)] = it->first;
      driverMap[CJoystickDriverPrimitive(analogStick->XIndex(), SemiAxisDirectionNegative)] = it->first;
      driverMap[CJoystickDriverPrimitive(analogStick->YIndex(), SemiAxisDirectionPositive)] = it->first;
      driverMap[CJoystickDriverPrimitive(analogStick->YIndex(), SemiAxisDirectionNegative)] = it->first;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
    {
      const ADDON::DriverAccelerometer* accelerometer = static_cast<const ADDON::DriverAccelerometer*>(feature.get());
      driverMap[CJoystickDriverPrimitive(accelerometer->XIndex(), SemiAxisDirectionPositive)] = it->first;
      driverMap[CJoystickDriverPrimitive(accelerometer->XIndex(), SemiAxisDirectionNegative)] = it->first;
      driverMap[CJoystickDriverPrimitive(accelerometer->YIndex(), SemiAxisDirectionPositive)] = it->first;
      driverMap[CJoystickDriverPrimitive(accelerometer->YIndex(), SemiAxisDirectionNegative)] = it->first;
      driverMap[CJoystickDriverPrimitive(accelerometer->ZIndex(), SemiAxisDirectionPositive)] = it->first;
      driverMap[CJoystickDriverPrimitive(accelerometer->ZIndex(), SemiAxisDirectionNegative)] = it->first;
      break;
    }

    default:
      break;
    }
  }

  return driverMap;
}

bool CAddonJoystickButtonMapRO::GetFeature(const CJoystickDriverPrimitive& primitive, std::string& feature)
{
  DriverMap::const_iterator it = m_driverMap.find(primitive);
  if (it != m_driverMap.end())
  {
    feature = it->second;
    return true;
  }

  return false;
}

bool CAddonJoystickButtonMapRO::GetButton(const std::string& feature, CJoystickDriverPrimitive& button)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(feature);
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

bool CAddonJoystickButtonMapRO::GetAnalogStick(const std::string& feature,
                                               int& horizIndex, bool& horizInverted,
                                               int& vertIndex,  bool& vertInverted)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(feature);
  if (it != m_features.end())
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    if (feature->Type() == JOYSTICK_DRIVER_TYPE_ANALOG_STICK)
    {
      const ADDON::DriverAnalogStick* driverAnalogStick = static_cast<const ADDON::DriverAnalogStick*>(feature);
      horizIndex    = driverAnalogStick->XIndex();
      horizInverted = driverAnalogStick->XInverted();
      vertIndex     = driverAnalogStick->YIndex();
      vertInverted  = driverAnalogStick->YInverted();
      retVal        = true;
    }
  }

  return retVal;
}

bool CAddonJoystickButtonMapRO::GetAccelerometer(const std::string& feature,
                                                 int& xIndex, bool& xInverted,
                                                 int& yIndex, bool& yInverted,
                                                 int& zIndex, bool& zInverted)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(feature);
  if (it != m_features.end())
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    if (feature->Type() == JOYSTICK_DRIVER_TYPE_ACCELEROMETER)
    {
      const ADDON::DriverAccelerometer* driverAccelerometer = static_cast<const ADDON::DriverAccelerometer*>(feature);
      xIndex    = driverAccelerometer->XIndex();
      xInverted = driverAccelerometer->XInverted();
      yIndex    = driverAccelerometer->YIndex();
      yInverted = driverAccelerometer->YInverted();
      zIndex    = driverAccelerometer->ZIndex();
      zInverted = driverAccelerometer->ZInverted();
      retVal    = true;
    }
  }

  return retVal;
}

HatDirection CAddonJoystickButtonMapRO::ToHatDirection(JOYSTICK_DRIVER_HAT_DIRECTION driverDirection)
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

SemiAxisDirection CAddonJoystickButtonMapRO::ToSemiAxisDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION dir)
{
  switch (dir)
  {
  case JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE: return SemiAxisDirectionNegative;
  case JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE: return SemiAxisDirectionPositive;
  default:                                          return SemiAxisDirectionUnknown;
  }
}
