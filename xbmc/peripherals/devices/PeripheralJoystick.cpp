/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "PeripheralJoystick.h"
#include "input/joysticks/generic/GenericJoystickDriverHandler.h"
#include "peripherals/Peripherals.h"
#include "peripherals/addons/AddonJoystickButtonMap.h"
#include "peripherals/bus/PeripheralBusAddon.h"
#include "utils/log.h"

#include <algorithm>

using namespace PERIPHERALS;

#ifndef SAFE_DELETE
#define SATE_DELETE(x)  do { delete (x); (x) = NULL; } while (0)
#endif

// --- CPeripheralJoystickInput ------------------------------------------------

CPeripheralJoystickInput::CPeripheralJoystickInput(IJoystickDriverHandler* handler)
  : m_driverHandler(handler),
    m_buttonMap(NULL),
    m_inputHandler(NULL),
    m_bOwnsDriverHandler(false)
{
}

CPeripheralJoystickInput::CPeripheralJoystickInput(CPeripheralJoystick* joystick, IJoystickInputHandler* handler)
  : m_driverHandler(NULL),
    m_inputHandler(handler),
    m_bOwnsDriverHandler(true)
{
  m_buttonMap = new CAddonJoystickButtonMap(joystick, handler->DeviceID());
  if (m_buttonMap->Load())
    m_driverHandler = new CGenericJoystickDriverHandler(handler, m_buttonMap);
  else
    SAFE_DELETE(m_buttonMap);
}

CPeripheralJoystickInput::~CPeripheralJoystickInput(void)
{
  if (m_bOwnsDriverHandler)
  {
    delete m_driverHandler;
    delete m_buttonMap;
  }
}

// --- CPeripheralJoystick -----------------------------------------------------

CPeripheralJoystick::CPeripheralJoystick(const PeripheralScanResult& scanResult) :
  CPeripheral(scanResult),
  m_requestedPort(JOYSTICK_PORT_UNKNOWN)
{
  m_features.push_back(FEATURE_JOYSTICK);
}

CPeripheralJoystick::~CPeripheralJoystick(void)
{
  for (std::vector<CPeripheralJoystickInput*>::iterator it = m_input.begin(); it != m_input.end(); ++it)
    delete *it;
}

bool CPeripheralJoystick::InitialiseFeature(const PeripheralFeature feature)
{
  if (!CPeripheral::InitialiseFeature(feature))
    return false;

  if (feature == FEATURE_JOYSTICK)
  {
    if (m_mappedBusType == PERIPHERAL_BUS_ADDON)
    {
      CPeripheralBusAddon* addonBus = static_cast<CPeripheralBusAddon*>(g_peripherals.GetBusByType(PERIPHERAL_BUS_ADDON));
      if (addonBus)
      {
        PeripheralAddonPtr addon;
        unsigned int index;
        if (addonBus->SplitLocation(m_strLocation, addon, index))
        {
          m_requestedPort = addon->GetRequestedPort(index);

          m_input.push_back(new CPeripheralJoystickInput(this, &m_defaultInputHandler));
        }
        else
          CLog::Log(LOGERROR, "CPeripheralJoystick: Invalid location (%s)", m_strLocation.c_str());
      }
    }
  }

  return !m_input.empty();
}

void CPeripheralJoystick::RegisterJoystickDriverHandler(IJoystickDriverHandler* handler)
{
  bool bFound = false;

  for (std::vector<CPeripheralJoystickInput*>::iterator it = m_input.begin(); it != m_input.end(); ++it)
  {
    if ((*it)->GetDriverHandler() == handler)
    {
      bFound = true;
      break;
    }
  }

  if (!bFound)
    m_input.push_back(new CPeripheralJoystickInput(handler));
}

void CPeripheralJoystick::UnregisterJoystickDriverHandler(IJoystickDriverHandler* handler)
{
  for (std::vector<CPeripheralJoystickInput*>::iterator it = m_input.begin(); it != m_input.end(); ++it)
  {
    if ((*it)->GetDriverHandler() == handler)
    {
      delete *it;
      m_input.erase(it);
      break;
    }
  }
}

void CPeripheralJoystick::RegisterJoystickInputHandler(IJoystickInputHandler* handler)
{
  bool bFound = false;

  for (std::vector<CPeripheralJoystickInput*>::iterator it = m_input.begin(); it != m_input.end(); ++it)
  {
    if ((*it)->GetInputHandler() == handler)
    {
      bFound = true;
      break;
    }
  }

  if (!bFound)
    m_input.push_back(new CPeripheralJoystickInput(this, handler));
}

void CPeripheralJoystick::UnregisterJoystickInputHandler(IJoystickInputHandler* handler)
{
  for (std::vector<CPeripheralJoystickInput*>::iterator it = m_input.begin(); it != m_input.end(); ++it)
  {
    if ((*it)->GetInputHandler() == handler)
    {
      delete *it;
      m_input.erase(it);
      break;
    }
  }
}

void CPeripheralJoystick::OnButtonMotion(unsigned int buttonIndex, bool bPressed)
{
  for (std::vector<CPeripheralJoystickInput*>::iterator it = m_input.begin(); it != m_input.end(); ++it)
    (*it)->GetDriverHandler()->OnButtonMotion(buttonIndex, bPressed);
}

void CPeripheralJoystick::OnHatMotion(unsigned int hatIndex, HatDirection direction)
{
  for (std::vector<CPeripheralJoystickInput*>::iterator it = m_input.begin(); it != m_input.end(); ++it)
    (*it)->GetDriverHandler()->OnHatMotion(hatIndex, direction);
}

void CPeripheralJoystick::OnAxisMotion(unsigned int axisIndex, float position)
{
  for (std::vector<CPeripheralJoystickInput*>::iterator it = m_input.begin(); it != m_input.end(); ++it)
    (*it)->GetDriverHandler()->OnAxisMotion(axisIndex, position);
}

void CPeripheralJoystick::ProcessAxisMotions(void)
{
  for (std::vector<CPeripheralJoystickInput*>::iterator it = m_input.begin(); it != m_input.end(); ++it)
    (*it)->GetDriverHandler()->ProcessAxisMotions();
}
