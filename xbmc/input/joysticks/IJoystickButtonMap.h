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
#pragma once

#include "JoystickDriverPrimitive.h"
#include "JoystickTypes.h"

#include <string>

/*!
 * \ingroup joysticks
 *
 * \brief Button map interface to translate between the driver's raw
 *        button/hat/axis elements and physical joystick features.
 *
 * \sa IJoystickButtonMapper
 */
class IJoystickButtonMap
{
public:
  virtual ~IJoystickButtonMap(void) { }

  /*!
   * \brief The add-on ID of the game controller associated with this button map
   *
   * The controller ID provided by the implementation serves as the context for the
   * feature indices below.
   *
   * \return The ID of this button map's game peripheral add-on
   */
  virtual std::string ControllerID(void) const = 0;

  /*!
   * \brief Load the button map into memory
   *
   * \return True if button map is ready to start translating buttons, false otherwise
   */
  virtual bool Load(void) = 0;

  /*!
   * \brief Get the feature associated with a driver primitive
   *
   * \param primitive    The driver primitive (a button, hat direction or semi-axis)
   * \param feature      The name of the resolved joystick feature, or
   *                         unmodified if GetFeature() returns false
   *
   * \return True if the driver primitive is associated with a feature, false otherwise
   */
   virtual bool GetFeature(const CJoystickDriverPrimitive& primitive, JoystickFeature& feature) = 0;

  /*!
   * \brief Get the driver primitive associated with a digital or analog button
   *
   * \param feature        Must be an analog or digital button or this will return false
   * \param primitive      The resolved driver primitive
   *
   * \return True if the index resolved to a driver primitive, false if the feature
   *         didn't resolve or isn't a digital or analog button
   */
  virtual bool GetPrimitiveFeature(const JoystickFeature& feature, CJoystickDriverPrimitive& primitive) = 0;

  /*!
   * \brief Add or update a digital or analog button
   *
   * \param feature        Must be an analog or digital button or this will return false
   * \param primitive      The driver primitive
   *
   * \return True if the feature was updated, false otherwise
   */
  virtual bool AddPrimitiveFeature(const JoystickFeature& feature, const CJoystickDriverPrimitive& primitive) = 0;

  /*!
   * \brief Get an analog stick from the button map
   *
   * \param feature  Must be an analog stick or this will return false
   * \param up       The primitive mapped to the up direction (possibly unknown)
   * \param down     The primitive mapped to the down direction (possibly unknown)
   * \param right    The primitive mapped to the right direction (possibly unknown)
   * \param left     The primitive mapped to the left direction (possibly unknown)
   *
   * It is not required that these primitives be axes. If a primitive is a
   * semiaxis, its opposite should point to the same axis index but with
   * opposite direction.
   *
   * \return True if the feature resolved to an analog stick with at least 1 known direction
   */
  virtual bool GetAnalogStick(const JoystickFeature& feature, CJoystickDriverPrimitive& up,
                                                              CJoystickDriverPrimitive& down,
                                                              CJoystickDriverPrimitive& right,
                                                              CJoystickDriverPrimitive& left) = 0;

  /*!
   * \brief Add or update an analog stick
   *
   * \param feature  Must be an analog stick or this will return false
   * \param up       The driver primitive for the up direction
   * \param down     The driver primitive for the down direction
   * \param right    The driver primitive for the right direction
   * \param left     The driver primitive for the left direction
   *
   * \return True if the analog stick was updated, false otherwise
   */
  virtual bool AddAnalogStick(const JoystickFeature& feature, const CJoystickDriverPrimitive& up,
                                                              const CJoystickDriverPrimitive& down,
                                                              const CJoystickDriverPrimitive& right,
                                                              const CJoystickDriverPrimitive& left) = 0;

  /*!
   * \brief Get an accelerometer from the button map
   *
   * \param feature       Must be an accelerometer or this will return false
   * \param positiveX     The semiaxis mapped to the positive X direction (possibly unknown)
   * \param positiveY     The semiaxis mapped to the positive Y direction (possibly unknown)
   * \param positiveZ     The semiaxis mapped to the positive Z direction (possibly unknown)
   *
   * \return True if the feature resolved to an accelerometer with at least 1 known axis
   */
  virtual bool GetAccelerometer(const JoystickFeature& feature, CJoystickDriverPrimitive& positiveX,
                                                                CJoystickDriverPrimitive& positiveY,
                                                                CJoystickDriverPrimitive& positiveZ) = 0;

  /*!
   * \brief Get or update an accelerometer
   *
   * \param feature       Must be an accelerometer or this will return false
   * \param positiveX     The semiaxis corresponding to the positive X direction
   * \param positiveY     The semiaxis corresponding to the positive Y direction
   * \param positiveZ     The semiaxis corresponding to the positive Z direction
   *
   * If the driver primitives are not mapped to a semiaxis, they will be ignored.
   *
   * \return True if the accelerometer was updated, false otherwise
   */
  virtual bool AddAccelerometer(const JoystickFeature& feature, const CJoystickDriverPrimitive& positiveX,
                                                                const CJoystickDriverPrimitive& positiveY,
                                                                const CJoystickDriverPrimitive& positiveZ) = 0;
};
