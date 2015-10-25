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

bool CAddonJoystickButtonMapWO::AddPrimitiveFeature(const ::JoystickFeature& feature, const CDriverPrimitive& primitive)
{
  ADDON::PrimitiveFeature primitiveFeature(feature, ToPrimitive(primitive));

  return m_addon->AddFeature(m_device, m_strControllerId, &primitiveFeature);
}

ADDON::DriverPrimitive CAddonJoystickButtonMapWO::ToPrimitive(const CDriverPrimitive& primitive)
{
  ADDON::DriverPrimitive retVal;

  switch (primitive.Type())
  {
    case CDriverPrimitive::Button:
    {
      retVal = ADDON::DriverPrimitive(primitive.Index());
      break;
    }
    case CDriverPrimitive::Hat:
    {
      retVal = ADDON::DriverPrimitive(primitive.Index(), ToHatDirection(primitive.HatDirection()));
      break;
    }
    case CDriverPrimitive::SemiAxis:
    {
      retVal = ADDON::DriverPrimitive(primitive.Index(), ToSemiAxisDirection(primitive.SemiAxisDirection()));
      break;
    }
    default:
      break;
  }

  return retVal;
}

bool CAddonJoystickButtonMapWO::AddAnalogStick(const ::JoystickFeature& feature,
                                               const CDriverPrimitive& up,
                                               const CDriverPrimitive& down,
                                               const CDriverPrimitive& right,
                                               const CDriverPrimitive& left)
{
  ADDON::AnalogStick analogStick(feature, ToPrimitive(up),    ToPrimitive(down),
                                          ToPrimitive(right), ToPrimitive(left));

  return m_addon->AddFeature(m_device, m_strControllerId, &analogStick);
}

bool CAddonJoystickButtonMapWO::AddAccelerometer(const ::JoystickFeature& feature,
                                                 const CDriverPrimitive& positiveX,
                                                 const CDriverPrimitive& positiveY,
                                                 const CDriverPrimitive& positiveZ)
{
  ADDON::Accelerometer accelerometer(feature, ToPrimitive(positiveX),
                                     ToPrimitive(positiveY),
                                     ToPrimitive(positiveZ));

  return m_addon->AddFeature(m_device, m_strControllerId, &accelerometer);
}

JOYSTICK_DRIVER_HAT_DIRECTION CAddonJoystickButtonMapWO::ToHatDirection(HAT_DIRECTION dir)
{
  switch (dir)
  {
    case HAT_DIRECTION::UP:     return JOYSTICK_DRIVER_HAT_UP;
    case HAT_DIRECTION::DOWN:   return JOYSTICK_DRIVER_HAT_DOWN;
    case HAT_DIRECTION::RIGHT:  return JOYSTICK_DRIVER_HAT_RIGHT;
    case HAT_DIRECTION::LEFT:   return JOYSTICK_DRIVER_HAT_LEFT;
    default:
      break;
  }
  return JOYSTICK_DRIVER_HAT_UNKNOWN;
}

JOYSTICK_DRIVER_SEMIAXIS_DIRECTION CAddonJoystickButtonMapWO::ToSemiAxisDirection(SEMIAXIS_DIRECTION dir)
{
  switch (dir)
  {
    case SEMIAXIS_DIRECTION::POSITIVE: return JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE;
    case SEMIAXIS_DIRECTION::NEGATIVE: return JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE;
    default:
      break;
  }
  return JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_UNKNOWN;
}
