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
#pragma once

#include "input/joysticks/IJoystickInputHandler.h"
#include "threads/CriticalSection.h"
#include "threads/Timer.h"

#include <set>

class IGenericJoystickGestureDetector;
class IGenericJoystickButtonMapper;

/*!
 * \ingroup joysticks_generic
 * \brief Generic implementation of IJoystickInputHandler to handle low level (raw)
 *        joystick events and translate them into joystick actions which are passed
 *        on to the registered IJoystickActionHandler implementation.
 *
 * The generic implementation supports single a double touch and hold
 * actions and basic gesture recognition for panning, swiping, pinching/zooming
 * and rotating.
 *
 * \sa IJoystickInputHandler
 */
class CGenericJoystickInputHandler : public IJoystickInputHandler, private ITimerCallback
{
public:
  virtual ~CGenericJoystickInputHandler();

  // implementation of IJoystickInputHandler
  virtual bool HandleJoystickEvent(JoystickEvent event,
                                   unsigned int  index,
                                   int64_t       timeNs,
                                   bool          bPressed  = false,
                                   HatDirection  direction = HatDirectionNone,
                                   float         axisPos   = 0.0f);

protected:
  CGenericJoystickInputHandler(unsigned int id, const std::string& strName, unsigned int vid, unsigned int pid);

  IGenericJoystickButtonMapper *ButtonMapper() { return m_buttonMapper; }

private:
  // private construction, and no assignments; use the provided singleton methods
  CGenericJoystickInputHandler(const CGenericJoystickInputHandler&);
  CGenericJoystickInputHandler const& operator=(CGenericJoystickInputHandler const&);

  // implementation of ITimerCallbacks
  virtual void OnTimeout();

  void triggerDetectors(JoystickEvent event, int32_t pointer);

  CCriticalSection                           m_critical;
  CTimer                                    *m_holdTimer;
  std::map<unsigned int, bool>               m_buttonStates; // action ID -> state
  std::set<IGenericJoystickGestureDetector*> m_detectors;
  IGenericJoystickButtonMapper              *m_buttonMapper;
};
