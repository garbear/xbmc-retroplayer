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

#include "input/joysticks/IJoystickInputHandling.h"
//#include "input/joysticks/JoystickTypes.h"

/*!
 * \ingroup joysticks_generic
 * \brief Interface defining methods to perform gesture detection
 */
class IGenericJoystickGestureDetector : public IJoystickInputHandling
{
public:
  IGenericJoystickGestureDetector(IJoystickActionHandler *handler)
    : m_done(false)
  {
    RegisterHandler(handler);
  }

  virtual ~IGenericJoystickGestureDetector() { }

  /*!
   * \brief Check whether the gesture recognition is finished or not
   *
   * \return True if the gesture recognition is finished otherwise false
   */
  bool IsDone() { return m_done; }

protected:
  /*!
   * \brief Whether the gesture recognition is finished or not
   */
  bool m_done;

  /*!
   * \brief DPI value of the touch screen
   */
  //float m_dpi;

  /*!
   * \brief Local list of all known touch pointers
   */
  //Pointer m_pointers[TOUCH_MAX_POINTERS];
};
