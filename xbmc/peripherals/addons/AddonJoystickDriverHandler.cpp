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

#include "AddonJoystickDriverHandler.h"
#include "input/joysticks/generic/GenericJoystickDriverHandler.h"
#include "input/joysticks/IJoystickInputHandler.h"
#include "peripherals/addons/AddonJoystickButtonMap.h"

using namespace PERIPHERALS;

#ifndef SAFE_DELETE
  #define SATE_DELETE(x)  do { delete (x); (x) = NULL; } while (0)
#endif

CAddonJoystickDriverHandler::CAddonJoystickDriverHandler(CPeripheral* peripheral, IJoystickInputHandler* handler)
  : m_driverHandler(NULL)
{
  m_buttonMap = new CAddonJoystickButtonMap(peripheral, handler->DeviceID());
  if (m_buttonMap->Load())
    m_driverHandler = new CGenericJoystickDriverHandler(handler, m_buttonMap);
  else
    SAFE_DELETE(m_buttonMap);
}

CAddonJoystickDriverHandler::~CAddonJoystickDriverHandler(void)
{
  delete m_driverHandler;
  delete m_buttonMap;
}

void CAddonJoystickDriverHandler::OnButtonMotion(unsigned int buttonIndex, bool bPressed)
{
  if (m_driverHandler)
    m_driverHandler->OnButtonMotion(buttonIndex, bPressed);
}

void CAddonJoystickDriverHandler::OnHatMotion(unsigned int hatIndex, HatDirection direction)
{
  if (m_driverHandler)
    m_driverHandler->OnHatMotion(hatIndex, direction);
}

void CAddonJoystickDriverHandler::OnAxisMotion(unsigned int axisIndex, float position)
{
  if (m_driverHandler)
    m_driverHandler->OnAxisMotion(axisIndex, position);
}

void CAddonJoystickDriverHandler::ProcessAxisMotions(void)
{
  if (m_driverHandler)
    m_driverHandler->ProcessAxisMotions();
}
