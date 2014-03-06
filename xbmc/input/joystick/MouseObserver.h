#pragma once
/*
 *      Copyright (C) 2007-2014 Team XBMC
 *      http://www.xbmc.org
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

#include "input/IInputHandler.h"

namespace INPUT
{

class CMouseObserver : public IInputHandler
{
public:
  virtual void ProcessKeyDown(unsigned int controllerID, uint32_t key, const CAction &action);
  virtual void ProcessKeyUp(unsigned int controllerID, uint32_t key);
  virtual void ProcessButtonDown(unsigned int controllerID, unsigned int buttonID, const CAction &action);
  virtual void ProcessButtonUp(unsigned int controllerID, unsigned int buttonID);
  virtual void ProcessDigitalAxisDown(unsigned int controllerID, unsigned int buttonID, const CAction &action);
  virtual void ProcessDigitalAxisUp(unsigned int controllerID, unsigned int buttonID);
  virtual void ProcessHatDown(unsigned int controllerID, unsigned int hatID, unsigned char dir, const CAction &action);
  virtual void ProcessHatUp(unsigned int controllerID, unsigned int hatID, unsigned char dir);
  virtual void ProcessAnalogAxis(unsigned int controllerID, unsigned int axisID, const CAction &action);
};

}
