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

#include "MouseObserver.h"
#include "input/MouseStat.h"

using namespace INPUT;

void CMouseObserver::ProcessKeyDown(unsigned int controllerID, uint32_t key, const CAction &action)
{
  g_Mouse.SetActive(false);
}

void CMouseObserver::ProcessKeyUp(unsigned int controllerID, uint32_t key)
{
  g_Mouse.SetActive(false);
}

void CMouseObserver::ProcessButtonDown(unsigned int controllerID, unsigned int buttonID, const CAction &action)
{
  g_Mouse.SetActive(false);
}

void CMouseObserver::ProcessButtonUp(unsigned int controllerID, unsigned int buttonID)
{
  g_Mouse.SetActive(false);
}

void CMouseObserver::ProcessDigitalAxisDown(unsigned int controllerID, unsigned int buttonID, const CAction &action)
{
  g_Mouse.SetActive(false);
}

void CMouseObserver::ProcessDigitalAxisUp(unsigned int controllerID, unsigned int buttonID)
{
  g_Mouse.SetActive(false);
}

void CMouseObserver::ProcessHatDown(unsigned int controllerID, unsigned int hatID, unsigned char dir, const CAction &action)
{
  g_Mouse.SetActive(false);
}

void CMouseObserver::ProcessHatUp(unsigned int controllerID, unsigned int hatID, unsigned char dir)
{
  g_Mouse.SetActive(false);
}

void CMouseObserver::ProcessAnalogAxis(unsigned int controllerID, unsigned int axisID, const CAction &action)
{
  g_Mouse.SetActive(false);
}
