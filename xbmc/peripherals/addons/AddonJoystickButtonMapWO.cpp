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

#include "AddonJoystickButtonMapWO.h"
#include "addons/AddonManager.h"
#include "addons/include/kodi_peripheral_utils.hpp"

using namespace ADDON;
using namespace PERIPHERALS;

CAddonJoystickButtonMapWO::CAddonJoystickButtonMapWO(CPeripheral* device, const PeripheralAddonPtr& addon, const std::string& strControllerId)
  : m_device(device),
    m_addon(addon),
    m_strControllerId(strControllerId)
{
}

bool CAddonJoystickButtonMapWO::Load(void)
{
  return m_addon.get() != NULL;
}

bool CAddonJoystickButtonMapWO::AddPrimitiveFeature(const ::JoystickFeature& feature, const CJoystickDriverPrimitive& primitive)
{
  ADDON::PrimitiveFeature primitiveFeature(feature, ToPrimitive(primitive));

  return m_addon->AddFeature(m_device, m_strControllerId, &primitiveFeature);
}

ADDON::DriverPrimitive CAddonJoystickButtonMapWO::ToPrimitive(const CJoystickDriverPrimitive& primitive)
{
  ADDON::DriverPrimitive retVal;

  switch (primitive.Type())
  {
    case DriverPrimitiveTypeButton:
    {
      retVal = ADDON::DriverPrimitive(primitive.Index());
      break;
    }
    case DriverPrimitiveTypeHatDirection:
    {
      retVal = ADDON::DriverPrimitive(primitive.Index(), ToHatDirection(primitive.HatDir()));
      break;
    }
    case DriverPrimitiveTypeSemiAxis:
    {
      retVal = ADDON::DriverPrimitive(primitive.Index(), ToSemiAxisDirection(primitive.SemiAxisDir()));
      break;
    }
    default:
      break;
  }

  return retVal;
}

bool CAddonJoystickButtonMapWO::AddAnalogStick(const ::JoystickFeature& feature,
                                               const CJoystickDriverPrimitive& up,
                                               const CJoystickDriverPrimitive& down,
                                               const CJoystickDriverPrimitive& right,
                                               const CJoystickDriverPrimitive& left)
{
  ADDON::AnalogStick analogStick(feature, ToPrimitive(up),    ToPrimitive(down),
                                          ToPrimitive(right), ToPrimitive(left));

  return m_addon->AddFeature(m_device, m_strControllerId, &analogStick);
}

bool CAddonJoystickButtonMapWO::AddAccelerometer(const ::JoystickFeature& feature,
                                                 const CJoystickDriverPrimitive& positiveX,
                                                 const CJoystickDriverPrimitive& positiveY,
                                                 const CJoystickDriverPrimitive& positiveZ)
{
  ADDON::Accelerometer accelerometer(feature, ToPrimitive(positiveX),
                                     ToPrimitive(positiveY),
                                     ToPrimitive(positiveZ));

  return m_addon->AddFeature(m_device, m_strControllerId, &accelerometer);
}

JOYSTICK_DRIVER_HAT_DIRECTION CAddonJoystickButtonMapWO::ToHatDirection(HatDirection dir)
{
  switch (dir)
  {
    case HatDirectionLeft:   return JOYSTICK_DRIVER_HAT_LEFT;
    case HatDirectionRight:  return JOYSTICK_DRIVER_HAT_RIGHT;
    case HatDirectionUp:     return JOYSTICK_DRIVER_HAT_UP;
    case HatDirectionDown:   return JOYSTICK_DRIVER_HAT_DOWN;
    default:                 break;
  }
  return JOYSTICK_DRIVER_HAT_UNKNOWN;
}

JOYSTICK_DRIVER_SEMIAXIS_DIRECTION CAddonJoystickButtonMapWO::ToSemiAxisDirection(SemiAxisDirection dir)
{
  switch (dir)
  {
    case SemiAxisDirectionNegative: return JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE;
    case SemiAxisDirectionPositive: return JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE;
    default:                        break;
  }
  return JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_UNKNOWN;
}
