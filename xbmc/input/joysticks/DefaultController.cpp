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

#include "DefaultController.h"
#include "ButtonKeyHandler.h"
#include "JoystickTranslator.h"
#include "input/Key.h"

#define DEFAULT_GAME_CONTROLLER    "game.controller.default"

#define ANALOG_DIGITAL_THRESHOLD   0.5f

#ifndef ABS
#define ABS(x)  ((x) >= 0 ? (x) : (-x))
#endif

#ifndef MAX
#define MAX(x, y)  ((x) >= (y) ? (x) : (y))
#endif

CDefaultController::CDefaultController(void)
  : m_handler(new CButtonKeyHandler)
{
}

CDefaultController::~CDefaultController(void)
{
  delete m_handler;
}

std::string CDefaultController::ControllerID(void) const
{
  return DEFAULT_GAME_CONTROLLER;
}

InputType CDefaultController::GetInputType(const std::string& feature) const
{
  return m_handler->GetInputType(GetButtonKeyID(feature));
}

bool CDefaultController::OnButtonPress(const std::string& feature, bool bPressed)
{
  const unsigned int buttonKeyId = GetButtonKeyID(feature);

  if (m_handler->GetInputType(buttonKeyId) == INPUT_TYPE_DIGITAL)
  {
    m_handler->OnDigitalButtonKey(buttonKeyId, bPressed);
    return true;
  }

  return false;
}

bool CDefaultController::OnButtonMotion(const std::string& feature, float magnitude)
{
  const unsigned int buttonKeyId = GetButtonKeyID(feature);

  if (m_handler->GetInputType(buttonKeyId) == INPUT_TYPE_ANALOG)
  {
    m_handler->OnAnalogButtonKey(buttonKeyId, magnitude);
    return true;
  }

  return false;
}

bool CDefaultController::OnAnalogStickMotion(const std::string& feature, float x, float y)
{
  // Calculate the direction of the stick's position
  const CardinalDirection analogStickDir = CJoystickTranslator::VectorToCardinalDirection(x, y);

  // Process directions in which the stick is not pointing first
  for (std::vector<CardinalDirection>::const_iterator it = GetDirections().begin(); it != GetDirections().end(); ++it)
  {
    if (*it == analogStickDir)
      continue;

    // Calculate the button key ID and input type for this direction
    const unsigned int buttonKeyId = GetButtonKeyID(feature, *it);
    const InputType inputType = m_handler->GetInputType(buttonKeyId);

    if (inputType == INPUT_TYPE_DIGITAL)
      m_handler->OnDigitalButtonKey(buttonKeyId, false);
    else if (inputType == INPUT_TYPE_ANALOG)
      m_handler->OnAnalogButtonKey(buttonKeyId, 0.0f);
  }

  // Process analog stick direction last to avoid prematurely clearing the hold timer

  // Calculate the button key ID and input type for the analog stick's direction
  const unsigned int buttonKeyId = GetButtonKeyID(feature, analogStickDir);
  const InputType inputType = m_handler->GetInputType(buttonKeyId);

  // Calculate the magnitude in the cardinal direction
  const float magnitude = MAX(ABS(x), ABS(y));

  if (inputType == INPUT_TYPE_DIGITAL)
  {
    const bool bIsPressed = (magnitude >= ANALOG_DIGITAL_THRESHOLD);
    m_handler->OnDigitalButtonKey(buttonKeyId, bIsPressed);
    return true;
  }
  else if (inputType == INPUT_TYPE_ANALOG)
  {
    m_handler->OnAnalogButtonKey(buttonKeyId, magnitude);
    return true;
  }

  return false;
}

bool CDefaultController::OnAccelerometerMotion(const std::string& feature, float x, float y, float z)
{
  return false; // TODO
}

unsigned int CDefaultController::GetButtonKeyID(const std::string& feature, CardinalDirection dir /* = DirectionUnknown */)
{
  if      (feature == "a")             return KEY_BUTTON_A;
  else if (feature == "b")             return KEY_BUTTON_B;
  else if (feature == "x")             return KEY_BUTTON_X;
  else if (feature == "y")             return KEY_BUTTON_Y;
  else if (feature == "start")         return KEY_BUTTON_START;
  else if (feature == "back")          return KEY_BUTTON_BACK;
  else if (feature == "guide")         return KEY_BUTTON_GUIDE;
  else if (feature == "leftbumper")    return KEY_BUTTON_LEFT_SHOULDER;
  else if (feature == "rightbumper")   return KEY_BUTTON_RIGHT_SHOULDER;
  else if (feature == "leftthumb")     return KEY_BUTTON_LEFT_THUMB_BUTTON;
  else if (feature == "rightthumb")    return KEY_BUTTON_RIGHT_THUMB_BUTTON;
  else if (feature == "up")            return KEY_BUTTON_DPAD_UP;
  else if (feature == "down")          return KEY_BUTTON_DPAD_DOWN;
  else if (feature == "right")         return KEY_BUTTON_DPAD_RIGHT;
  else if (feature == "left")          return KEY_BUTTON_DPAD_LEFT;
  else if (feature == "lefttrigger")   return KEY_BUTTON_LEFT_TRIGGER;
  else if (feature == "righttrigger")  return KEY_BUTTON_RIGHT_TRIGGER;
  else if (feature == "leftstick")
  {
    switch (dir)
    {
    case DirectionUp:     return KEY_BUTTON_LEFT_THUMB_STICK_UP;
    case DirectionDown:   return KEY_BUTTON_LEFT_THUMB_STICK_DOWN;
    case DirectionRight:  return KEY_BUTTON_LEFT_THUMB_STICK_RIGHT;
    case DirectionLeft:   return KEY_BUTTON_LEFT_THUMB_STICK_LEFT;
    default:
      break;
    }
  }
  else if (feature == "rightstick")
  {
    switch (dir)
    {
    case DirectionUp:     return KEY_BUTTON_RIGHT_THUMB_STICK_UP;
    case DirectionDown:   return KEY_BUTTON_RIGHT_THUMB_STICK_DOWN;
    case DirectionRight:  return KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
    case DirectionLeft:   return KEY_BUTTON_RIGHT_THUMB_STICK_LEFT;
    default:
      break;
    }
  }
  else if (feature == "accelerometer") return 0; // TODO

  return 0;
}

const std::vector<CardinalDirection>& CDefaultController::GetDirections(void)
{
  static std::vector<CardinalDirection> directions;
  if (directions.empty())
  {
    directions.push_back(DirectionUp);
    directions.push_back(DirectionDown);
    directions.push_back(DirectionRight);
    directions.push_back(DirectionLeft);
  }
  return directions;
}
