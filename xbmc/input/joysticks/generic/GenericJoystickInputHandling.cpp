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

#include "GenericJoystickInputHandling.h"
#include "DigitalAnalogButtonConverter.h"
#include "input/joysticks/IJoystickButtonMap.h"
#include "input/joysticks/IJoystickInputHandler.h"
#include "input/joysticks/JoystickDriverPrimitive.h"
#include "input/joysticks/JoystickTranslator.h"
#include "utils/log.h"

#include <algorithm>

CGenericJoystickInputHandling::CGenericJoystickInputHandling(IJoystickInputHandler* handler, IJoystickButtonMap* buttonMap)
 : m_handler(new CDigitalAnalogButtonConverter(handler)),
   m_buttonMap(buttonMap)
{
}

CGenericJoystickInputHandling::~CGenericJoystickInputHandling(void)
{
  delete m_handler;
}

bool CGenericJoystickInputHandling::OnButtonMotion(unsigned int buttonIndex, bool bPressed)
{
  const char pressed = bPressed ? 1 : 0;

  if (m_buttonStates.size() <= buttonIndex)
    m_buttonStates.resize(buttonIndex + 1);

  bool bHandled = false;

  std::string feature;
  if (m_buttonMap->GetFeature(CJoystickDriverPrimitive(buttonIndex), feature))
  {
    CLog::Log(LOGDEBUG, "CGenericJoystickInputHandling: %s feature [ %s ] %s",
              m_handler->ControllerID().c_str(), feature.c_str(), bPressed ? "pressed" : "released");

    char& wasPressed = m_buttonStates[buttonIndex];

    if (!wasPressed && pressed)
      OnPress(feature);
    else if (wasPressed && !pressed)
      OnRelease(feature);

    wasPressed = pressed;

    if (pressed)
      bHandled = true;
  }

  return bHandled;
}

bool CGenericJoystickInputHandling::OnHatMotion(unsigned int hatIndex, HatDirection newDirection)
{
  if (m_hatStates.size() <= hatIndex)
    m_hatStates.resize(hatIndex + 1);

  HatDirection& oldDirection = m_hatStates[hatIndex];

  bool bHandled = false;

  bHandled |= ProcessHatDirection(hatIndex, oldDirection, newDirection, HatDirectionUp);
  bHandled |= ProcessHatDirection(hatIndex, oldDirection, newDirection, HatDirectionRight);
  bHandled |= ProcessHatDirection(hatIndex, oldDirection, newDirection, HatDirectionDown);
  bHandled |= ProcessHatDirection(hatIndex, oldDirection, newDirection, HatDirectionLeft);

  oldDirection = newDirection;

  return bHandled;
}

bool CGenericJoystickInputHandling::ProcessHatDirection(int index,
    HatDirection oldDir, HatDirection newDir, HatDirection targetDir)
{
  bool bHandled = false;

  if ((oldDir & targetDir) != (newDir & targetDir))
  {
    const bool bActivated = (newDir & targetDir) != HatDirectionNone;

    std::string feature;
    if (m_buttonMap->GetFeature(CJoystickDriverPrimitive(index, targetDir), feature))
    {
      if (bActivated)
        bHandled = true;

      CLog::Log(LOGDEBUG, "CGenericJoystickInputHandling: %s feature [ %s ] %s from hat",
                m_handler->ControllerID().c_str(), feature.c_str(), bActivated ? "activated" : "deactivated");

      if (bActivated)
        OnPress(feature);
      else
        OnRelease(feature);
    }
  }

  return false;
}

bool CGenericJoystickInputHandling::OnAxisMotion(unsigned int axisIndex, float newPosition)
{
  if (m_axisStates.size() <= axisIndex)
    m_axisStates.resize(axisIndex + 1);

  if (m_axisStates[axisIndex] == 0.0f && newPosition == 0.0f)
    return false;

  float oldPosition = m_axisStates[axisIndex];
  m_axisStates[axisIndex] = newPosition;

  CJoystickDriverPrimitive positiveAxis(axisIndex, SemiAxisDirectionPositive);
  CJoystickDriverPrimitive negativeAxis(axisIndex, SemiAxisDirectionNegative);

  std::string positiveFeature;
  std::string negativeFeature;

  bool bHasFeaturePositive = m_buttonMap->GetFeature(positiveAxis, positiveFeature);
  bool bHasFeatureNegative = m_buttonMap->GetFeature(negativeAxis, negativeFeature);

  bool bHandled = false;

  if (bHasFeaturePositive || bHasFeatureNegative)
  {
    bHandled = true;

    // If the positive and negative semiaxis correspond to the same feature,
    // then we must be dealing with an analog stick or accelerometer. These both
    // require multiple axes, so record the axis and batch-process later during
    // ProcessAxisMotions()

    bool bNeedsMoreAxes = (positiveFeature == negativeFeature);

    if (bNeedsMoreAxes)
    {
      if (std::find(m_featuresWithMotion.begin(), m_featuresWithMotion.end(), positiveFeature) == m_featuresWithMotion.end())
        m_featuresWithMotion.push_back(positiveFeature);
    }
    else
    {
      if (bHasFeaturePositive)
      {
        // If new position passes through the origin, 0.0f is sent exactly once
        // until the position becomes positive again
        if (newPosition > 0)
          m_handler->OnButtonMotion(positiveFeature, newPosition);
        else if (oldPosition > 0)
          m_handler->OnButtonMotion(positiveFeature, 0.0f);
      }

      if (bHasFeatureNegative)
      {
        // If new position passes through the origin, 0.0f is sent exactly once
        // until the position becomes negative again
        if (newPosition < 0)
          m_handler->OnButtonMotion(negativeFeature, -1.0f * newPosition); // magnitude is >= 0
        else if (oldPosition < 0)
          m_handler->OnButtonMotion(negativeFeature, 0.0f);
      }
    }
  }

  return bHandled;
}

void CGenericJoystickInputHandling::ProcessAxisMotions(void)
{
  std::vector<std::string> featuresToProcess;
  featuresToProcess.swap(m_featuresWithMotion);

  // Invoke callbacks for features with motion
  for (std::vector<std::string>::const_iterator it = featuresToProcess.begin(); it != featuresToProcess.end(); ++it)
  {
    const std::string feature = *it;

    int  xIndex;
    bool xInverted;
    int  yIndex;
    bool yInverted;
    int  zIndex;
    bool zInverted;

    if (m_buttonMap->GetAnalogStick(feature, xIndex, xInverted, yIndex,  yInverted))
    {
      const float horizPos = GetAxisState(xIndex) * (xInverted ? -1.0f : 1.0f);
      const float vertPos  = GetAxisState(yIndex)  * (yInverted  ? -1.0f : 1.0f);
      m_handler->OnAnalogStickMotion(feature, horizPos, vertPos);
    }
    else if (m_buttonMap->GetAccelerometer(feature, xIndex, xInverted, yIndex, yInverted, zIndex, zInverted))
    {
      const float xPos = GetAxisState(xIndex) * (xInverted ? -1.0f : 1.0f);
      const float yPos = GetAxisState(yIndex) * (yInverted ? -1.0f : 1.0f);
      const float zPos = GetAxisState(zIndex) * (zInverted ? -1.0f : 1.0f);
      m_handler->OnAccelerometerMotion(feature, xPos, yPos, zPos);
    }
  }

  // Digital buttons emulating analog buttons need to be repeated every frame
  for (std::vector<std::string>::const_iterator it = m_repeatingFeatures.begin(); it != m_repeatingFeatures.end(); ++it)
    m_handler->OnButtonPress(*it, true);
}

void CGenericJoystickInputHandling::OnPress(const std::string& feature)
{
  const InputType inputType = m_handler->GetInputType(feature);

  if (inputType == INPUT_TYPE_DIGITAL)
    m_handler->OnButtonPress(feature, true);
  else if (inputType == INPUT_TYPE_ANALOG)
    StartDigitalRepeating(feature); // Analog actions repeat every frame
}

void CGenericJoystickInputHandling::OnRelease(const std::string& feature)
{
  m_handler->OnButtonPress(feature, false);
  StopDigitalRepeating(feature);
}

void CGenericJoystickInputHandling::StartDigitalRepeating(const std::string& feature)
{
  m_repeatingFeatures.push_back(feature);
}

void CGenericJoystickInputHandling::StopDigitalRepeating(const std::string& feature)
{
  m_repeatingFeatures.erase(std::remove(m_repeatingFeatures.begin(), m_repeatingFeatures.end(), feature), m_repeatingFeatures.end());
}

float CGenericJoystickInputHandling::GetAxisState(int axisIndex) const
{
  return (0 <= axisIndex && axisIndex < (int)m_axisStates.size()) ? m_axisStates[axisIndex] : 0;
}
