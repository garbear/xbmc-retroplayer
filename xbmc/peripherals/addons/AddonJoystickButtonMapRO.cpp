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
#include "utils/log.h"

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

  if (m_addon && m_addon->GetFeatures(m_device, m_strControllerId, m_features))
  {
    CLog::Log(LOGDEBUG, "Loaded button map with %lu features for controller %s",
              m_features.size(), m_strControllerId.c_str());

    m_driverMap = CreateLookupTable(m_features);

    return true;
  }
  else
  {
    CLog::Log(LOGDEBUG, "Failed to load button map for controller %s", m_strControllerId.c_str());
  }

  return false;
}

CAddonJoystickButtonMapRO::DriverMap CAddonJoystickButtonMapRO::CreateLookupTable(const JoystickFeatureMap& features)
{
  DriverMap driverMap;

  for (JoystickFeatureMap::const_iterator it = features.begin(); it != features.end(); ++it)
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    switch (feature->Type())
    {
    case JOYSTICK_FEATURE_TYPE_PRIMITIVE:
    {
      const ADDON::PrimitiveFeature* primitive = static_cast<const ADDON::PrimitiveFeature*>(feature);
      driverMap[ToPrimitive(primitive->Primitive())] = it->first;
      break;
    }

    case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
    {
      const ADDON::AnalogStick* analogStick = static_cast<const ADDON::AnalogStick*>(feature);
      driverMap[ToPrimitive(analogStick->Up())] = it->first;
      driverMap[ToPrimitive(analogStick->Down())] = it->first;
      driverMap[ToPrimitive(analogStick->Right())] = it->first;
      driverMap[ToPrimitive(analogStick->Left())] = it->first;
      break;
    }

    case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
    {
      const ADDON::Accelerometer* accelerometer = static_cast<const ADDON::Accelerometer*>(feature);

      CJoystickDriverPrimitive x_axis(ToPrimitive(accelerometer->PositiveX()));
      CJoystickDriverPrimitive y_axis(ToPrimitive(accelerometer->PositiveY()));
      CJoystickDriverPrimitive z_axis(ToPrimitive(accelerometer->PositiveZ()));

      driverMap[x_axis] = it->first;
      driverMap[y_axis] = it->first;
      driverMap[z_axis] = it->first;

      CJoystickDriverPrimitive x_axis_opposite(x_axis.Index(),
          static_cast<SemiAxisDirection>(x_axis.SemiAxisDir() * -1));

      CJoystickDriverPrimitive y_axis_opposite(y_axis.Index(),
          static_cast<SemiAxisDirection>(y_axis.SemiAxisDir() * -1));

      CJoystickDriverPrimitive z_axis_opposite(z_axis.Index(),
          static_cast<SemiAxisDirection>(z_axis.SemiAxisDir() * -1));

      driverMap[x_axis_opposite] = it->first;
      driverMap[y_axis_opposite] = it->first;
      driverMap[z_axis_opposite] = it->first;
      break;
    }

    default:
      break;
    }
  }

  return driverMap;
}

bool CAddonJoystickButtonMapRO::GetFeature(const CJoystickDriverPrimitive& primitive, ::JoystickFeature& feature)
{
  DriverMap::const_iterator it = m_driverMap.find(primitive);
  if (it != m_driverMap.end())
  {
    feature = it->second;
    return true;
  }

  return false;
}

bool CAddonJoystickButtonMapRO::GetPrimitiveFeature(const ::JoystickFeature& feature, CJoystickDriverPrimitive& primitive)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(feature);
  if (it != m_features.end())
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    if (feature->Type() == JOYSTICK_FEATURE_TYPE_PRIMITIVE)
    {
      primitive = ToPrimitive(static_cast<const ADDON::PrimitiveFeature*>(feature)->Primitive());
      retVal = true;
    }
  }

  return retVal;
}

bool CAddonJoystickButtonMapRO::GetAnalogStick(const ::JoystickFeature& feature,
                                               CJoystickDriverPrimitive& up,
                                               CJoystickDriverPrimitive& down,
                                               CJoystickDriverPrimitive& right,
                                               CJoystickDriverPrimitive& left)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(feature);
  if (it != m_features.end())
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    if (feature->Type() == JOYSTICK_FEATURE_TYPE_ANALOG_STICK)
    {
      const ADDON::AnalogStick* analogStick = static_cast<const ADDON::AnalogStick*>(feature);
      up     = ToPrimitive(analogStick->Up());
      down   = ToPrimitive(analogStick->Down());
      right  = ToPrimitive(analogStick->Right());
      left   = ToPrimitive(analogStick->Left());
      retVal = true;
    }
  }

  return retVal;
}

bool CAddonJoystickButtonMapRO::GetAccelerometer(const ::JoystickFeature& feature,
                                                 CJoystickDriverPrimitive& positiveX,
                                                 CJoystickDriverPrimitive& positiveY,
                                                 CJoystickDriverPrimitive& positiveZ)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(feature);
  if (it != m_features.end())
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    if (feature->Type() == JOYSTICK_FEATURE_TYPE_ACCELEROMETER)
    {
      const ADDON::Accelerometer* accelerometer = static_cast<const ADDON::Accelerometer*>(feature);
      positiveX = ToPrimitive(accelerometer->PositiveX());
      positiveY = ToPrimitive(accelerometer->PositiveY());
      positiveZ = ToPrimitive(accelerometer->PositiveZ());
      retVal    = true;
    }
  }

  return retVal;
}

CJoystickDriverPrimitive CAddonJoystickButtonMapRO::ToPrimitive(const ADDON::DriverPrimitive& primitive)
{
  CJoystickDriverPrimitive retVal;

  switch (primitive.Type())
  {
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_BUTTON:
    {
      retVal = CJoystickDriverPrimitive(primitive.DriverIndex());
      break;
    }
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_HAT_DIRECTION:
    {
      retVal = CJoystickDriverPrimitive(primitive.DriverIndex(), ToHatDirection(primitive.HatDirection()));
      break;
    }
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_SEMIAXIS:
    {
      retVal = CJoystickDriverPrimitive(primitive.DriverIndex(), ToSemiAxisDirection(primitive.SemiAxisDirection()));
      break;
    }
    default:
      break;
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
