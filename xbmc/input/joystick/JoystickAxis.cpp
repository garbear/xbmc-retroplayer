/*
 *      Copyright (C) 2007-2014 Team XBMC
 *      http://www.xbmc.org
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

#include "JoystickAxis.h"
#include "settings/AdvancedSettings.h"

using namespace INPUT;

#define AXIS_DIGITAL_DEADZONE   0.5f // Axis must be pushed past this for digital action repeats

#ifndef CLAMP
#define CLAMP(value, min, max)  ((value) > (max) ? (max) : (value) < (min) ? (min) : (value))
#endif

void CJoystickAxis::SetAxis(long value, unsigned long maxAxisAmount)
{
  value = CLAMP(value, -maxAxisAmount, maxAxisAmount);

  const unsigned long deadzoneRange = (unsigned long)(g_advancedSettings.m_controllerDeadzone * maxAxisAmount);

  if (value > deadzoneRange)
    m_value = (float)(value - deadzoneRange) / (float)(maxAxisAmount - deadzoneRange);
  else if (value < -deadzoneRange)
    m_value = (float)(value + deadzoneRange) / (float)(maxAxisAmount - deadzoneRange);
  else
    m_value = 0.0f;
}
