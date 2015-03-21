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
#include "input/joysticks/generic/GenericJoystickInputHandler.h"

#include <string>
#include <vector>

#define JOYSTICK_PORT_UNKNOWN  0

class IJoystickButtonMap;

namespace PERIPHERALS
{
  class CPeripheralJoystick : public CPeripheral, // TODO: extend CPeripheralHID
                              public IJoystickDriverHandler,
                              public IJoystickInputHandler
  {
  public:
    CPeripheralJoystick(const PeripheralScanResult& scanResult);
    virtual ~CPeripheralJoystick(void);

    // implementation of CPeripheral
    virtual bool InitialiseFeature(const PeripheralFeature feature);

    // implementation of IJoystickDriverHandler
    virtual void OnButtonMotion(unsigned int index, bool bPressed);
    virtual void OnHatMotion(unsigned int index, HatDirection direction);
    virtual void OnAxisMotion(unsigned int index, float position);
    virtual void ProcessAxisMotions(void);

    // implementation of IJoystickInputHandler
    virtual bool OnButtonPress(JoystickFeatureID id, bool bPressed);
    virtual bool OnButtonMotion(JoystickFeatureID id, float magnitude);
    virtual bool OnAnalogStickMotion(JoystickFeatureID id, float x, float y);
    virtual bool OnAccelerometerMotion(JoystickFeatureID id, float x, float y, float z);

    int RequestedPort(void) const { return m_requestedPort; }

    IJoystickButtonMap* GetButtonMap(void) { return m_buttonMap; }

    // TODO: Move to CPeripheral
    void SetDeviceName(const std::string& strName) { m_strDeviceName = strName; } // Override value in peripherals.xml

    void RegisterDriverHandler(IJoystickDriverHandler* handler);
    void UnregisterDriverHandler(IJoystickDriverHandler* handler);

    void RegisterInputHandler(IJoystickInputHandler* handler);
    void UnregisterInputHandler(IJoystickInputHandler* handler);

  protected:
    int                                 m_requestedPort;
    IJoystickButtonMap*                 m_buttonMap;
    std::vector<IJoystickDriverHandler*> m_driverHandlers;
    std::vector<IJoystickInputHandler*> m_inputHandlers;
    IJoystickDriverHandler*             m_driverHandler;
    CGenericJoystickInputHandler        m_fallbackHandler;
  };
}
