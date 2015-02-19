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
#pragma once

#include "IGUIControllerWizard.h"
#include "games/GameTypes.h"
#include "input/joysticks/IJoystickButtonMapper.h"
#include "input/joysticks/JoystickDriverPrimitive.h"
#include "threads/CriticalSection.h"
#include "utils/Observer.h"

namespace GAME
{

/*!
 * \brief FSM that walks the user through mapping controller features in order
 *        from beginning to end
 */
class CGUIControllerWizard : public IGUIControllerWizard,
                             public IJoystickButtonMapper,
                             public Observer
{
public:
  CGUIControllerWizard(IGUIControllerWizardCallbacks* callbacks,
                       const GameControllerPtr&       controller);

  virtual ~CGUIControllerWizard(void);

  // implementation of IGUIControllerWizard
  virtual void Run(unsigned int iStartFeature = 0);
  virtual bool Abort(void);
  virtual void OnFocus(unsigned int featureIndex);

  // implementation of IJoystickButtonMapper
  virtual std::string ControllerID(void) const;
  virtual bool IsMapping(void) const;
  virtual bool MapPrimitive(IJoystickButtonMap* buttonMap, const CJoystickDriverPrimitive& primitive);

  // implementation of Observer
  virtual void Notify(const Observable& obs, const ObservableMessage msg);

private:
  enum WizardState
  {
    STATE_IDLE,
    STATE_PROMPT_BUTTON,
    STATE_PROMPT_ANALOG_STICK_UP,
    STATE_PROMPT_ANALOG_STICK_DOWN,
    STATE_PROMPT_ANALOG_STICK_RIGHT,
    STATE_PROMPT_ANALOG_STICK_LEFT,
    STATE_NEXT_FEATURE,
    STATE_FINISHED,
  };

  void Step(void);
  void Action(WizardState state);

  void SetPrompt(const std::string& strPromptMsg);
  void CancelPrompt(void);

  IGUIControllerWizardCallbacks* const m_callbacks;
  const GameControllerPtr              m_controller;
  WizardState                          m_state;
  unsigned int                         m_featureIndex;
  bool                                 m_bAutoClose;
  CJoystickDriverPrimitive             m_lastAnalogStickDir;
  CCriticalSection                     m_mutex;
};

}
