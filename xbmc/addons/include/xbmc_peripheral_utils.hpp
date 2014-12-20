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
#pragma once

#include "xbmc_peripheral_types.h"

#include <string>
#include <string.h>
#include <vector>

namespace ADDON
{
  class JoystickButton
  {
  public:
    JoystickButton(void) : m_id(), m_type() { }

    JoystickButton(JOYSTICK_BUTTON_ID id, JOYSTICK_BUTTON_TYPE type, const std::string& strLabel)
      : m_id(id), m_type(type), m_strLabel(strLabel) { }

    JoystickButton(const JOYSTICK_BUTTON& button)
      : m_id(button.id), m_type(button.type), m_strLabel(button.label ? button.label : "") { }

    JOYSTICK_BUTTON_ID   ID(void) const    { return m_id; }
    JOYSTICK_BUTTON_TYPE Type(void) const  { return m_type; }
    const std::string&   Label(void) const { return m_strLabel; }

    void SetID(JOYSTICK_BUTTON_ID id)          { m_id = id; }
    void SetType(JOYSTICK_BUTTON_TYPE type)    { m_type = type; }
    void SetLabel(const std::string& strLabel) { m_strLabel = strLabel; }

    void ToStruct(JOYSTICK_BUTTON& button)
    {
      button.id = m_id;
      button.type = m_type;
      button.label = new char[m_strLabel.size() + 1];
      strcpy(button.label, m_strLabel.c_str());
    }

    static void FreeStruct(JOYSTICK_BUTTON& button)
    {
      delete[] button.label;
      button.label = NULL;
    }

  private:
    JOYSTICK_BUTTON_ID   m_id;
    JOYSTICK_BUTTON_TYPE m_type;
    std::string          m_strLabel;
  };

  class JoystickAnalogStick
  {
  public:
    JoystickAnalogStick(void) : m_id() { }

    JoystickAnalogStick(JOYSTICK_ANALOG_STICK_ID id, const std::string& strLabel)
      : m_id(id), m_strLabel(strLabel) { }

    JoystickAnalogStick(const JOYSTICK_ANALOG_STICK& analogStick)
      : m_id(analogStick.id), m_strLabel(analogStick.label ? analogStick.label : "") { }

    JOYSTICK_ANALOG_STICK_ID ID(void) const    { return m_id; }
    const std::string&       Label(void) const { return m_strLabel; }

    void SetID(JOYSTICK_ANALOG_STICK_ID id)    { m_id = id; }
    void SetLabel(const std::string& strLabel) { m_strLabel = strLabel; }

    void ToStruct(JOYSTICK_ANALOG_STICK& analogStick)
    {
      analogStick.id = m_id;
      analogStick.label = new char[m_strLabel.size() + 1];
      strcpy(analogStick.label, m_strLabel.c_str());
    }

    static void FreeStruct(JOYSTICK_ANALOG_STICK& analogStick)
    {
      delete[] analogStick.label;
      analogStick.label = NULL;
    }

  private:
    JOYSTICK_ANALOG_STICK_ID m_id;
    std::string              m_strLabel;
  };

  class JoystickInfo
  {
  public:
    JoystickInfo(void) : m_requestedPlayer(0), m_buttonCount(0), m_hatCount(0), m_axisCount(0) { }

    JoystickInfo(JOYSTICK_INFO& info)
    : m_strName(info.name),
      m_requestedPlayer(info.requested_player_num),
      m_buttonCount(info.virtual_layout.button_count),
      m_hatCount(info.virtual_layout.hat_count),
      m_axisCount(info.virtual_layout.axis_count)
    {
      for (unsigned int i = 0; i < info.physical_layout.button_count; i++)
        m_buttons.push_back(JoystickButton(info.physical_layout.buttons[i]));

      for (unsigned int i = 0; i < info.physical_layout.analog_stick_count; i++)
        m_analogSticks.push_back(JoystickAnalogStick(info.physical_layout.analog_sticks[i]));
    }

    const std::string& Name(void) const            { return m_strName; }
    unsigned int       RequestedPlayer(void) const { return m_requestedPlayer; }
    unsigned int       ButtonCount(void) const     { return m_buttonCount; }
    unsigned int       HatCount(void) const        { return m_hatCount; }
    unsigned int       AxisCount(void) const       { return m_axisCount; }

    const std::vector<JoystickButton>&      Buttons(void) const      { return m_buttons; }
    const std::vector<JoystickAnalogStick>& AnalogSticks(void) const { return m_analogSticks; }

    void SetName(const std::string& strName)              { m_strName         = strName; }
    void SetRequestedPlayer(unsigned int requestedPlayer) { m_requestedPlayer = requestedPlayer; }
    void SetButtonCount(unsigned int buttonCount)         { m_buttonCount     = buttonCount; }
    void SetHatCount(unsigned int hatCount)               { m_hatCount        = hatCount; }
    void SetAxisCount(unsigned int axisCount)             { m_axisCount       = axisCount; }

    std::vector<JoystickButton>&      Buttons(void)      { return m_buttons; }
    std::vector<JoystickAnalogStick>& AnalogSticks(void) { return m_analogSticks; }

    void ToStruct(JOYSTICK_INFO& info)
    {
      info.name                               = new char[m_strName.size() + 1];
      strcpy(info.name, m_strName.c_str());
      info.requested_player_num               = m_requestedPlayer;
      info.virtual_layout.button_count        = m_buttonCount;
      info.virtual_layout.hat_count           = m_hatCount;
      info.virtual_layout.axis_count          = m_axisCount;
      info.physical_layout.button_count       = m_buttons.size();
      info.physical_layout.buttons            = NULL;
      info.physical_layout.analog_stick_count = m_analogSticks.size();
      info.physical_layout.analog_sticks      = NULL;

      if (m_buttons.size())
      {
        info.physical_layout.buttons = new JOYSTICK_BUTTON[m_buttons.size()];
        for (unsigned int i = 0; i < m_buttons.size(); i++)
          m_buttons[i].ToStruct(info.physical_layout.buttons[i]);
      }

      if (m_analogSticks.size())
      {
        info.physical_layout.analog_sticks = new JOYSTICK_ANALOG_STICK[m_analogSticks.size()];
        for (unsigned int i = 0; i < m_analogSticks.size(); i++)
          m_analogSticks[i].ToStruct(info.physical_layout.analog_sticks[i]);
      }
    }

    void FreeStruct(JOYSTICK_INFO& info)
    {
      delete[] info.name;
      info.name = NULL;

      for (unsigned int i = 0; i < info.physical_layout.button_count; i++)
        JoystickButton::FreeStruct(info.physical_layout.buttons[i]);
      delete[] info.physical_layout.buttons;
      info.physical_layout.buttons = NULL;

      for (unsigned int i = 0; i < info.physical_layout.analog_stick_count; i++)
        JoystickAnalogStick::FreeStruct(info.physical_layout.analog_sticks[i]);
      delete[] info.physical_layout.analog_sticks;
      info.physical_layout.analog_sticks = NULL;
    }

  private:
    std::string  m_strName;
    unsigned int m_requestedPlayer;
    unsigned int m_buttonCount;
    unsigned int m_hatCount;
    unsigned int m_axisCount;

    std::vector<JoystickButton>      m_buttons;
    std::vector<JoystickAnalogStick> m_analogSticks;
  };

  class PeripheralEvent
  {
  public:
    PeripheralEvent(void) : m_index(0), m_type(), m_data(NULL) { }

    PeripheralEvent(const PERIPHERAL_EVENT& event)
    : m_index(event.peripheral_index),
      m_type(event.type),
      m_data(NULL)
    {
      SetData(event.event_data);
    }

    ~PeripheralEvent(void)
    {
      ClearData();
    }

    unsigned int        PeripheralIndex(void) const { return m_index; }
    JOYSTICK_EVENT_TYPE Type(void) const            { return m_type; }

    template <typename EVENT_TYPE>
    const EVENT_TYPE& EventAsType(void) const
    {
      static const EVENT_TYPE emptyEvent = { };
      return m_data ? *static_cast<const EVENT_TYPE*>(m_data) : emptyEvent;
    }

    void ClearData(void)
    {
      PERIPHERAL_EVENT event = { m_index, m_type, m_data };
      FreeStruct(event);
    }

    void SetData(void* event)
    {
      ClearData();

      if (event)
      {
        switch (m_type)
        {
          case JOYSTICK_EVENT_TYPE_VIRTUAL_BUTTON:
          {
            JOYSTICK_EVENT_VIRTUAL_BUTTON* setEvent = static_cast<JOYSTICK_EVENT_VIRTUAL_BUTTON*>(event);
            JOYSTICK_EVENT_VIRTUAL_BUTTON* newEvent = new JOYSTICK_EVENT_VIRTUAL_BUTTON;
            *newEvent = *setEvent;
            m_data = newEvent;
            break;
          }
          case JOYSTICK_EVENT_TYPE_VIRTUAL_HAT:
          {
            JOYSTICK_EVENT_VIRTUAL_HAT* setEvent = static_cast<JOYSTICK_EVENT_VIRTUAL_HAT*>(event);
            JOYSTICK_EVENT_VIRTUAL_HAT* newEvent = new JOYSTICK_EVENT_VIRTUAL_HAT;
            *newEvent = *setEvent;
            m_data = newEvent;
            break;
          }
          case JOYSTICK_EVENT_TYPE_VIRTUAL_AXIS:
          {
            JOYSTICK_EVENT_VIRTUAL_AXIS* setEvent = static_cast<JOYSTICK_EVENT_VIRTUAL_AXIS*>(event);
            JOYSTICK_EVENT_VIRTUAL_AXIS* newEvent = new JOYSTICK_EVENT_VIRTUAL_AXIS;
            *newEvent = *setEvent;
            m_data = newEvent;
            break;
          }
          case JOYSTICK_EVENT_TYPE_BUTTON_DIGITAL:
          {
            JOYSTICK_EVENT_BUTTON_DIGITAL* setEvent = static_cast<JOYSTICK_EVENT_BUTTON_DIGITAL*>(event);
            JOYSTICK_EVENT_BUTTON_DIGITAL* newEvent = new JOYSTICK_EVENT_BUTTON_DIGITAL;
            *newEvent = *setEvent;
            m_data = newEvent;
            break;
          }
          case JOYSTICK_EVENT_TYPE_BUTTON_ANALOG:
          {
            JOYSTICK_EVENT_BUTTON_ANALOG* setEvent = static_cast<JOYSTICK_EVENT_BUTTON_ANALOG*>(event);
            JOYSTICK_EVENT_BUTTON_ANALOG* newEvent = new JOYSTICK_EVENT_BUTTON_ANALOG;
            *newEvent = *setEvent;
            m_data = newEvent;
            break;
          }
          case JOYSTICK_EVENT_TYPE_ANALOG_STICK:
          {
            JOYSTICK_EVENT_ANALOG_STICK* setEvent = static_cast<JOYSTICK_EVENT_ANALOG_STICK*>(event);
            JOYSTICK_EVENT_ANALOG_STICK* newEvent = new JOYSTICK_EVENT_ANALOG_STICK;
            *newEvent = *setEvent;
            m_data = newEvent;
            break;
          }
          case JOYSTICK_EVENT_TYPE_ACCELEROMETER:
          {
            JOYSTICK_EVENT_ACCELEROMETER* setEvent = static_cast<JOYSTICK_EVENT_ACCELEROMETER*>(event);
            JOYSTICK_EVENT_ACCELEROMETER* newEvent = new JOYSTICK_EVENT_ACCELEROMETER;
            *newEvent = *setEvent;
            m_data = newEvent;
            break;
          }
          case JOYSTICK_EVENT_TYPE_NONE:
          default:
            break;
        }
      }
    }

    static void ToStructs(const std::vector<PeripheralEvent>& events, PERIPHERAL_EVENT** eventStructs)
    {
      if (!events.empty() && eventStructs)
      {
        *eventStructs = new PERIPHERAL_EVENT[events.size()];
        for (unsigned int i = 0; i < events.size(); i++)
        {
          (*eventStructs)[i].peripheral_index = events.at(i).m_index;
          (*eventStructs)[i].type             = events.at(i).m_type;
          (*eventStructs)[i].event_data       = NULL;

          switch (events.at(i).m_type)
          {
            case JOYSTICK_EVENT_TYPE_VIRTUAL_BUTTON:
              (*eventStructs)[i].event_data = new JOYSTICK_EVENT_VIRTUAL_BUTTON(events.at(i).EventAsType<JOYSTICK_EVENT_VIRTUAL_BUTTON>());
              break;
            case JOYSTICK_EVENT_TYPE_VIRTUAL_HAT:
              (*eventStructs)[i].event_data = new JOYSTICK_EVENT_VIRTUAL_HAT(events.at(i).EventAsType<JOYSTICK_EVENT_VIRTUAL_HAT>());
              break;
            case JOYSTICK_EVENT_TYPE_VIRTUAL_AXIS:
              (*eventStructs)[i].event_data = new JOYSTICK_EVENT_VIRTUAL_AXIS(events.at(i).EventAsType<JOYSTICK_EVENT_VIRTUAL_AXIS>());
              break;
            case JOYSTICK_EVENT_TYPE_BUTTON_DIGITAL:
              (*eventStructs)[i].event_data = new JOYSTICK_EVENT_BUTTON_DIGITAL(events.at(i).EventAsType<JOYSTICK_EVENT_BUTTON_DIGITAL>());
              break;
            case JOYSTICK_EVENT_TYPE_BUTTON_ANALOG:
              (*eventStructs)[i].event_data = new JOYSTICK_EVENT_BUTTON_ANALOG(events.at(i).EventAsType<JOYSTICK_EVENT_BUTTON_ANALOG>());
              break;
            case JOYSTICK_EVENT_TYPE_ANALOG_STICK:
              (*eventStructs)[i].event_data = new JOYSTICK_EVENT_ANALOG_STICK(events.at(i).EventAsType<JOYSTICK_EVENT_ANALOG_STICK>());
              break;
            case JOYSTICK_EVENT_TYPE_ACCELEROMETER:
              (*eventStructs)[i].event_data = new JOYSTICK_EVENT_ACCELEROMETER(events.at(i).EventAsType<JOYSTICK_EVENT_ACCELEROMETER>());
              break;
            case JOYSTICK_EVENT_TYPE_NONE:
            default:
              break;
          }
        }
      }
    }

    static void FreeStruct(PERIPHERAL_EVENT& event)
    {
      switch (event.type)
      {
        case JOYSTICK_EVENT_TYPE_VIRTUAL_BUTTON:
          delete static_cast<JOYSTICK_EVENT_VIRTUAL_BUTTON*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_VIRTUAL_HAT:
          delete static_cast<JOYSTICK_EVENT_VIRTUAL_HAT*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_VIRTUAL_AXIS:
          delete static_cast<JOYSTICK_EVENT_VIRTUAL_AXIS*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_BUTTON_DIGITAL:
          delete static_cast<JOYSTICK_EVENT_BUTTON_DIGITAL*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_BUTTON_ANALOG:
          delete static_cast<JOYSTICK_EVENT_BUTTON_ANALOG*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_ANALOG_STICK:
          delete static_cast<JOYSTICK_EVENT_ANALOG_STICK*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_ACCELEROMETER:
          delete static_cast<JOYSTICK_EVENT_ACCELEROMETER*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_NONE:
        default:
          break;
      }
      event.event_data = NULL;
    }

    static void FreeStructs(unsigned int eventCount, PERIPHERAL_EVENT* events)
    {
      for (unsigned int i = 0; i < eventCount; i++)
        FreeStruct(events[i]);
      delete[] events;
    }

  private:
    unsigned int        m_index;
    JOYSTICK_EVENT_TYPE m_type;
    void*               m_data;
  };
}
