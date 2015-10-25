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

#include "DriverPrimitive.h"

using namespace JOYSTICK;

CDriverPrimitive::CDriverPrimitive(void)
  : m_type(),
    m_driverIndex(0),
    m_hatDirection(),
    m_semiAxisDirection()
{
}

CDriverPrimitive::CDriverPrimitive(unsigned int buttonIndex)
  : m_type(CDriverPrimitive::Button),
    m_driverIndex(buttonIndex),
    m_hatDirection(),
    m_semiAxisDirection()
{
}

CDriverPrimitive::CDriverPrimitive(unsigned int hatIndex, HAT_DIRECTION direction)
  : m_type(CDriverPrimitive::Hat),
    m_driverIndex(hatIndex),
    m_hatDirection(direction),
    m_semiAxisDirection()
{
}

CDriverPrimitive::CDriverPrimitive(unsigned int axisIndex, SEMIAXIS_DIRECTION direction)
  : m_type(CDriverPrimitive::SemiAxis),
    m_driverIndex(axisIndex),
    m_hatDirection(),
    m_semiAxisDirection(direction)
{
}

bool CDriverPrimitive::operator==(const CDriverPrimitive& rhs) const
{
  if (m_type == rhs.m_type)
  {
    switch (m_type)
    {
    case CDriverPrimitive::Button:
      return m_driverIndex == rhs.m_driverIndex;
    case CDriverPrimitive::Hat:
      return m_driverIndex == rhs.m_driverIndex && m_hatDirection == rhs.m_hatDirection;
    case CDriverPrimitive::SemiAxis:
      return m_driverIndex == rhs.m_driverIndex && m_semiAxisDirection == rhs.m_semiAxisDirection;
    default:
      return true;
    }
  }
  return false;
}

bool CDriverPrimitive::operator<(const CDriverPrimitive& rhs) const
{
  if (m_type < rhs.m_type) return true;
  if (m_type > rhs.m_type) return false;

  if (m_type != CDriverPrimitive::Unknown)
  {
    if (m_driverIndex < rhs.m_driverIndex) return true;
    if (m_driverIndex > rhs.m_driverIndex) return false;
  }

  if (m_type == CDriverPrimitive::Hat)
  {
    if (m_hatDirection < rhs.m_hatDirection) return true;
    if (m_hatDirection > rhs.m_hatDirection) return false;
  }

  if (m_type == CDriverPrimitive::SemiAxis)
  {
    if (m_semiAxisDirection < rhs.m_semiAxisDirection) return true;
    if (m_semiAxisDirection > rhs.m_semiAxisDirection) return false;
  }

  return false;
}

bool CDriverPrimitive::IsValid(void) const
{
  return m_type == CDriverPrimitive::Button ||

        (m_type == CDriverPrimitive::Hat && (m_hatDirection == HAT_DIRECTION::UP     ||
                                             m_hatDirection == HAT_DIRECTION::DOWN   ||
                                             m_hatDirection == HAT_DIRECTION::RIGHT  ||
                                             m_hatDirection == HAT_DIRECTION::LEFT)) ||

        (m_type == CDriverPrimitive::SemiAxis && (m_semiAxisDirection == SEMIAXIS_DIRECTION::POSITIVE ||
                                                  m_semiAxisDirection == SEMIAXIS_DIRECTION::NEGATIVE));
}
