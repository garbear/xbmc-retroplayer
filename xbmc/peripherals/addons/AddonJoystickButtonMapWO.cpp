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

#include <assert.h>

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

bool CAddonJoystickButtonMapWO::MapButton(const std::string& feature, const CJoystickDriverPrimitive& primitive)
{
  bool retVal(false);

  switch (primitive.Type())
  {
    case DriverPrimitiveTypeButton:
    {
      ADDON::DriverButton driverButton(feature, primitive.Index());
      retVal = m_addon->MapJoystickFeature(m_device, m_strControllerId, &driverButton);
      break;
    }
    case DriverPrimitiveTypeHatDirection:
    {
      ADDON::DriverHat driverHat(feature, primitive.Index(), ToHatDirection(primitive.HatDir()));
      retVal = m_addon->MapJoystickFeature(m_device, m_strControllerId, &driverHat);
      break;
    }
    case DriverPrimitiveTypeSemiAxis:
    {
      ADDON::DriverSemiAxis driverSemiAxis(feature, primitive.Index(), ToSemiAxisDirection(primitive.SemiAxisDir()));
      retVal = m_addon->MapJoystickFeature(m_device, m_strControllerId, &driverSemiAxis);
      break;
    }
    default:
      break;
  }

  return retVal;
}

bool CAddonJoystickButtonMapWO::MapAnalogStick(const std::string& feature,
                                               int horizIndex, bool horizInverted,
                                               int vertIndex,  bool vertInverted)
{
  ADDON::DriverAnalogStick driverAnalogStick(feature,
                                             horizIndex, horizInverted,
                                             vertIndex,  vertInverted);

  return m_addon->MapJoystickFeature(m_device, m_strControllerId, &driverAnalogStick);
}

bool CAddonJoystickButtonMapWO::MapAccelerometer(const std::string& feature,
                                                  int xIndex, bool xInverted,
                                                  int yIndex, bool yInverted,
                                                  int zIndex, bool zInverted)
{
  ADDON::DriverAccelerometer driverAccelerometer(feature,
                                                 xIndex, xInverted,
                                                 yIndex, yInverted,
                                                 zIndex, zInverted);

  return m_addon->MapJoystickFeature(m_device, m_strControllerId, &driverAccelerometer);
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
