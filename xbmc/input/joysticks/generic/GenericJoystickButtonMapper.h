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

#include "IGenericJoystickButtonMapper.h"

class CGenericJoystickButtonMapper : public IGenericJoystickButtonMapper
{
public:
  CGenericJoystickButtonMapper(IJoystickActionHandler *handler, const std::string &strName)
    : IGenericJoystickButtonMapper(handler, strName)
  {
  }

  virtual ~CGenericJoystickButtonMapper() { }

  // Implementation of IGenericJoystickButtonMapper
  virtual bool OnRawButtonPress(unsigned int id, bool bPressed);
  virtual bool OnRawHatMotion(unsigned int id, HatDirection direction);
  virtual bool OnRawAxisMotion(unsigned int id, float position);

private:
};
