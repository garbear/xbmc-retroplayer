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
#include "input/joysticks/generic/GenericJoystickInputHandler.h"
#include "peripherals/Peripherals.h"
#include "peripherals/bus/PeripheralBusAddon.h"
#include "utils/log.h"

using namespace PERIPHERALS;

CPeripheralJoystick::CPeripheralJoystick(const PeripheralScanResult& scanResult) :
  CPeripheral(scanResult),
  m_inputHandler(NULL)
{
  m_features.push_back(FEATURE_JOYSTICK);
}

CPeripheralJoystick::~CPeripheralJoystick(void)
{
  delete m_inputHandler; // TODO
}

bool CPeripheralJoystick::InitialiseFeature(const PeripheralFeature feature)
{
  if (!CPeripheral::InitialiseFeature(feature))
    return false;

  bool bReturn(false);

  if (feature == FEATURE_JOYSTICK)
  {
    if (m_busType == PERIPHERAL_BUS_ADDON)
    {
      CPeripheralBusAddon* addonBus = static_cast<CPeripheralBusAddon*>(g_peripherals.GetBusByType(PERIPHERAL_BUS_ADDON));
      if (addonBus)
      {
        PeripheralAddonPtr addon;
        unsigned int index;
        if (addonBus->SplitLocation(m_strLocation, addon, index))
        {
          //m_strDeviceName = m_addon->GetName(index); // TODO

          // TODO: Need a manager
          //m_inputHandler = m_addon->GetInputHandler(index); // TODO
          //m_inputHandler = new CGenericJoystickInputHandler(Index(), m_strDeviceName, m_iVendorId, m_iProductId);
          //m_inputHandler->SetButtonCount(joystickInfo.ButtonCount());
          //m_inputHandler->SetHatCount(joystickInfo.HatCount());
          //m_inputHandler->SetAxisCount(joystickInfo.AxisCount());
          //m_inputHandler->SetRequestedPort(joystickInfo.RequestedPlayer()); // TODO

          bReturn = true;
        }
        else
          CLog::Log(LOGERROR, "CPeripheralJoystick: Invalid location (%s)", m_strLocation.c_str());
      }
    }
    else
    {
      m_inputHandler = NULL; // TODO
      bReturn = true;
    }
  }

  return bReturn;
}

bool CPeripheralJoystick::HandleJoystickEvent(JoystickEvent event,
                                              unsigned int  index,
                                              int64_t       timeNs,
                                              bool          bPressed  /* = false */,
                                              HatDirection  direction /* = HatDirectionNone */,
                                              float         axisPos   /* = 0.0f */)
{
  if (m_inputHandler)
    return m_inputHandler->HandleJoystickEvent(event, index, timeNs, bPressed, direction, axisPos);
  return true;
}
