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

#include <math.h>

#include "GenericJoystickInputHandler.h"
#include "input/joysticks/generic/GenericJoystickButtonMapper.h"
#include "input/joysticks/generic/GenericJoystickRotateDetector.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

CGenericJoystickInputHandler::CGenericJoystickInputHandler(unsigned int id, const std::string& strName, unsigned int vid, unsigned int pid)
{
  m_holdTimer    = new CTimer(this);
  m_buttonMapper = new CGenericJoystickButtonMapper(this);

  m_detectors.insert(new CGenericJoystickRotateDetector(this));
}

CGenericJoystickInputHandler::~CGenericJoystickInputHandler()
{
  delete m_holdTimer;
  delete m_buttonMapper;

  for (std::set<IGenericJoystickGestureDetector*>::const_iterator detector = m_detectors.begin(); detector != m_detectors.end(); detector++)
    delete *detector;
  m_detectors.clear();
}

bool CGenericJoystickInputHandler::HandleJoystickEvent(JoystickEvent event,
                                                       unsigned int  index,
                                                       int64_t       timeNs,
                                                       bool          bPressed  /* = false */,
                                                       HatDirection  direction /* = HatDirectionNone */,
                                                       float         axisPos   /* = 0.0f */)
{
  if (timeNs < 0)
    return false;

  CSingleLock lock(m_critical);

  bool result = false;

  switch (event)
  {
    case JoystickEventRawButton:
      result = ButtonMapper()->OnRawButtonPress(index, bPressed);
      break;

    case JoystickEventRawHat:
      result = ButtonMapper()->OnRawHatMotion(index, direction);
      break;

    case JoystickEventRawAxis:
      result = ButtonMapper()->OnRawAxisMotion(index, axisPos);
      break;

    default:
      CLog::Log(LOGDEBUG, "CGenericJoystickInputHandler: unknown JoystickEvent: %u", event);
      break;
  }

  return result;
}

void CGenericJoystickInputHandler::OnTimeout()
{
  /* TODO
  CSingleLock lock(m_critical);

  switch (m_gestureState)
  {
    case TouchGestureSingleTouch:
      break;

    case TouchGestureMultiTouchStart:
      break;

    default:
      break;
  }
  */
}

void CGenericJoystickInputHandler::triggerDetectors(JoystickEvent event, int32_t pointer)
{
  switch (event)
  {
    case TouchInputAbort:
    {
      for (std::set<IGenericJoystickGestureDetector*>::const_iterator detector = m_detectors.begin(); detector != m_detectors.end(); detector++)
        delete (*detector);
      m_detectors.clear();
      break;
    }

    case TouchInputDown:
    {
      for (std::set<IGenericJoystickGestureDetector*>::const_iterator detector = m_detectors.begin(); detector != m_detectors.end(); detector++)
        (*detector)->OnTouchDown(pointer, m_pointers[pointer]);
      break;
    }

    case TouchInputUp:
    {
      for (std::set<IGenericJoystickGestureDetector*>::const_iterator detector = m_detectors.begin(); detector != m_detectors.end(); detector++)
        (*detector)->OnTouchUp(pointer, m_pointers[pointer]);
      break;
    }

    case TouchInputMove:
    {
      for (std::set<IGenericJoystickGestureDetector*>::const_iterator detector = m_detectors.begin(); detector != m_detectors.end(); detector++)
        (*detector)->OnTouchMove(pointer, m_pointers[pointer]);
      break;
    }

    default:
      return;
  }

  std::set<IGenericJoystickGestureDetector*> finishedDetectors;
  for (std::set<IGenericJoystickGestureDetector*>::const_iterator detector = m_detectors.begin(); detector != m_detectors.end(); detector++)
  {
    if ((*detector)->IsDone())
      finishedDetectors.insert(*detector);
  }

  for (std::set<IGenericJoystickGestureDetector*>::const_iterator detector = finishedDetectors.begin(); detector != finishedDetectors.end(); detector++)
  {
    m_detectors.erase(*detector);
    delete *detector;
  }
}
