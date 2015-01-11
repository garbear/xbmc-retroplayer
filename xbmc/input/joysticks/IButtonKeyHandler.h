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

#include "IJoystickInputHandler.h"

/*!
 * \ingroup joysticks
 *
 * \brief Interface for handling joystick.xml button keys
 *
 * Keys can be mapped to analog actions (e.g. "AnalogSeekForward") or digital
 * actions (e.g. "Up").
 */
class IButtonKeyHandler
{
public:
  virtual ~IButtonKeyHandler(void) { }

  /*!
   * \brief Get the type of action mapped to the specified button key ID
   *
   * \param buttonKeyId  The button key ID from Key.h
   *
   * \return The type of action mapped to buttonKeyId, or INPUT_TYPE_UNKNOWN if
   *         no action is mapped to the specified key
   */
  virtual InputType GetInputType(unsigned int buttonKeyId) const = 0;

  /*!
   * \brief A button mapped to a digital action has been pressed or released
   *
   * \param buttonKeyId  The button key ID from Key.h
   * \param bPressed     true if the button is activated, false if deactivated
   */
  virtual void OnDigitalButtonKey(unsigned int buttonKeyId, bool bPressed) = 0;

  /*!
   * \brief Callback for keys mapped to analog actions
   *
   * \param buttonKeyId  The button key ID from Key.h
   * \param magnitude    The amount of the analog action
   *
   * If buttonKeyId is not mapped to an analog action, no action need be taken
   */
  virtual void OnAnalogButtonKey(unsigned int buttonKeyId, float magnitude) = 0;
};
