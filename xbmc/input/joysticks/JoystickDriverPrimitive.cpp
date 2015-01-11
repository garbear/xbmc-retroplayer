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

#include "JoystickDriverPrimitive.h"

CJoystickDriverPrimitive::CJoystickDriverPrimitive(void)
  : m_type(),
    m_driverIndex(0),
    m_hatDirection(),
    m_semiAxisDirection()
{
}

CJoystickDriverPrimitive::CJoystickDriverPrimitive(unsigned int buttonIndex)
  : m_type(DriverPrimitiveTypeButton),
    m_driverIndex(buttonIndex),
    m_hatDirection(),
    m_semiAxisDirection()
{
}

CJoystickDriverPrimitive::CJoystickDriverPrimitive(unsigned int hatIndex, HatDirection direction)
  : m_type(DriverPrimitiveTypeHatDirection),
    m_driverIndex(hatIndex),
    m_hatDirection(direction),
    m_semiAxisDirection()
{
}

CJoystickDriverPrimitive::CJoystickDriverPrimitive(unsigned int axisIndex, SemiAxisDirection direction)
  : m_type(DriverPrimitiveTypeSemiAxis),
    m_driverIndex(axisIndex),
    m_hatDirection(),
    m_semiAxisDirection(direction)
{
}

bool CJoystickDriverPrimitive::operator==(const CJoystickDriverPrimitive& rhs) const
{
  if (m_type == rhs.m_type)
  {
    switch (m_type)
    {
    case DriverPrimitiveTypeButton:
      return m_driverIndex == rhs.m_driverIndex;
    case DriverPrimitiveTypeHatDirection:
      return m_driverIndex == rhs.m_driverIndex && m_hatDirection == rhs.m_hatDirection;
    case DriverPrimitiveTypeSemiAxis:
      return m_driverIndex == rhs.m_driverIndex && m_semiAxisDirection == rhs.m_semiAxisDirection;
    default:
      return true;
    }
  }
  return false;
}

bool CJoystickDriverPrimitive::operator<(const CJoystickDriverPrimitive& rhs) const
{
  if (m_type < rhs.m_type) return true;
  if (m_type > rhs.m_type) return false;

  if (m_type != DriverPrimitiveTypeUnknown)
  {
    if (m_driverIndex < rhs.m_driverIndex) return true;
    if (m_driverIndex > rhs.m_driverIndex) return false;
  }

  if (m_type == DriverPrimitiveTypeHatDirection)
  {
    if (m_hatDirection < rhs.m_hatDirection) return true;
    if (m_hatDirection > rhs.m_hatDirection) return false;
  }

  if (m_type == DriverPrimitiveTypeSemiAxis)
  {
    if (m_semiAxisDirection < rhs.m_semiAxisDirection) return true;
    if (m_semiAxisDirection > rhs.m_semiAxisDirection) return false;
  }

  return false;
}

bool CJoystickDriverPrimitive::IsValid(void) const
{
  return m_type == DriverPrimitiveTypeButton ||
        (m_type == DriverPrimitiveTypeHatDirection && (m_hatDirection == HatDirectionLeft  ||
                                                       m_hatDirection == HatDirectionRight ||
                                                       m_hatDirection == HatDirectionUp    ||
                                                       m_hatDirection == HatDirectionDown))  ||
        (m_type == DriverPrimitiveTypeSemiAxis && (m_semiAxisDirection == SemiAxisDirectionPositive ||
                                                   m_semiAxisDirection == SemiAxisDirectionNegative));
}
