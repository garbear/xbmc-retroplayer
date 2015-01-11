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

#include "DigitalAnalogButtonConverter.h"

#include <algorithm>
#include <assert.h>

#define ANALOG_DIGITAL_THRESHOLD  0.5f

CDigitalAnalogButtonConverter::CDigitalAnalogButtonConverter(IJoystickInputHandler* handler)
  : m_handler(handler)
{
  assert(m_handler);
}

std::string CDigitalAnalogButtonConverter::ControllerID(void) const
{
  return m_handler->ControllerID();
}

InputType CDigitalAnalogButtonConverter::GetInputType(const std::string& feature) const
{
  return m_handler->GetInputType(feature);
}

bool CDigitalAnalogButtonConverter::OnButtonPress(const std::string& feature, bool bPressed)
{
  if (GetInputType(feature) == INPUT_TYPE_ANALOG)
    return m_handler->OnButtonMotion(feature, bPressed ? 1.0f : 0.0f);

  return m_handler->OnButtonPress(feature, bPressed);
}

bool CDigitalAnalogButtonConverter::OnButtonMotion(const std::string& feature, float magnitude)
{
  if (GetInputType(feature) == INPUT_TYPE_DIGITAL)
  {
    const bool bIsPressed = (magnitude >= ANALOG_DIGITAL_THRESHOLD);

    if (bIsPressed && !IsActivated(feature))
    {
      Activate(feature);
      return m_handler->OnButtonPress(feature, true);
    }
    else if (!bIsPressed && IsActivated(feature))
    {
      Deactivate(feature);
      return m_handler->OnButtonPress(feature, false);
    }
    else
    {
      return true;
    }
  }

  return m_handler->OnButtonMotion(feature, magnitude);
}

bool CDigitalAnalogButtonConverter::OnAnalogStickMotion(const std::string& feature, float x, float y)
{
  return m_handler->OnAnalogStickMotion(feature, x, y);
}

bool CDigitalAnalogButtonConverter::OnAccelerometerMotion(const std::string& feature, float x, float y, float z)
{
  return m_handler->OnAccelerometerMotion(feature, x, y, z);
}

bool CDigitalAnalogButtonConverter::IsActivated(const std::string& feature) const
{
  return std::find(m_activatedFeatures.begin(), m_activatedFeatures.end(), feature) != m_activatedFeatures.end();
}

void CDigitalAnalogButtonConverter::Activate(const std::string& feature)
{
  m_activatedFeatures.push_back(feature);
}

void CDigitalAnalogButtonConverter::Deactivate(const std::string& feature)
{
  m_activatedFeatures.erase(std::remove(m_activatedFeatures.begin(), m_activatedFeatures.end(), feature), m_activatedFeatures.end());
}
