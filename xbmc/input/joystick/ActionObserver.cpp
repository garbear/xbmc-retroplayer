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

#include "ActionObserver.h"
#include "JoystickManager.h"
#include "Application.h"

using namespace INPUT;

void CActionObserver::ProcessButtonDown(unsigned int controllerID, unsigned int buttonID, const CAction &action)
{
  // Ignore button press if we woke up the screensaver
  if (!CJoystickManager::Get().Wakeup())
    g_application.ExecuteInputAction(action);

  // Track the button press for deferred repeated execution
  CJoystickManager::Get().Track(action);
}

void CActionObserver::ProcessButtonUp(unsigned int controllerID, unsigned int buttonID)
{
  CJoystickManager::Get().ResetActionRepeater(); // If a button was released, reset the tracker
}

void CActionObserver::ProcessDigitalAxisDown(unsigned int controllerID, unsigned int buttonID, const CAction &action)
{
  // TODO
}

void CActionObserver::ProcessDigitalAxisUp(unsigned int controllerID, unsigned int buttonID)
{
  // TODO}
}

void CActionObserver::ProcessHatDown(unsigned int controllerID, unsigned int hatID, unsigned char dir, const CAction &action)
{
  // TODO}
}

void CActionObserver::ProcessHatUp(unsigned int controllerID, unsigned int hatID, unsigned char dir)
{
  // TODO
}

void CActionObserver::ProcessAnalogAxis(unsigned int controllerID, unsigned int axisID, const CAction &action)
{
  // TODO
}
