#pragma once
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

#include "Peripheral.h"
#include "input/joysticks/JoystickTypes.h"
#include "input/joysticks/IJoystickDriverHandler.h"
#include "input/joysticks/generic/DefaultJoystickInputHandler.h"

#include <string>
#include <vector>

#define JOYSTICK_PORT_UNKNOWN  (-1)

class IJoystickButtonMap;

namespace PERIPHERALS
{
  class CPeripheralJoystick : public CPeripheral, // TODO: extend CPeripheralHID
                              public IJoystickDriverHandler
  {
  public:
    CPeripheralJoystick(const PeripheralScanResult& scanResult);
    virtual ~CPeripheralJoystick(void);

    // implementation of CPeripheral
    virtual bool InitialiseFeature(const PeripheralFeature feature);
    virtual void RegisterJoystickDriverHandler(IJoystickDriverHandler* handler);
    virtual void UnregisterJoystickDriverHandler(IJoystickDriverHandler* handler);
    virtual void RegisterJoystickInputHandler(IJoystickInputHandler* handler);
    virtual void UnregisterJoystickInputHandler(IJoystickInputHandler* handler);

    // implementation of IJoystickDriverHandler
    virtual void OnButtonMotion(unsigned int buttonIndex, bool bPressed);
    virtual void OnHatMotion(unsigned int hatIndex, HatDirection direction);
    virtual void OnAxisMotion(unsigned int axisIndex, float position);
    virtual void ProcessAxisMotions(void);

    /*!
     * \brief Joystick is requesting a specific port number
     *
     * This could indicate that the joystick is connected to a hardware port
     * with a number label. Some controllers, such as the Xbox 360 controller,
     * also have LEDs that indicate the controller is on a specific port.
     *
     * \return The 0-indexed port number, or JOYSTICK_PORT_UNKNOWN if no port is requested
     */
    int RequestedPort(void) const { return m_requestedPort; }

    // TODO: Move to CPeripheral
    void SetDeviceName(const std::string& strName) { m_strDeviceName = strName; } // Override value in peripherals.xml

  protected:
    int                                  m_requestedPort;
    IJoystickButtonMap*                  m_buttonMap;
    IJoystickDriverHandler*              m_defaultDriverHandler;
    std::vector<IJoystickDriverHandler*> m_driverHandlers;
    CDefaultJoystickInputHandler         m_defaultInputHandler;
  };
}
