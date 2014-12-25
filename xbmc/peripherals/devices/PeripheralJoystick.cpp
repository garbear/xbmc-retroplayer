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
#include "addons/AddonManager.h"
#include "guilib/Key.h"s
#include "peripherals/Peripherals.h"
#include "peripherals/bus/PeripheralBusAddon.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

using namespace PERIPHERALS;

CPeripheralJoystick::CPeripheralJoystick(const PeripheralScanResult& scanResult) :
  CPeripheral(scanResult),
  m_index(0),
  m_requestedPort(0),
  m_buttonCount(0),
  m_hatCount(0),
  m_axisCount(0)
{
  CPeripheralBusAddon* addonBus = static_cast<CPeripheralBusAddon*>(g_peripherals.GetBusByType(PERIPHERAL_BUS_ADDON));
  if (addonBus)
  {
    if (!addonBus->SplitLocation(scanResult.m_strLocation, m_addon, m_index))
      CLog::Log(LOGERROR, "CPeripheralJoystick: Invalid location (%s)", scanResult.m_strLocation.c_str());
  }

  m_features.push_back(FEATURE_JOYSTICK);
}

bool CPeripheralJoystick::InitialiseFeature(const PeripheralFeature feature)
{
  if (!CPeripheral::InitialiseFeature(feature))
    return false;

  bool bReturn(false);

  if (feature == FEATURE_JOYSTICK)
  {
    ADDON::Joystick joystickInfo;
    if (m_addon && m_addon->GetJoystickInfo(Index(), joystickInfo))
    {
      bReturn = true;
      m_strDeviceName = joystickInfo.Name();
      m_requestedPort = joystickInfo.RequestedPlayer();
      m_buttonCount = joystickInfo.ButtonCount();
      m_hatCount = joystickInfo.HatCount();
      m_axisCount = joystickInfo.AxisCount();
    }
  }

  return bReturn;
}

/*
void CPeripheralJoystick::OnEvent(const PeripheralEvent& event)
{
  // TODO
  CLog::Log(LOGDEBUG, "PERIPHERAL - received event for %s", m_strDeviceName.c_str());
}
*/

void CPeripheralJoystick::GetKey(CKey& key)
{
  if (m_addon)
  {
    std::vector<ADDON::PeripheralEvent> events;
    if (m_addon->GetEvents(Index(), events))
    {
      for (std::vector<ADDON::PeripheralEvent>::const_iterator it = events.begin(); it != events.end(); ++it)
      {
        const ADDON::PeripheralEvent& event = *it;
        switch (event.Type())
        {
          case JOYSTICK_EVENT_TYPE_VIRTUAL_BUTTON:
          {
            break;
          }
          case JOYSTICK_EVENT_TYPE_VIRTUAL_HAT:

            break;
          case JOYSTICK_EVENT_TYPE_VIRTUAL_AXIS:

            break;
          case JOYSTICK_EVENT_TYPE_BUTTON_DIGITAL:
          {
            if (event.DigitalState() == JOYSTICK_STATE_BUTTON_PRESSED)
            {
              CKey newKey(cecDevice->GetButton(), cecDevice->GetHoldTime());
              JOYSTICK_ID id = event.ButtonID();
            }
            break;
          }
          case JOYSTICK_EVENT_TYPE_BUTTON_ANALOG:

            break;
          case JOYSTICK_EVENT_TYPE_ANALOG_STICK:

            break;
          case JOYSTICK_EVENT_TYPE_ANALOG_STICK_THRESHOLD:

            break;
          case JOYSTICK_EVENT_TYPE_ACCELEROMETER:

            break;
          case JOYSTICK_EVENT_TYPE_NONE:
          default:
            break;
        }
      }
    }

  }

}
