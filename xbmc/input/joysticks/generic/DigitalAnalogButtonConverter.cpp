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

#include "DigitalAnalogButtonConverter.h"

#include <algorithm>
#include <assert.h>

#define ANALOG_DIGITAL_THRESHOLD  0.5f

CDigitalAnalogButtonConverter::CDigitalAnalogButtonConverter(IJoystickInputHandler* handler)
  : m_handler(handler)
{
  assert(m_handler);
}

std::string CDigitalAnalogButtonConverter::DeviceID(void) const
{
  return m_handler->DeviceID();
}

bool CDigitalAnalogButtonConverter::OnButtonPress(unsigned int featureIndex, bool bPressed)
{
  /* TODO
  if (m_handler->IsAnalogButton(featureIndex))
    return m_handler->OnButtonMotion(featureIndex, bPressed ? 1.0f : 0.0f);
  */

  return m_handler->OnButtonPress(featureIndex, bPressed);
}

bool CDigitalAnalogButtonConverter::OnButtonMotion(unsigned int featureIndex, float magnitude)
{
  /* TODO
  if (m_handler->IsDigitalButton(featureIndex))
  {
    const bool bIsPressed = magnitude >= ANALOG_DIGITAL_THRESHOLD;
    const bool bWasPressed = std::find(m_pressedButtons.begin(), m_pressedButtons.end(), featureIndex) != m_pressedButtons.end();

    if (!bWasPressed && bIsPressed)
    {
      m_pressedButtons.push_back(featureIndex);
      return m_handler->OnButtonPress(featureIndex, true);
    }
    else if (bWasPressed && !bIsPressed)
    {
      m_pressedButtons.erase(std::remove(m_pressedButtons.begin(), m_pressedButtons.end(), featureIndex));
      return m_handler->OnButtonPress(featureIndex, false);
    }
    else
    {
      return false;
    }
  }
  */

  return m_handler->OnButtonMotion(featureIndex, magnitude);
}

bool CDigitalAnalogButtonConverter::OnAnalogStickMotion(unsigned int featureIndex, float x, float y)
{
  return m_handler->OnAnalogStickMotion(featureIndex, x, y);
}

bool CDigitalAnalogButtonConverter::OnAccelerometerMotion(unsigned int featureIndex, float x, float y, float z)
{
  return m_handler->OnAccelerometerMotion(featureIndex, x, y, z);
}
