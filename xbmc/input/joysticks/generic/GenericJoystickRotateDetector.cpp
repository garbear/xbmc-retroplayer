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

#include "GenericJoystickRotateDetector.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795028842
#endif

CGenericJoystickRotateDetector::CGenericJoystickRotateDetector(IJoystickActionHandler *handler)
  : IGenericJoystickGestureDetector(handler),
    m_angle(0.0f)
{
}

bool CGenericJoystickRotateDetector::OnButtonPress(unsigned int id, bool bPressed)
{
  return false; // TODO
}

bool CGenericJoystickRotateDetector::OnButtonMotion(unsigned int id, float magnitude)
{
  return false; // TODO
}

bool CGenericJoystickRotateDetector::OnAnalogStickMotion(unsigned int id, float x, float y)
{
  return false; // TODO
}

bool CGenericJoystickRotateDetector::OnAnalogStickThreshold(unsigned int id, bool bPressed, HatDirection direction /* = HatDirectionNone */)
{
  return false; // TODO
}

bool CGenericJoystickRotateDetector::OnAccelerometerMotion(unsigned int id, float x, float y, float z)
{
  return false; // TODO
}
