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

    // implementation of IJoystickDriverHandler
    virtual void OnButtonMotion(unsigned int buttonIndex, bool bPressed);
    virtual void OnHatMotion(unsigned int hatIndex, HatDirection direction);
    virtual void OnAxisMotion(unsigned int axisIndex, float position);
    virtual void ProcessAxisMotions(void);

    /*!
     * \brief Get the name of the driver or API providing this joystick
     */
    const std::string& Provider(void) const { return m_strProvider; }

    /*!
     * \brief Get the specific port number requested by this joystick
     *
     * This could indicate that the joystick is connected to a hardware port
     * with a number label; some controllers, such as the Xbox 360 controller,
     * also have LEDs that indicate the controller is on a specific port.
     *
     * \return The 0-indexed port number, or JOYSTICK_PORT_UNKNOWN if no port is requested
     */
    int RequestedPort(void) const { return m_requestedPort; }

    /*!
     * \brief Get the number of elements reported by the driver
     */
    unsigned int ButtonCount(void) const { return m_buttonCount; }
    unsigned int HatCount(void) const    { return m_hatCount; }
    unsigned int AxisCount(void) const   { return m_axisCount; }

    // TODO: Move to CPeripheral
    void SetDeviceName(const std::string& strName) { m_strDeviceName = strName; } // Override value in peripherals.xml

    void SetProvider(const std::string& provider) { m_strProvider   = provider; }
    void SetRequestedPort(int port)               { m_requestedPort = port; }
    void SetButtonCount(unsigned int buttonCount) { m_buttonCount   = buttonCount; }
    void SetHatCount(unsigned int hatCount)       { m_hatCount      = hatCount; }
    void SetAxisCount(unsigned int axisCount)     { m_axisCount     = axisCount; }

  protected:
    std::string                          m_strProvider;
    int                                  m_requestedPort;
    unsigned int                         m_buttonCount;
    unsigned int                         m_hatCount;
    unsigned int                         m_axisCount;
    CDefaultJoystickInputHandler         m_defaultInputHandler;
    std::vector<IJoystickDriverHandler*> m_driverHandlers;
  };
}
