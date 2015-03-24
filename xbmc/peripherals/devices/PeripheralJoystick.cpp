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

CPeripheralJoystick::CPeripheralJoystick(const PeripheralScanResult& scanResult) :
  CPeripheral(scanResult),
  m_requestedPort(JOYSTICK_PORT_UNKNOWN),
  m_buttonMap(NULL),
  m_driverHandler(NULL)
{
  m_features.push_back(FEATURE_JOYSTICK);
}

CPeripheralJoystick::~CPeripheralJoystick(void)
{
  delete m_driverHandler;
  delete m_buttonMap;
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

          m_buttonMap = new CAddonJoystickButtonMap(addon, index);
          if (m_buttonMap->Load())
            m_driverHandler = new CGenericJoystickDriverHandler(this, m_buttonMap);
          else
            SAFE_DELETE(m_buttonMap);
        }
        else
          CLog::Log(LOGERROR, "CPeripheralJoystick: Invalid location (%s)", m_strLocation.c_str());
      }
    }
  }

  return m_driverHandler != NULL;
}

void CPeripheralJoystick::OnButtonMotion(unsigned int buttonIndex, bool bPressed)
{
  for (std::vector<IJoystickDriverHandler*>::iterator it = m_driverHandlers.begin(); it != m_driverHandlers.end(); ++it)
    (*it)->OnButtonMotion(buttonIndex, bPressed);

  if (m_driverHandler)
    m_driverHandler->OnButtonMotion(buttonIndex, bPressed);
}

void CPeripheralJoystick::OnHatMotion(unsigned int hatIndex, HatDirection direction)
{
  for (std::vector<IJoystickDriverHandler*>::iterator it = m_driverHandlers.begin(); it != m_driverHandlers.end(); ++it)
    (*it)->OnHatMotion(hatIndex, direction);

  if (m_driverHandler)
    m_driverHandler->OnHatMotion(hatIndex, direction);
}

void CPeripheralJoystick::OnAxisMotion(unsigned int axisIndex, float position)
{
  for (std::vector<IJoystickDriverHandler*>::iterator it = m_driverHandlers.begin(); it != m_driverHandlers.end(); ++it)
    (*it)->OnAxisMotion(axisIndex, position);

  if (m_driverHandler)
    m_driverHandler->OnAxisMotion(axisIndex, position);
}

void CPeripheralJoystick::ProcessAxisMotions(void)
{
  for (std::vector<IJoystickDriverHandler*>::iterator it = m_driverHandlers.begin(); it != m_driverHandlers.end(); ++it)
    (*it)->ProcessAxisMotions();

  if (m_driverHandler)
    m_driverHandler->ProcessAxisMotions();
}

bool CPeripheralJoystick::OnButtonPress(unsigned int featureIndex, bool bPressed)
{
  bool bHandled = false;

  for (std::vector<IJoystickInputHandler*>::iterator it = m_inputHandlers.begin(); it != m_inputHandlers.end(); ++it)
    bHandled |= (*it)->OnButtonPress(featureIndex, bPressed);

  return bHandled || m_fallbackHandler.OnButtonPress(featureIndex, bPressed);
}

bool CPeripheralJoystick::OnButtonMotion(unsigned int featureIndex, float magnitude)
{
  bool bHandled = false;

  for (std::vector<IJoystickInputHandler*>::iterator it = m_inputHandlers.begin(); it != m_inputHandlers.end(); ++it)
    bHandled |= (*it)->OnButtonMotion(featureIndex, magnitude);

  return bHandled || m_fallbackHandler.OnButtonMotion(featureIndex, magnitude);
}

bool CPeripheralJoystick::OnAnalogStickMotion(unsigned int featureIndex, float x, float y)
{
  bool bHandled = false;

  for (std::vector<IJoystickInputHandler*>::iterator it = m_inputHandlers.begin(); it != m_inputHandlers.end(); ++it)
    bHandled |= (*it)->OnAnalogStickMotion(featureIndex, x, y);

  return bHandled || m_fallbackHandler.OnAnalogStickMotion(featureIndex, x, y);
}

bool CPeripheralJoystick::OnAccelerometerMotion(unsigned int featureIndex, float x, float y, float z)
{
  bool bHandled = false;

  for (std::vector<IJoystickInputHandler*>::iterator it = m_inputHandlers.begin(); it != m_inputHandlers.end(); ++it)
    bHandled |= (*it)->OnAccelerometerMotion(featureIndex, x, y, z);

  return bHandled || m_fallbackHandler.OnAccelerometerMotion(featureIndex, x, y, z);
}

void CPeripheralJoystick::RegisterDriverHandler(IJoystickDriverHandler* handler)
{
  if (std::find(m_driverHandlers.begin(), m_driverHandlers.end(), handler) == m_driverHandlers.end())
    m_driverHandlers.push_back(handler);
}

void CPeripheralJoystick::UnregisterDriverHandler(IJoystickDriverHandler* handler)
{
  std::vector<IJoystickDriverHandler*>::iterator it = std::find(m_driverHandlers.begin(), m_driverHandlers.end(), handler);
  if (it != m_driverHandlers.end())
    m_driverHandlers.erase(it);
}

void CPeripheralJoystick::RegisterInputHandler(IJoystickInputHandler* handler)
{
  if (std::find(m_inputHandlers.begin(), m_inputHandlers.end(), handler) == m_inputHandlers.end())
    m_inputHandlers.push_back(handler);
}

void CPeripheralJoystick::UnregisterInputHandler(IJoystickInputHandler* handler)
{
  std::vector<IJoystickInputHandler*>::iterator it = std::find(m_inputHandlers.begin(), m_inputHandlers.end(), handler);
  if (it != m_inputHandlers.end())
    m_inputHandlers.erase(it);
}
