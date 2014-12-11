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
  class AddonButton
  {
  public:
    AddonButton(void) : m_index(0), m_id(JOYSTICK_ID_BUTTON_UNKNOWN), m_state(JOYSTICK_BUTTON_STATE_UP) { }

    AddonButton(const JOYSTICK_BUTTON& button)
    : m_index(button.index),
      m_id(button.button_id),
      m_strLabel(button.label ? button.label : ""),
      m_state(button.state)
    {
    }

    unsigned int Index(void) const { return m_index; }
    void SetIndex(unsigned int index) { m_index = index; }

    JOYSTICK_ID_BUTTON ID(void) const { return m_id; }
    void SetID(JOYSTICK_ID_BUTTON buttonId) { m_id = buttonId; }

    const std::string& Label(void) const { return m_strLabel; }
    void SetLabel(const std::string& strLabel) { m_strLabel = strLabel; }

    JOYSTICK_BUTTON_STATE State(void) const { return m_state; }
    void SetState(JOYSTICK_BUTTON_STATE state) { m_state = state; }

    void ToButton(JOYSTICK_BUTTON& button)
    {
      button.index = m_index;
      button.button_id = m_id;
      button.label = new char[m_strLabel.size() + 1];
      std::strcpy(button.label, m_strLabel.c_str());
      button.state = m_state;
    }

    static void Free(JOYSTICK_BUTTON& button)
    {
      delete[] button.label;
    }

  private:
    unsigned int          m_index;
    JOYSTICK_ID_BUTTON    m_id;
    std::string           m_strLabel;
    JOYSTICK_BUTTON_STATE m_state;
  };

  class AddonHat
  {
  public:
    AddonHat(void) : m_index(0), m_state(JOYSTICK_HAT_STATE_UNPRESSED) { }

    AddonHat(const JOYSTICK_HAT& hat) : m_index(hat.index), m_state(hat.state) { }

    unsigned int Index(void) const { return m_index; }
    void SetIndex(unsigned int index) { m_index = index; }

    JOYSTICK_HAT_STATE State(void) const { return m_state; }
    void SetState(JOYSTICK_HAT_STATE state) { m_state = state; }

    void ToHat(JOYSTICK_HAT& hat)
    {
      hat.index = m_index;
      hat.state = m_state;
    }

    static void Free(JOYSTICK_HAT& hat)
    {
    }

  private:
    unsigned int       m_index;
    JOYSTICK_HAT_STATE m_state;
  };

  class AddonAxis
  {
  public:
    AddonAxis(void) : m_index(0), m_state(0.0f) { }

    AddonAxis(const JOYSTICK_AXIS& axis) : m_index(axis.index), m_state(axis.state) { }

    unsigned int Index(void) const { return m_index; }
    void SetIndex(unsigned int index) { m_index = index; }

    float State(void) const { return m_state; }
    void SetState(float state) { m_state = state; }

    void ToAxis(JOYSTICK_AXIS& axis)
    {
      axis.index = m_index;
      axis.state = m_state;
    }

    static void Free(JOYSTICK_AXIS& axis)
    {
    }

  private:
    unsigned int m_index;
    float        m_state;
  };

  class AddonTrigger
  {
  public:
    AddonTrigger(unsigned int axisIndex, JOYSTICK_AXIS_SIGN axisSign)
    : m_index(0),
      m_axisIndex(axisIndex),
      m_axisSign(axisSign),
      m_id(JOYSTICK_ID_TRIGGER_UNKNOWN),
      m_state(0.0f)
    {
    }

    AddonTrigger(const JOYSTICK_TRIGGER& trigger)
    : m_index(trigger.index),
      m_axisIndex(trigger.axis_index),
      m_axisSign(trigger.axis_sign),
      m_id(trigger.trigger_id),
      m_strLabel(trigger.label ? trigger.label : ""),
      m_state(trigger.state)
    {
    }

    unsigned int Index(void) const { return m_index; }
    void SetIndex(unsigned int index) { m_index = index; }

    unsigned int AxisIndex(void) const { return m_axisIndex; }

    JOYSTICK_AXIS_SIGN AxisSign(void) const { return m_axisSign; }

    JOYSTICK_ID_TRIGGER ID(void) const { return m_id; }
    void SetID(JOYSTICK_ID_TRIGGER triggerId) { m_id = triggerId; }

    const std::string& Label(void) const { return m_strLabel; }
    void SetLabel(const std::string& strLabel) { m_strLabel = strLabel; }

    float State(void) const { return m_state; }
    void SetState(float state) { m_state = state; }

    void ToTrigger(JOYSTICK_TRIGGER& trigger)
    {
      trigger.index = m_index;
      trigger.axis_index = m_axisIndex;
      trigger.axis_sign = m_axisSign;
      trigger.label = new char[m_strLabel.size() + 1];
      std::strcpy(trigger.label, m_strLabel.c_str());
      trigger.state = m_state;
    }

    static void Free(JOYSTICK_TRIGGER& trigger)
    {
      delete[] trigger.label;
    }

  private:
    unsigned int        m_index;
    unsigned int        m_axisIndex;
    JOYSTICK_AXIS_SIGN  m_axisSign;
    JOYSTICK_ID_TRIGGER m_id;
    std::string         m_strLabel;
    float               m_state;
  };

  class AddonAnalogStick
  {
  public:
    AddonAnalogStick(unsigned int horizAxisIndex, JOYSTICK_DIRECTION horizAxisPositiveDir,
                     unsigned int vertAxisIndex, JOYSTICK_DIRECTION vertAxisPositiveDir)
    : m_index(0),
      m_horizAxisIndex(horizAxisIndex),
      m_horizAxisPositiveDir(horizAxisPositiveDir),
      m_vertAxisIndex(vertAxisIndex),
      m_vertAxisPositiveDir(vertAxisPositiveDir),
      m_id(JOYSTICK_ID_ANALOG_STICK_UNKNOWN),
      m_horizState(0.0f),
      m_vertState(0.0f)
    {
    }

    AddonAnalogStick(const JOYSTICK_ANALOG_STICK& analogStick)
    : m_index(analogStick.index),
      m_horizAxisIndex(analogStick.horiz_axis_index),
      m_horizAxisPositiveDir(analogStick.horiz_axis_positive_dir),
      m_vertAxisIndex(analogStick.vert_axis_index),
      m_vertAxisPositiveDir(analogStick.vert_axis_positive_dir),
      m_id(analogStick.analog_stick_id),
      m_strLabel(analogStick.label ? analogStick.label : ""),
      m_horizState(analogStick.horiz_state),
      m_vertState(analogStick.vert_state)
    {
    }

    unsigned int Index(void) const { return m_index; }
    void SetIndex(unsigned int index) { m_index = index; }

    unsigned int HorizAxisIndex(void) const { return m_horizAxisIndex; }

    JOYSTICK_DIRECTION HorizAxisPositiveDir(void) const { return m_horizAxisPositiveDir; }

    unsigned int vertAxisIndex(void) const { return m_vertAxisIndex; }

    JOYSTICK_DIRECTION VertAxisPositiveDir(void) const { return m_vertAxisPositiveDir; }

    JOYSTICK_ID_ANALOG_STICK ID(void) const { return m_id; }
    void SetID(JOYSTICK_ID_ANALOG_STICK analogStickId) { m_id = analogStickId; }

    const std::string& Label(void) const { return m_strLabel; }
    void SetLabel(const std::string& strLabel) { m_strLabel = strLabel; }

    float HorizState(void) const { return m_horizState; }
    void SetHorizState(float horizState) { m_horizState = horizState; }

    float VertState(void) const { return m_vertState; }
    void SetVertState(float vertState) { m_vertState = vertState; }

    void ToAnalogStick(JOYSTICK_ANALOG_STICK& analogStick)
    {
      // TODO
    }

    static void Free(JOYSTICK_ANALOG_STICK& analogStick)
    {
      // TODO
    }

  private:
    unsigned int             m_index;
    unsigned int             m_horizAxisIndex;
    JOYSTICK_DIRECTION       m_horizAxisPositiveDir;
    unsigned int             m_vertAxisIndex;
    JOYSTICK_DIRECTION       m_vertAxisPositiveDir;
    JOYSTICK_ID_ANALOG_STICK m_id;
    std::string              m_strLabel;
    float                    m_horizState;
    float                    m_vertState;
  };

  class AddonJoystick
  {
  public:
    AddonJoystick(void) : m_index(0), m_requestedPlayer(0) { }

    AddonJoystick(const JOYSTICK& joystick)
    : m_index(joystick.index),
      m_requestedPlayer(joystick.requested_player),
      m_strName(joystick.name ? joystick.name : ""),
      m_strIconPath(joystick.icon_path ? joystick.icon_path : "")
    {
      m_buttons.reserve(joystick.button_count);
      for (unsigned int i = 0; i < joystick.button_count; i++)
        m_buttons.push_back(joystick.buttons[i]);

      m_hats.reserve(joystick.hat_count);
      for (unsigned int i = 0; i < joystick.hat_count; i++)
        m_hats.push_back(joystick.hats[i]);

      m_axes.reserve(joystick.axis_count);
      for (unsigned int i = 0; i < joystick.axis_count; i++)
        m_axes.push_back(joystick.axes[i]);

      m_triggers.reserve(joystick.trigger_count);
      for (unsigned int i = 0; i < joystick.trigger_count; i++)
        m_triggers.push_back(joystick.triggers[i]);

      m_analogSticks.reserve(joystick.analog_stick_count);
      for (unsigned int i = 0; i < joystick.analog_stick_count; i++)
        m_analogSticks.push_back(joystick.analog_sticks[i]);
    }

    unsigned int Index(void) const { return m_index; }
    void SetIndex(unsigned int index) { m_index = index; }

    unsigned int RequestedPlayer(void) const { return m_requestedPlayer; }
    void SetRequestedPlayer(unsigned int requestedPlayer) { m_requestedPlayer = requestedPlayer; }

    const std::string& Name(void) const { return m_strName; }
    void SetName(const std::string& strName) { m_strName = strName; }

    const std::string& IconPath(void) const { return m_strIconPath; }
    void SetIconPath(const std::string& strIconPath) { m_strIconPath = strIconPath; }

    const std::vector<AddonButton>& Buttons(void) const { return m_buttons; }
          std::vector<AddonButton>& Buttons(void)       { return m_buttons; }

    const std::vector<AddonHat>& Hats(void) const { return m_hats; }
          std::vector<AddonHat>& Hats(void)       { return m_hats; }

    const std::vector<AddonAxis>& Axes(void) const { return m_axes; }
          std::vector<AddonAxis>& Axes(void)       { return m_axes; }

    const std::vector<AddonTrigger>& Triggers(void) const { return m_triggers; }
          std::vector<AddonTrigger>& Triggers(void)       { return m_triggers; }

    const std::vector<AddonAnalogStick>& AnalogSticks(void) const { return m_analogSticks; }
          std::vector<AddonAnalogStick>& AnalogSticks(void)       { return m_analogSticks; }

    void ToJoystick(JOYSTICK& joystick)
    {
      joystick.index = m_index;
      joystick.requested_player = m_requestedPlayer;

      joystick.name = new char[m_strName.length() + 1];
      std::strcpy(joystick.name, m_strName.c_str());

      joystick.icon_path = new char[m_strIconPath.length() + 1];
      std::strcpy(joystick.icon_path, m_strIconPath.c_str());

      joystick.button_count = m_buttons.size();
      joystick.buttons = new JOYSTICK_BUTTON[m_buttons.size()];
      for (unsigned int i = 0; i < m_buttons.size(); i++)
        m_buttons[i].ToButton(joystick.buttons[i]);

      joystick.hat_count = m_hats.size();
      joystick.hats = new JOYSTICK_HAT[m_hats.size()];
      for (unsigned int i = 0; i < m_hats.size(); i++)
        m_hats[i].ToHat(joystick.hats[i]);

      joystick.axis_count = m_axes.size();
      joystick.axes = new JOYSTICK_AXIS[m_axes.size()];
      for (unsigned int i = 0; i < m_axes.size(); i++)
        m_axes[i].ToAxis(joystick.axes[i]);

      joystick.trigger_count = m_triggers.size();
      joystick.triggers = new JOYSTICK_TRIGGER[m_triggers.size()];
      for (unsigned int i = 0; i < m_triggers.size(); i++)
        m_triggers[i].ToTrigger(joystick.triggers[i]);

      joystick.analog_stick_count = m_analogSticks.size();
      joystick.analog_sticks = new JOYSTICK_ANALOG_STICK[m_analogSticks.size()];
      for (unsigned int i = 0; i < m_analogSticks.size(); i++)
        m_analogSticks[i].ToAnalogStick(joystick.analog_sticks[i]);
    }

    static void Free(JOYSTICK& joystick)
    {
      delete[] joystick.name;
      delete[] joystick.icon_path;

      for (unsigned int i = 0; i < joystick.button_count; i++)
        AddonButton::Free(joystick.buttons[i]);
      delete[] joystick.buttons;

      for (unsigned int i = 0; i < joystick.hat_count; i++)
        AddonHat::Free(joystick.hats[i]);
      delete[] joystick.hats;

      for (unsigned int i = 0; i < joystick.axis_count; i++)
        AddonAxis::Free(joystick.axes[i]);
      delete[] joystick.axes;

      for (unsigned int i = 0; i < joystick.trigger_count; i++)
        AddonTrigger::Free(joystick.triggers[i]);
      delete[] joystick.triggers;

      for (unsigned int i = 0; i < joystick.analog_stick_count; i++)
        AddonAnalogStick::Free(joystick.analog_sticks[i]);
      delete[] joystick.analog_sticks;
    }

  private:
    unsigned int                  m_index;
    unsigned int                  m_requestedPlayer;
    std::string                   m_strName;
    std::string                   m_strIconPath;
    std::vector<AddonButton>      m_buttons;
    std::vector<AddonHat>         m_hats;
    std::vector<AddonAxis>        m_axes;
    std::vector<AddonTrigger>     m_triggers;
    std::vector<AddonAnalogStick> m_analogSticks;
  };
}
