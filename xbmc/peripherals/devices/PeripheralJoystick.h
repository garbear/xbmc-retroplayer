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
#include "addons/include/xbmc_peripheral_utils.hpp"
#include "peripherals/PeripheralAddon.h"

class CKey;

namespace PERIPHERALS
{
  class JoystickEvent
  {
  public:
    JoystickEvent(JOYSTICK_EVENT_TYPE type = JOYSTICK_EVENT_TYPE_NONE, unsigned int peripheralIndex = 0)
    : m_type(type),
      m_peripheralIndex(peripheralIndex),
      m_virtualIndex(0),
      m_buttonId(),
      m_digitalState(),
      m_hatState(),
      m_analogState1(),
      m_analogState2(),
      m_analogState3()
    {
    }

    JoystickEvent(const PERIPHERAL_EVENT& event)
    : m_type(event.type),
      m_peripheralIndex(event.peripheral_index),
      m_virtualIndex(0),
      m_buttonId(),
      m_digitalState(),
      m_hatState(),
      m_analogState1(),
      m_analogState2(),
      m_analogState3()
    {
      switch (m_type)
      {
      case JOYSTICK_EVENT_TYPE_VIRTUAL_BUTTON:
        m_virtualIndex = event.virtual_index;
        SetDigitalState(event.digital_state);
        break;
      case JOYSTICK_EVENT_TYPE_VIRTUAL_HAT:
        m_virtualIndex = event.virtual_index;
        SetHatState(event.hat_state);
        break;
      case JOYSTICK_EVENT_TYPE_VIRTUAL_AXIS:
        m_virtualIndex = event.virtual_index;
        SetAnalogState(event.analog_state);
        break;
      case JOYSTICK_EVENT_TYPE_BUTTON_DIGITAL:
        m_buttonId = event.button_id;
        SetDigitalState(event.digital_state);
        break;
      case JOYSTICK_EVENT_TYPE_BUTTON_ANALOG:
        m_buttonId = event.button_id;
        SetAnalogState(event.analog_state);
        break;
      case JOYSTICK_EVENT_TYPE_ANALOG_STICK:
      case JOYSTICK_EVENT_TYPE_ANALOG_STICK_THRESHOLD:
        m_buttonId = event.button_id;
        SetAnalogStick(event.analog_stick.horiz, event.analog_stick.vert);
        break;
      case JOYSTICK_EVENT_TYPE_ACCELEROMETER:
        m_buttonId = event.button_id;
        SetAccelerometer(event.accelerometer.x, event.accelerometer.y, event.accelerometer.z);
        break;
      case JOYSTICK_EVENT_TYPE_NONE:
      default:
        break;
      }
    }

    virtual ~JoystickEvent(void) { }

    JOYSTICK_EVENT_TYPE   Type(void) const            { return m_type; }
    unsigned int          PeripheralIndex(void) const { return m_peripheralIndex; }
    unsigned int          VirtualIndex(void) const    { return m_virtualIndex; }
    JOYSTICK_ID           ButtonID(void) const        { return m_buttonId; }
    JOYSTICK_STATE_BUTTON DigitalState(void) const    { return m_digitalState; }
    JOYSTICK_STATE_HAT    HatState(void) const        { return m_hatState; }
    JOYSTICK_STATE_ANALOG AnalogState(void) const     { return m_analogState1; }
    JOYSTICK_STATE_ANALOG HorizontalState(void) const { return m_analogState1; }
    JOYSTICK_STATE_ANALOG VerticalState(void) const   { return m_analogState2; }
    JOYSTICK_STATE_ANALOG X(void) const               { return m_analogState1; }
    JOYSTICK_STATE_ANALOG Y(void) const               { return m_analogState2; }
    JOYSTICK_STATE_ANALOG Z(void) const               { return m_analogState3; }

    void SetType(JOYSTICK_EVENT_TYPE type)                { m_type            = type; }
    void SetPeripheralIndex(unsigned int peripheralIndex) { m_peripheralIndex = peripheralIndex; }
    void SetVirtualIndex(unsigned int virtualIndex)       { m_virtualIndex    = virtualIndex; }
    void SetButtonID(JOYSTICK_ID buttonId)                { m_buttonId        = buttonId; }

    void SetDigitalState(JOYSTICK_STATE_BUTTON digitalState)
    {
      m_digitalState = digitalState;
      m_analogState1 = digitalState ? 1.0f : 0.0f;
    }

    void SetHatState(JOYSTICK_STATE_HAT hatState)
    {
      m_hatState     = hatState;
      m_digitalState = hatState ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
    }

    void SetAnalogState(JOYSTICK_STATE_ANALOG analogState)
    {
      m_analogState1 = CONSTRAIN(-1.0f, analogState, 1.0f);
      m_digitalState = analogState > DIGITAL_ANALOG_THRESHOLD ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
    }

    void SetAnalogStick(JOYSTICK_STATE_ANALOG horizontal, JOYSTICK_STATE_ANALOG vertical)
    {
      m_analogState1 = CONSTRAIN(-1.0f, horizontal, 1.0f);
      m_analogState2 = CONSTRAIN(-1.0f, vertical,   1.0f);
      m_digitalState = std::sqrt(m_analogState1 * m_analogState1 +
                                 m_analogState2 * m_analogState2) > DIGITAL_ANALOG_THRESHOLD ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
    }

    void SetAccelerometer(JOYSTICK_STATE_ANALOG x, JOYSTICK_STATE_ANALOG y, JOYSTICK_STATE_ANALOG z)
    {
      m_analogState1 = CONSTRAIN(-1.0f, x, 1.0f);
      m_analogState2 = CONSTRAIN(-1.0f, y, 1.0f);
      m_analogState3 = CONSTRAIN(-1.0f, z, 1.0f);
    }

    void ToStruct(PERIPHERAL_EVENT& event) const
    {
      event.type             = m_type;
      event.peripheral_index = m_peripheralIndex;

      switch (m_type)
      {
      case JOYSTICK_EVENT_TYPE_VIRTUAL_BUTTON:
        event.virtual_index = m_virtualIndex;
        event.digital_state = m_digitalState;
        break;
      case JOYSTICK_EVENT_TYPE_VIRTUAL_HAT:
        event.virtual_index = m_virtualIndex;
        event.hat_state     = m_hatState;
        break;
      case JOYSTICK_EVENT_TYPE_VIRTUAL_AXIS:
        event.virtual_index = m_virtualIndex;
        event.analog_state  = m_analogState1;
        break;
      case JOYSTICK_EVENT_TYPE_BUTTON_DIGITAL:
        event.button_id     = m_buttonId;
        event.digital_state = m_digitalState;
        break;
      case JOYSTICK_EVENT_TYPE_BUTTON_ANALOG:
        event.button_id    = m_buttonId;
        event.analog_state = m_analogState1;
        break;
      case JOYSTICK_EVENT_TYPE_ANALOG_STICK:
      case JOYSTICK_EVENT_TYPE_ANALOG_STICK_THRESHOLD:
        event.button_id          = m_buttonId;
        event.analog_stick.horiz = m_analogState1;
        event.analog_stick.vert  = m_analogState2;
        break;
      case JOYSTICK_EVENT_TYPE_ACCELEROMETER:
        event.button_id       = m_buttonId;
        event.accelerometer.x = m_analogState1;
        event.accelerometer.y = m_analogState2;
        event.accelerometer.z = m_analogState3;
        break;
      case JOYSTICK_EVENT_TYPE_NONE:
      default:
        break;
      }
    }

    static void FreeStruct(PERIPHERAL_EVENT& event)
    {
      (void)event;
    }

  private:
    JOYSTICK_EVENT_TYPE    m_type;
    unsigned int           m_peripheralIndex;
    unsigned int           m_virtualIndex;
    JOYSTICK_ID            m_buttonId;
    JOYSTICK_STATE_BUTTON  m_digitalState;
    JOYSTICK_STATE_HAT     m_hatState;
    JOYSTICK_STATE_ANALOG  m_analogState1;
    JOYSTICK_STATE_ANALOG  m_analogState2;
    JOYSTICK_STATE_ANALOG  m_analogState3;
  };

  class IJoystickCallback
  {
  public:
    virtual ~IJoystickCallback(void) { }

    virtual void ButtonEvent(const JoystickEvent& event, const CJoystick* joystick);
  };

  class CPeripheralJoystick : public CPeripheral, // TODO: extend CPeripheralHID
                              public Observable
  {
  public:
    CPeripheralJoystick(const PeripheralScanResult& scanResult);
    virtual ~CPeripheralJoystick(void) { }

    virtual bool InitialiseFeature(const PeripheralFeature feature);

    unsigned int Index(void) const { return m_index; }

    /*
    unsigned int RequestedPort(void) const { return m_requestedPort; }
    unsigned int ButtonCount(void) const { return m_buttonCount; }
    unsigned int HatCount(void) const { return m_hatCount; }
    unsigned int AxisCount(void) const { return m_axisCount; }
    */

    void OnEvent(const ADDON::PeripheralEvent& event);
    unsigned int GetLastVirtualIndex(void) const { return m_lastVirtualIndex; }
    void SetLastVirtualIndex(unsigned int index);

  private:
    unsigned int       m_index;
    unsigned int       m_requestedPort;
    unsigned int       m_buttonCount;
    unsigned int       m_hatCount;
    unsigned int       m_axisCount;
    unsigned int       m_lastVirtualIndex;
    std::vector<CKey*> m_keys;
    PeripheralAddonPtr m_addon;
  };
}
