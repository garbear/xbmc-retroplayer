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

#include <cstring>
#include <string>
#include <vector>

namespace ADDON
{
  /*
  class JoystickButton
  {
  public:
    JOYSTICK_BUTTON_TYPE type;
    JOYSTICK_BUTTON_ID   id;
    std::string          strLabel;
  };

  class JoystickTrigger
  {
  public:
    JOYSTICK_TRIGGER_TYPE   type;
    JOYSTICK_TRIGGER_ID     id;
    std::string          strLabel;
  };

  class JoystickAnalogStick
  {
  public:
    JOYSTICK_ANALOG_STICK_ID id;
    std::string              strLabel;
  };

  class Joystick
  {
  public:
    class JoystickElement { };

    class Button : public JoystickElement
    {
    public:
      JOYSTICK_BUTTON_TYPE type;
      JOYSTICK_BUTTON_ID   id;
      std::string          strLabel;
    };

    class Trigger : public JoystickElement
    {
    public:
      JOYSTICK_TRIGGER_TYPE   type;
      JOYSTICK_TRIGGER_ID     id;
      std::string          strLabel;
    };

    class AnalogStick : public JoystickElement
    {
    public:
      JOYSTICK_ANALOG_STICK_ID id;
      std::string              strLabel;
    };

    unsigned int             index;
    unsigned int             requested_player;
    std::string              name;
    std::string              icon_path;
    unsigned int             virtual_button_count;
    unsigned int             virtual_hat_count;
    unsigned int             virtual_axis_count;
    std::vector<JoystickButton>      m_buttons;
    std::vector<JoystickTrigger>     m_triggers;
    std::vector<JoystickAnalogStick> m_analogSticks;
  };

  class ButtonMapValue { };

  class ButtonMapButton
  {

  };


  typedef std::map<JoystickElement*, ButtonMapValue> ButtonMap;

  typedef std::map<>

  class ButtonMap
  {
  public:
    ButtonMap(void) : m_index(0), m_id(JOYSTICK_ID_BUTTON_UNKNOWN) { }

    ButtonMap(const JOYSTICK_MAP_BUTTON& buttonMap)
    : m_index(buttonMap.index),
      m_id(buttonMap.id),
      m_strLabel(buttonMap.label ? buttonMap.label : "")
    {
    }

    unsigned int       Index(void) const { return m_index; }
    JOYSTICK_ID_BUTTON ID(void) const    { return m_id; }
    const std::string& Label(void) const { return m_strLabel; }

    void SetIndex(unsigned int index)          { m_index = index; }
    void SetID(JOYSTICK_ID_BUTTON buttonId)    { m_id = buttonId; }
    void SetLabel(const std::string& strLabel) { m_strLabel = strLabel; }

    void ToButtonMap(JOYSTICK_MAP_BUTTON& buttonMap) const
    {
      buttonMap.index = m_index;
      buttonMap.id    = m_id;
      buttonMap.label = new char[m_strLabel.size() + 1];
      std::strcpy(buttonMap.label, m_strLabel.c_str());
    }

    static void Free(JOYSTICK_MAP_BUTTON& buttonMap)
    {
      delete[] buttonMap.label;
    }

  private:
    unsigned int       m_index;
    JOYSTICK_ID_BUTTON m_id;
    std::string        m_strLabel;
  };

  class TriggerMap
  {
  public:
    TriggerMap(void) : m_axisIndex(0), m_axisSign(JOYSTICK_AXIS_POSITIVE), m_id(JOYSTICK_ID_TRIGGER_UNKNOWN) { }

    TriggerMap(const JOYSTICK_MAP_TRIGGER& triggerMap)
    : m_axisIndex(triggerMap.axis_index),
      m_axisSign(triggerMap.axis_sign),
      m_id(triggerMap.id),
      m_strLabel(triggerMap.label ? triggerMap.label : "")
    {
    }

    unsigned int        AxisIndex(void) const { return m_axisIndex; }
    JOYSTICK_AXIS_SIGN  AxisSign(void) const  { return m_axisSign; }
    JOYSTICK_ID_TRIGGER ID(void) const        { return m_id; }
    const std::string&  Label(void) const     { return m_strLabel; }

    void SetAxisIndex(unsigned int axisIndex)     { m_axisIndex = axisIndex; }
    void SetAxisSign(JOYSTICK_AXIS_SIGN axisSign) { m_axisSign = axisSign; }
    void SetID(JOYSTICK_ID_TRIGGER triggerId)     { m_id = triggerId; }
    void SetLabel(const std::string& strLabel)    { m_strLabel = strLabel; }

    void ToTriggerMap(JOYSTICK_MAP_TRIGGER& triggerMap) const
    {
      triggerMap.axis_index = m_axisIndex;
      triggerMap.axis_sign  = m_axisSign;
      triggerMap.id         = m_id;
      triggerMap.label      = new char[m_strLabel.size() + 1];
      std::strcpy(triggerMap.label, m_strLabel.c_str());
    }

    static void Free(JOYSTICK_MAP_TRIGGER& triggerMap)
    {
      delete[] triggerMap.label;
    }

  private:
    unsigned int        m_axisIndex;
    JOYSTICK_AXIS_SIGN  m_axisSign;
    JOYSTICK_ID_TRIGGER m_id;
    std::string         m_strLabel;
  };

  class AnalogStickMap
  {
  public:
    AnalogStickMap(void)
    : m_horizAxisIndex(0),
      m_horizPositiveDir(JOYSTICK_POSITIVE_DIR_UP_RIGHT),
      m_vertAxisIndex(0),
      m_vertPositiveDir(JOYSTICK_POSITIVE_DIR_UP_RIGHT),
      m_id(JOYSTICK_ID_ANALOG_STICK_UNKNOWN)
   {
   }

    AnalogStickMap(const JOYSTICK_MAP_ANALOG_STICK& analogStickMap)
    : m_horizAxisIndex(analogStickMap.horiz_axis_index),
      m_horizPositiveDir(analogStickMap.horiz_positive_dir),
      m_vertAxisIndex(analogStickMap.vert_axis_index),
      m_vertPositiveDir(analogStickMap.vert_positive_dir),
      m_id(analogStickMap.id),
      m_strLabel(analogStickMap.label ? analogStickMap.label : "")
    {
    }

    unsigned int             HorizAxisIndex(void) const { return m_horizAxisIndex; }
    JOYSTICK_POSITIVE_DIR    HorizAxisSign(void) const  { return m_horizPositiveDir; }
    unsigned int             VertAxisIndex(void) const  { return m_vertAxisIndex; }
    JOYSTICK_POSITIVE_DIR    VertAxisSign(void) const   { return m_vertPositiveDir; }
    JOYSTICK_ID_ANALOG_STICK ID(void) const             { return m_id; }
    const std::string&       Label(void) const          { return m_strLabel; }

    void SetHorizAxisIndex(unsigned int horizAxisIndex)           { m_horizAxisIndex = horizAxisIndex; }
    void SetHorizAxisSign(JOYSTICK_POSITIVE_DIR horizPositiveDir) { m_horizPositiveDir = horizPositiveDir; }
    void SetVertAxisIndex(unsigned int vertAxisIndex)             { m_vertAxisIndex = vertAxisIndex; }
    void SetVertAxisSign(JOYSTICK_POSITIVE_DIR vertPositiveDir)   { m_vertPositiveDir = vertPositiveDir; }
    void SetID(JOYSTICK_ID_ANALOG_STICK analogStickId)            { m_id = analogStickId; }
    void SetLabel(const std::string& strLabel)                    { m_strLabel = strLabel; }

    void ToAnalogStickMap(JOYSTICK_MAP_ANALOG_STICK& analogStickMap) const
    {
      analogStickMap.horiz_axis_index   = m_horizAxisIndex;
      analogStickMap.horiz_positive_dir = m_horizPositiveDir;
      analogStickMap.vert_axis_index    = m_vertAxisIndex;
      analogStickMap.vert_positive_dir  = m_vertPositiveDir;
      analogStickMap.id                 = m_id;
      analogStickMap.label              = new char[m_strLabel.size() + 1];
      std::strcpy(analogStickMap.label, m_strLabel.c_str());
    }

    static void Free(JOYSTICK_MAP_ANALOG_STICK& analogStickMap)
    {
      delete[] analogStickMap.label;
    }

  private:
    unsigned int             m_horizAxisIndex;
    JOYSTICK_POSITIVE_DIR    m_horizPositiveDir;
    unsigned int             m_vertAxisIndex;
    JOYSTICK_POSITIVE_DIR    m_vertPositiveDir;
    JOYSTICK_ID_ANALOG_STICK m_id;
    std::string              m_strLabel;
  };

  class JoystickLayout
  {
  public:
    JoystickLayout(void) { }

    JoystickLayout(const JOYSTICK_LAYOUT& layout)
    {
      for (unsigned int i = 0; i < layout.button_count; i++)
        m_buttonMap.push_back(layout.buttons[i]);

      for (unsigned int i = 0; i < layout.trigger_count; i++)
        m_triggerMap.push_back(layout.triggers[i]);

      for (unsigned int i = 0; i < layout.analog_stick_count; i++)
        m_analogStickMap.push_back(layout.analog_sticks[i]);
    }

    const std::vector<ButtonMap>&      ButtonMaps(void) const      { return m_buttonMap; }
    const std::vector<TriggerMap>&     TriggerMaps(void) const     { return m_triggerMap; }
    const std::vector<AnalogStickMap>& AnalogStickMaps(void) const { return m_analogStickMap; }

    std::vector<ButtonMap>&      ButtonMaps(void)      { return m_buttonMap; }
    std::vector<TriggerMap>&     TriggerMaps(void)     { return m_triggerMap; }
    std::vector<AnalogStickMap>& AnalogStickMaps(void) { return m_analogStickMap; }

    void ToJoystickLayout(JOYSTICK_LAYOUT& layout) const
    {
      layout.button_count = m_buttonMap.size();
      layout.buttons = new JOYSTICK_MAP_BUTTON[m_buttonMap.size()];
      for (unsigned int i = 0; i < m_buttonMap.size(); i++)
        m_buttonMap[i].ToButtonMap(layout.buttons[i]);

      layout.trigger_count = m_triggerMap.size();
      layout.triggers = new JOYSTICK_MAP_TRIGGER[m_triggerMap.size()];
      for (unsigned int i = 0; i < m_triggerMap.size(); i++)
        m_triggerMap[i].ToTriggerMap(layout.triggers[i]);

      layout.analog_stick_count = m_analogStickMap.size();
      layout.analog_sticks = new JOYSTICK_MAP_ANALOG_STICK[m_analogStickMap.size()];
      for (unsigned int i = 0; i < m_analogStickMap.size(); i++)
        m_analogStickMap[i].ToAnalogStickMap(layout.analog_sticks[i]);
    }

    static void Free(JOYSTICK_LAYOUT& layout)
    {
      for (unsigned int i = 0; i < layout.button_count; i++)
        ButtonMap::Free(layout.buttons[i]);
      delete[] layout.buttons;

      for (unsigned int i = 0; i < layout.trigger_count; i++)
        TriggerMap::Free(layout.triggers[i]);
      delete[] layout.triggers;

      for (unsigned int i = 0; i < layout.analog_stick_count; i++)
        AnalogStickMap::Free(layout.analog_sticks[i]);
      delete[] layout.analog_sticks;
    }

  private:
    std::vector<ButtonMap>      m_buttonMap;
    std::vector<TriggerMap>     m_triggerMap;
    std::vector<AnalogStickMap> m_analogStickMap;
  };

  class JoystickConfiguration
  {
  public:
    JoystickConfiguration(void) : m_index(0), m_requestedPlayer(0) { }

    JoystickConfiguration(const JOYSTICK_CONFIGURATION& joystick)
    : m_index(joystick.index),
      m_requestedPlayer(joystick.requested_player),
      m_strName(joystick.name ? joystick.name : ""),
      m_strIconPath(joystick.icon_path ? joystick.icon_path : ""),
      m_layout(joystick.layout)
    {
      m_buttonIndexes.reserve(joystick.button_count);
      for (unsigned int i = 0; i < joystick.button_count; i++)
        m_buttonIndexes.push_back(joystick.buttons[i]);

      m_hatIndexes.reserve(joystick.hat_count);
      for (unsigned int i = 0; i < joystick.hat_count; i++)
        m_hatIndexes.push_back(joystick.hats[i]);

      m_axisIndexes.reserve(joystick.axis_count);
      for (unsigned int i = 0; i < joystick.axis_count; i++)
        m_axisIndexes.push_back(joystick.axes[i]);
    }

    unsigned int       Index(void) const { return m_index; }
    unsigned int       RequestedPlayer(void) const { return m_requestedPlayer; }
    const std::string& Name(void) const { return m_strName; }
    const std::string& IconPath(void) const { return m_strIconPath; }

    const std::vector<unsigned int>& ButtonIndexes(void) const { return m_buttonIndexes; }
    const std::vector<unsigned int>& HatIndexes(void) const    { return m_hatIndexes; }
    const std::vector<unsigned int>& AxisIndexes(void) const   { return m_axisIndexes; }

    void SetIndex(unsigned int index)                     { m_index = index; }
    void SetRequestedPlayer(unsigned int requestedPlayer) { m_requestedPlayer = requestedPlayer; }
    void SetName(const std::string& strName)              { m_strName = strName; }
    void SetIconPath(const std::string& strIconPath)      { m_strIconPath = strIconPath; }

    std::vector<unsigned int>& ButtonIndexes(void) { return m_buttonIndexes; }
    std::vector<unsigned int>& HatIndexes(void)    { return m_hatIndexes; }
    std::vector<unsigned int>& AxisIndexes(void)   { return m_axisIndexes; }

    void ToJoystickConfiguration(JOYSTICK_CONFIGURATION& joystick) const
    {
      joystick.index            = m_index;
      joystick.requested_player = m_requestedPlayer;
      joystick.name             = new char[m_strName.length() + 1];
      std::strcpy(joystick.name, m_strName.c_str());
      joystick.icon_path        = new char[m_strIconPath.length() + 1];
      std::strcpy(joystick.icon_path, m_strIconPath.c_str());

      joystick.button_count = m_buttonIndexes.size();
      joystick.buttons = new unsigned int[m_buttonIndexes.size()];
      for (unsigned int i = 0; i < m_buttonIndexes.size(); i++)
        joystick.buttons[i] = m_buttonIndexes[i];

      joystick.hat_count = m_hatIndexes.size();
      joystick.hats = new unsigned int[m_hatIndexes.size()];
      for (unsigned int i = 0; i < m_hatIndexes.size(); i++)
        joystick.hats[i] = m_hatIndexes[i];

      joystick.axis_count = m_axisIndexes.size();
      joystick.axes = new unsigned int[m_axisIndexes.size()];
      for (unsigned int i = 0; i < m_axisIndexes.size(); i++)
        joystick.axes[i] = m_axisIndexes[i];

      m_layout.ToJoystickLayout(joystick.layout);
    }

    static void Free(JOYSTICK_CONFIGURATION& joystick)
    {
      delete[] joystick.name;
      delete[] joystick.icon_path;
      delete[] joystick.buttons;
      delete[] joystick.hats;
      delete[] joystick.axes;

      JoystickLayout::Free(joystick.layout);
    }

  private:
    unsigned int              m_index;
    unsigned int              m_requestedPlayer;
    std::string               m_strName;
    std::string               m_strIconPath;
    std::vector<unsigned int> m_buttonIndexes;
    std::vector<unsigned int> m_hatIndexes;
    std::vector<unsigned int> m_axisIndexes;
    JoystickLayout            m_layout;
  };

  class JoystickEvent
  {
  public:
    virtual ~JoystickEvent(void) { }
    virtual JOYSTICK_EVENT_TYPE Type(void) const = 0;
  };

  class RawEvent : public JoystickEvent
  {
  public:
    RawEvent(void) : m_index(0) { }
    unsigned int Index(void) const { return m_index; }
    void SetIndex(unsigned int index) { m_index = index; }

  private:
    unsigned int m_index;
  };

  class ButtonEvent : public RawEvent
  {
  public:
    ButtonEvent(void) : RawEvent(), m_state() { }
    virtual JOYSTICK_EVENT_TYPE Type(void) const { return JOYSTICK_EVENT_TYPE_RAW_BUTTON; }
    JOYSTICK_STATE_BUTTON State(void) const { return m_state; }
    void SetState(JOYSTICK_STATE_BUTTON state) { m_state = state; }

  private:
    JOYSTICK_STATE_BUTTON m_state;
  };

  class HatEvent : public RawEvent
  {
  public:
    HatEvent(void) : RawEvent(), m_state() { }
    virtual JOYSTICK_EVENT_TYPE Type(void) const { return JOYSTICK_EVENT_TYPE_RAW_HAT; }
    JOYSTICK_STATE_HAT State(void) const { return m_state; }
    void SetState(JOYSTICK_STATE_HAT state) { m_state = state; }

  private:
    JOYSTICK_STATE_HAT m_state;
  };

  class AxisEvent : public RawEvent
  {
  public:
    AxisEvent(void) : RawEvent(), m_state() { }
    virtual JOYSTICK_EVENT_TYPE Type(void) const { return JOYSTICK_EVENT_TYPE_RAW_AXIS; }
    JOYSTICK_STATE_AXIS State(void) const { return m_state; }
    void SetState(JOYSTICK_STATE_AXIS state) { m_state = state; }

  private:
    JOYSTICK_STATE_AXIS m_state;
  };

  template <typename ID_TYPE>
  class MappedEvent : public JoystickEvent
  {
  public:
    MappedEvent(void) : m_id() { }
    ID_TYPE ID(void) const { return m_id; }
    void SetIndex(ID_TYPE id) { m_id = id; }

  private:
    ID_TYPE m_id;
  };

  class MappedButtonEvent : public MappedEvent<JOYSTICK_ID_BUTTON>
  {
  public:
    MappedButtonEvent(void) : MappedEvent<JOYSTICK_ID_BUTTON>(), m_state() { }
    JOYSTICK_STATE_BUTTON State(void) const { return m_state; }
    void SetState(JOYSTICK_STATE_BUTTON state) { m_state = state; }

  private:
    JOYSTICK_STATE_BUTTON m_state;
  };

  class MappedTriggerEvent : public MappedEvent<JOYSTICK_ID_TRIGGER>
  {
  public:
    MappedTriggerEvent(void) : MappedEvent<JOYSTICK_ID_TRIGGER>(), m_state() { }
    JOYSTICK_STATE_TRIGGER State(void) const { return m_state; }
    void SetState(JOYSTICK_STATE_TRIGGER state) { m_state = state; }

  private:
    JOYSTICK_STATE_TRIGGER m_state;
  };

  class MappedAnalogStickEvent : public MappedEvent<JOYSTICK_ID_ANALOG_STICK>
  {
  public:
    MappedAnalogStickEvent(void) : MappedEvent<JOYSTICK_ID_ANALOG_STICK>(), m_state() { }
    JOYSTICK_STATE_ANALOG_STICK State(void) const { return m_state; }
    void SetState(JOYSTICK_STATE_ANALOG_STICK state) { m_state = state; }

  private:
    JOYSTICK_STATE_ANALOG_STICK m_state;
  };
  */
}
