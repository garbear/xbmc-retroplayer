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
#include "peripherals/Peripherals.h"
#include "peripherals/bus/PeripheralBusAddon.h"

using namespace ADDON;
using namespace PERIPHERALS;

CAddonJoystickButtonMap::CAddonJoystickButtonMap(CPeripheral* device, const std::string& strDeviceId)
  : m_device(device),
    m_strDeviceId(strDeviceId)
{
}

bool CAddonJoystickButtonMap::Load(void)
{
  m_addon = GetAddon(m_device);

  if (m_addon && m_addon->GetJoystickFeatures(m_device, m_strDeviceId, m_features))
  {
    m_driverMap = GetDriverMap(m_features);
    return true;
  }

  return false;
}

PeripheralAddonPtr CAddonJoystickButtonMap::GetAddon(CPeripheral* device)
{
  PeripheralAddonPtr addon;

  CPeripheralBusAddon* addonBus = static_cast<CPeripheralBusAddon*>(g_peripherals.GetBusByType(PERIPHERAL_BUS_ADDON));

  if (device && addonBus)
  {
    PeripheralBusType busType = device->GetBusType();

    if (busType == PERIPHERAL_BUS_ADDON)
    {
      // If device is from an add-on, use that add-on
      unsigned int index;
      addonBus->SplitLocation(device->Location(), addon, index);
    }
    else
    {
      // Otherwise, have the add-on bus find a suitable add-on
      addonBus->GetAddonWithButtonMap(addon);
    }
  }

  return addon;
}

std::map<CJoystickDriverPrimitive, unsigned int> CAddonJoystickButtonMap::GetDriverMap(const JoystickFeatureMap& features)
{
  std::map<CJoystickDriverPrimitive, unsigned int> driverMap;

  for (JoystickFeatureMap::const_iterator it = features.begin(); it != features.end(); ++it)
  {
    const unsigned int index = it->first;
    const JoystickFeaturePtr& feature = it->second;

    switch (feature->Type())
    {
    case JOYSTICK_DRIVER_TYPE_BUTTON:
    {
      const ADDON::DriverButton* button = static_cast<const ADDON::DriverButton*>(feature.get());
      driverMap[CJoystickDriverPrimitive(button->Index())] = index;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
    {
      const ADDON::DriverHat* hat = static_cast<const ADDON::DriverHat*>(feature.get());
      driverMap[CJoystickDriverPrimitive(hat->Index(), ToHatDirection(hat->Direction()))] = index;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
    {
      const ADDON::DriverSemiAxis* semiaxis = static_cast<const ADDON::DriverSemiAxis*>(feature.get());
      driverMap[CJoystickDriverPrimitive(semiaxis->Index(), ToSemiAxisDirection(semiaxis->Direction()))] = index;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
    {
      const ADDON::DriverAnalogStick* analogStick = static_cast<const ADDON::DriverAnalogStick*>(feature.get());
      driverMap[CJoystickDriverPrimitive(analogStick->XIndex(), SemiAxisDirectionPositive)] = index;
      driverMap[CJoystickDriverPrimitive(analogStick->XIndex(), SemiAxisDirectionNegative)] = index;
      driverMap[CJoystickDriverPrimitive(analogStick->YIndex(), SemiAxisDirectionPositive)] = index;
      driverMap[CJoystickDriverPrimitive(analogStick->YIndex(), SemiAxisDirectionNegative)] = index;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
    {
      const ADDON::DriverAccelerometer* accelerometer = static_cast<const ADDON::DriverAccelerometer*>(feature.get());
      driverMap[CJoystickDriverPrimitive(accelerometer->XIndex(), SemiAxisDirectionPositive)] = index;
      driverMap[CJoystickDriverPrimitive(accelerometer->XIndex(), SemiAxisDirectionNegative)] = index;
      driverMap[CJoystickDriverPrimitive(accelerometer->YIndex(), SemiAxisDirectionPositive)] = index;
      driverMap[CJoystickDriverPrimitive(accelerometer->YIndex(), SemiAxisDirectionNegative)] = index;
      driverMap[CJoystickDriverPrimitive(accelerometer->ZIndex(), SemiAxisDirectionPositive)] = index;
      driverMap[CJoystickDriverPrimitive(accelerometer->ZIndex(), SemiAxisDirectionNegative)] = index;
      break;
    }

    default:
      break;
    }
  }

  return driverMap;
}

bool CAddonJoystickButtonMap::GetFeature(const CJoystickDriverPrimitive& source, unsigned int& featureIndex)
{
  std::map<CJoystickDriverPrimitive, unsigned int>::const_iterator it = m_driverMap.find(source);
  if (it != m_driverMap.end())
  {
    featureIndex = it->second;
    return true;
  }

  return false;
}

bool CAddonJoystickButtonMap::GetButton(unsigned int featureIndex, CJoystickDriverPrimitive& button)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(featureIndex);
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

bool CAddonJoystickButtonMap::GetAnalogStick(unsigned int featureIndex,
                                             int& horizIndex, bool& horizInverted,
                                             int& vertIndex,  bool& vertInverted)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(featureIndex);
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

bool CAddonJoystickButtonMap::GetAccelerometer(unsigned int featureIndex,
                                               int& xIndex, bool& xInverted,
                                               int& yIndex, bool& yInverted,
                                               int& zIndex, bool& zInverted)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(featureIndex);
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
