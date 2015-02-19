/*
 *      Copyright (C) 2015 Team XBMC
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

#include "GUIControllerWizard.h"
#include "games/addons/GameController.h"
#include "guilib/LocalizeStrings.h"
#include "input/joysticks/IJoystickButtonMap.h"
#include "peripherals/Peripherals.h"
#include "threads/SingleLock.h"
#include "utils/StringUtils.h"

#include <assert.h>

using namespace GAME;
using namespace PERIPHERALS;

#define ESC_KEY_CODE  27

CGUIControllerWizard::CGUIControllerWizard(IGUIControllerWizardCallbacks* callbacks,
                                           const GameControllerPtr&       controller)
  : m_callbacks(callbacks),
    m_controller(controller),
    m_state(STATE_IDLE),
    m_featureIndex(0),
    m_bAutoClose(true)
{
  assert(m_callbacks != NULL);
  assert(m_controller.get() != NULL);

  g_peripherals.RegisterJoystickButtonMapper(this);
  g_peripherals.RegisterObserver(this);
}

CGUIControllerWizard::~CGUIControllerWizard(void)
{
  g_peripherals.UnregisterObserver(this);
  g_peripherals.UnregisterJoystickButtonMapper(this);
}

void CGUIControllerWizard::Run(unsigned int iStartFeature /* = 0 */)
{
  CSingleLock lock(m_mutex);

  Abort();

  m_state = STATE_IDLE;
  m_featureIndex = iStartFeature;

  Step();
}

void CGUIControllerWizard::Step()
{
  const std::vector<CGameControllerFeature>& features = m_controller->Layout().Features();

  if (m_featureIndex >= features.size())
  {
    m_state = STATE_FINISHED;
  }
  else
  {
    switch (m_state)
    {
      case STATE_IDLE:
      case STATE_NEXT_FEATURE:
        switch (features[m_featureIndex].Type())
        {
          case FEATURE_BUTTON:
            m_state = STATE_PROMPT_BUTTON;
            break;
          case FEATURE_ANALOG_STICK:
            m_state = STATE_PROMPT_ANALOG_STICK_UP;
            break;
          default:
            m_state = STATE_NEXT_FEATURE;
            break;
        }
        break;
      case STATE_PROMPT_BUTTON:
        m_state = STATE_NEXT_FEATURE;
        break;
      case STATE_PROMPT_ANALOG_STICK_UP:
        if (m_lastAnalogStickDir.Type() == DriverPrimitiveTypeSemiAxis)
          m_state = STATE_PROMPT_ANALOG_STICK_RIGHT;
        else
          m_state = STATE_PROMPT_ANALOG_STICK_DOWN;
        break;
      case STATE_PROMPT_ANALOG_STICK_DOWN:
        m_state = STATE_PROMPT_ANALOG_STICK_RIGHT;
        break;
      case STATE_PROMPT_ANALOG_STICK_RIGHT:
        if (m_lastAnalogStickDir.Type() == DriverPrimitiveTypeSemiAxis)
          m_state = STATE_NEXT_FEATURE;
        else
          m_state = STATE_PROMPT_ANALOG_STICK_LEFT;
        break;
      case STATE_PROMPT_ANALOG_STICK_LEFT:
        m_state = STATE_NEXT_FEATURE;
        break;
      default:
        break;
    }
  }

  Action(m_state);
}

void CGUIControllerWizard::Action(WizardState state)
{
  const std::vector<CGameControllerFeature>& features = m_controller->Layout().Features();

  switch (state)
  {
    case STATE_IDLE:
    {
      break;
    }
    case STATE_PROMPT_BUTTON:
    {
      std::string strPromptTemplate = g_localizeStrings.Get(35051); // "Press %s..."
      std::string strFeatureName = m_controller->GetString(features[m_featureIndex].Label());
      std::string strPrompt = StringUtils::Format(strPromptTemplate.c_str(), strFeatureName.c_str());
      SetPrompt(strPrompt);
      break;
    }
    case STATE_PROMPT_ANALOG_STICK_UP:
    {
      std::string strPromptTemplate = g_localizeStrings.Get(35052); // "Move %s up..."
      std::string strFeatureName = m_controller->GetString(features[m_featureIndex].Label());
      std::string strPrompt = StringUtils::Format(strPromptTemplate.c_str(), strFeatureName.c_str());
      SetPrompt(strPrompt);
      break;
    }
    case STATE_PROMPT_ANALOG_STICK_DOWN:
    {
      std::string strPromptTemplate = g_localizeStrings.Get(35053); // "Move %s down..."
      std::string strFeatureName = m_controller->GetString(features[m_featureIndex].Label());
      std::string strPrompt = StringUtils::Format(strPromptTemplate.c_str(), strFeatureName.c_str());
      SetPrompt(strPrompt);
      break;
    }
    case STATE_PROMPT_ANALOG_STICK_RIGHT:
    {
      std::string strPromptTemplate = g_localizeStrings.Get(35054); // "Move %s right..."
      std::string strFeatureName = m_controller->GetString(features[m_featureIndex].Label());
      std::string strPrompt = StringUtils::Format(strPromptTemplate.c_str(), strFeatureName.c_str());
      SetPrompt(strPrompt);
      break;
    }
    case STATE_PROMPT_ANALOG_STICK_LEFT:
    {
      std::string strPromptTemplate = g_localizeStrings.Get(35055); // "Move %s left..."
      std::string strFeatureName = m_controller->GetString(features[m_featureIndex].Label());
      std::string strPrompt = StringUtils::Format(strPromptTemplate.c_str(), strFeatureName.c_str());
      SetPrompt(strPrompt);
      break;
    }
    case STATE_NEXT_FEATURE:
    {
      m_featureIndex++;
      Step();
      break;
    }
    case STATE_FINISHED:
    {
      if (m_bAutoClose)
        m_callbacks->End();
      break;
    }
    default:
      break;
  }
}

bool CGUIControllerWizard::Abort(void)
{
  CSingleLock lock(m_mutex);

  bool bAborted = false;

  if (IsMapping())
  {
    CancelPrompt();

    m_state = STATE_FINISHED;
    m_bAutoClose = false;

    bAborted = true;
  }

  return bAborted;
}

void CGUIControllerWizard::OnFocus(unsigned int featureIndex)
{
  CSingleLock lock(m_mutex);

  if (m_featureIndex != featureIndex)
    Abort();
}

std::string CGUIControllerWizard::ControllerID(void) const
{
  return m_controller->ID();
}

bool CGUIControllerWizard::IsMapping(void) const
{
  CSingleLock lock(m_mutex);

  switch (m_state)
  {
    case STATE_PROMPT_BUTTON:
    case STATE_PROMPT_ANALOG_STICK_UP:
    case STATE_PROMPT_ANALOG_STICK_DOWN:
    case STATE_PROMPT_ANALOG_STICK_RIGHT:
    case STATE_PROMPT_ANALOG_STICK_LEFT:
      return true;

    default:
      break;
  }

  return false;
}

bool CGUIControllerWizard::MapPrimitive(IJoystickButtonMap* buttonMap, const CJoystickDriverPrimitive& primitive)
{
  CSingleLock lock(m_mutex);

  bool bHandled = false;

  // Handle esc key separately
  if (primitive.Type() == DriverPrimitiveTypeButton && primitive.Index() == ESC_KEY_CODE)
  {
    bHandled = Abort();
  }
  else
  {
    const std::vector<CGameControllerFeature>& features = m_controller->Layout().Features();

    switch (m_state)
    {
      case STATE_PROMPT_BUTTON:
      {
        bHandled = buttonMap->MapButton(features[m_featureIndex].Name(), primitive);
        break;
      }
      case STATE_PROMPT_ANALOG_STICK_UP:
      {
        if (primitive.Type() == DriverPrimitiveTypeSemiAxis)
        {
          int  horizIndex = -1;
          bool horizInverted = false;
          int  vertIndex = -1;
          bool vertInverted = false;

          buttonMap->GetAnalogStick(features[m_featureIndex].Name(),
                                    horizIndex, horizInverted,
                                    vertIndex,  vertInverted);

          vertIndex = primitive.Index();
          vertInverted = (primitive.SemiAxisDir() == SemiAxisDirectionNegative);

          bHandled = buttonMap->MapAnalogStick(features[m_featureIndex].Name(),
                                               horizIndex, horizInverted,
                                               vertIndex,  vertInverted);

          m_lastAnalogStickDir = primitive;
        }
        break;
      }
      case STATE_PROMPT_ANALOG_STICK_DOWN:
      {
        bHandled = true; // TODO
        m_lastAnalogStickDir = primitive;
        break;
      }
      case STATE_PROMPT_ANALOG_STICK_RIGHT:
      {
        if (primitive.Type() == DriverPrimitiveTypeSemiAxis)
        {
          int  horizIndex = -1;
          bool horizInverted = false;
          int  vertIndex = -1;
          bool vertInverted = false;

          buttonMap->GetAnalogStick(features[m_featureIndex].Name(),
                                    horizIndex, horizInverted,
                                    vertIndex,  vertInverted);

          horizIndex = primitive.Index();
          horizInverted = (primitive.SemiAxisDir() == SemiAxisDirectionNegative);

          bHandled = buttonMap->MapAnalogStick(features[m_featureIndex].Name(),
                                               horizIndex, horizInverted,
                                               vertIndex,  vertInverted);

          m_lastAnalogStickDir = primitive;
        }
        break;
      }
      case STATE_PROMPT_ANALOG_STICK_LEFT:
      {
        bHandled = true; // TODO
        m_lastAnalogStickDir = primitive;
        break;
      }
      default:
        break;
    }

    if (bHandled)
    {
      CancelPrompt();
      Step();
    }
  }

  return bHandled;
}

void CGUIControllerWizard::Notify(const Observable& obs, const ObservableMessage msg)
{
  switch (msg)
  {
    case ObservableMessagePeripheralsChanged:
    {
      g_peripherals.UnregisterJoystickButtonMapper(this);
      g_peripherals.RegisterJoystickButtonMapper(this);
      break;
    }
    default:
      break;
  }
}

void CGUIControllerWizard::SetPrompt(const std::string& strPromptMsg)
{
  m_callbacks->Focus(m_featureIndex);
  m_callbacks->SetLabel(m_featureIndex, strPromptMsg);
}

void CGUIControllerWizard::CancelPrompt(void)
{
  m_callbacks->ResetLabel(m_featureIndex);
}
