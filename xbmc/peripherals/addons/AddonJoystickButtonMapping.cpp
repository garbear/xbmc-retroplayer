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

#include "AddonJoystickButtonMapping.h"
#include "input/joysticks/generic/GenericJoystickButtonMapping.h"
#include "input/joysticks/IJoystickButtonMapper.h"
#include "peripherals/addons/AddonJoystickButtonMap.h"
#include "utils/log.h"

using namespace PERIPHERALS;

#ifndef SAFE_DELETE
  #define SATE_DELETE(x)  do { delete (x); (x) = NULL; } while (0)
#endif

CAddonJoystickButtonMapping::CAddonJoystickButtonMapping(CPeripheral* peripheral, IJoystickButtonMapper* mapper)
  : m_driverHandler(NULL)
{
  m_buttonMap = new CAddonJoystickButtonMap(peripheral, mapper->ControllerID());

  if (m_buttonMap->Load())
  {
    m_driverHandler = new CGenericJoystickButtonMapping(mapper, m_buttonMap);
    CLog::Log(LOGDEBUG, "Successfully loaded button map for controller %s", mapper->ControllerID().c_str());
  }
  else
  {
    CLog::Log(LOGERROR, "Failed to load button map for controller \"%s\"", mapper->ControllerID().c_str());
    SAFE_DELETE(m_buttonMap);
  }
}

CAddonJoystickButtonMapping::~CAddonJoystickButtonMapping(void)
{
  delete m_driverHandler;
  delete m_buttonMap;
}

bool CAddonJoystickButtonMapping::OnButtonMotion(unsigned int buttonIndex, bool bPressed)
{
  if (m_driverHandler)
    return m_driverHandler->OnButtonMotion(buttonIndex, bPressed);

  return false;
}

bool CAddonJoystickButtonMapping::OnHatMotion(unsigned int hatIndex, HatDirection direction)
{
  if (m_driverHandler)
    return m_driverHandler->OnHatMotion(hatIndex, direction);

  return false;
}

bool CAddonJoystickButtonMapping::OnAxisMotion(unsigned int axisIndex, float position)
{
  if (m_driverHandler)
    return m_driverHandler->OnAxisMotion(axisIndex, position);

  return false;
}

void CAddonJoystickButtonMapping::ProcessAxisMotions(void)
{
  if (m_driverHandler)
    m_driverHandler->ProcessAxisMotions();
}
