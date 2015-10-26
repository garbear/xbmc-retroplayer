/*
 *      Copyright (C) 2014-2015 Team XBMC
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

#include "kodi_peripheral_types.h"

#include <cstring>
#include <map>
#include <string>
#include <vector>

#define PERIPHERAL_SAFE_DELETE(x)        do { delete   (x); (x) = NULL; } while (0)
#define PERIPHERAL_SAFE_DELETE_ARRAY(x)  do { delete[] (x); (x) = NULL; } while (0)

namespace ADDON
{
  /*!
   * Utility class to manipulate arrays of peripheral types.
   */
  template <class THE_CLASS, typename THE_STRUCT>
  class PeripheralVector
  {
  public:
    static void ToStructs(const std::vector<THE_CLASS>& vecObjects, THE_STRUCT** pStructs)
    {
      if (!pStructs)
        return;

      if (vecObjects.empty())
      {
        *pStructs = NULL;
      }
      else
      {
        (*pStructs) = new THE_STRUCT[vecObjects.size()];
        for (unsigned int i = 0; i < vecObjects.size(); i++)
          vecObjects.at(i).ToStruct((*pStructs)[i]);
      }
    }

    static void ToStructs(const std::vector<THE_CLASS*>& vecObjects, THE_STRUCT** pStructs)
    {
      if (!pStructs)
        return;

      if (vecObjects.empty())
      {
        *pStructs = NULL;
      }
      else
      {
        *pStructs = new THE_STRUCT[vecObjects.size()];
        for (unsigned int i = 0; i < vecObjects.size(); i++)
          vecObjects.at(i)->ToStruct((*pStructs)[i]);
      }
    }

    static void FreeStructs(unsigned int structCount, THE_STRUCT* structs)
    {
      if (structs)
      {
        for (unsigned int i = 0; i < structCount; i++)
          THE_CLASS::FreeStruct(structs[i]);
      }
      PERIPHERAL_SAFE_DELETE_ARRAY(structs);
    }
  };

  /*!
   * ADDON::Peripheral
   *
   * Wrapper class providing peripheral information. Classes can extend
   * Peripheral to inherit peripheral properties.
   */
  class Peripheral
  {
  public:
    Peripheral(PERIPHERAL_TYPE type = PERIPHERAL_TYPE_UNKNOWN, const std::string& strName = "") :
      m_type(type),
      m_strName(strName),
      m_vendorId(0),
      m_productId(0),
      m_index(0)
    {
    }

    Peripheral(const PERIPHERAL_INFO& info) :
      m_type(info.type),
      m_strName(info.name ? info.name : ""),
      m_vendorId(info.vendor_id),
      m_productId(info.product_id),
      m_index(info.index)
    {
    }

    virtual ~Peripheral(void) { }

    PERIPHERAL_TYPE    Type(void) const      { return m_type; }
    const std::string& Name(void) const      { return m_strName; }
    uint16_t           VendorID(void) const  { return m_vendorId; }
    uint16_t           ProductID(void) const { return m_productId; }
    unsigned int       Index(void) const     { return m_index; }

    void SetType(PERIPHERAL_TYPE type)       { m_type      = type; }
    void SetName(const std::string& strName) { m_strName   = strName; }
    void SetVendorID(uint16_t vendorId)      { m_vendorId  = vendorId; }
    void SetProductID(uint16_t productId)    { m_productId = productId; }
    void SetIndex(unsigned int index)        { m_index     = index; }

    void ToStruct(PERIPHERAL_INFO& info) const
    {
      info.type       = m_type;
      info.name       = new char[m_strName.size() + 1];
      info.vendor_id  = m_vendorId;
      info.product_id = m_productId;
      info.index      = m_index;

      std::strcpy(info.name, m_strName.c_str());
    }

    static void FreeStruct(PERIPHERAL_INFO& info)
    {
      PERIPHERAL_SAFE_DELETE_ARRAY(info.name);
    }

  private:
    PERIPHERAL_TYPE  m_type;
    std::string      m_strName;
    uint16_t         m_vendorId;
    uint16_t         m_productId;
    unsigned int     m_index;
  };

  typedef PeripheralVector<Peripheral, PERIPHERAL_INFO> Peripherals;

  /*!
   * ADDON::PeripheralEvent
   *
   * Wrapper class for peripheral events.
   */
  class PeripheralEvent
  {
  public:
    PeripheralEvent(void) :
      m_event()
    {
    }

    PeripheralEvent(unsigned int peripheralIndex, unsigned int buttonIndex, JOYSTICK_STATE_BUTTON state) :
      m_event()
    {
      SetType(PERIPHERAL_EVENT_TYPE_DRIVER_BUTTON);
      SetPeripheralIndex(peripheralIndex);
      SetDriverIndex(buttonIndex);
      SetButtonState(state);
    }

    PeripheralEvent(unsigned int peripheralIndex, unsigned int hatIndex, JOYSTICK_STATE_HAT state) :
      m_event()
    {
      SetType(PERIPHERAL_EVENT_TYPE_DRIVER_HAT);
      SetPeripheralIndex(peripheralIndex);
      SetDriverIndex(hatIndex);
      SetHatState(state);
    }

    PeripheralEvent(unsigned int peripheralIndex, unsigned int axisIndex, JOYSTICK_STATE_AXIS state) :
      m_event()
    {
      SetType(PERIPHERAL_EVENT_TYPE_DRIVER_AXIS);
      SetPeripheralIndex(peripheralIndex);
      SetDriverIndex(axisIndex);
      SetAxisState(state);
    }

    PeripheralEvent(const PERIPHERAL_EVENT& event) :
      m_event(event)
    {
    }

    PERIPHERAL_EVENT_TYPE Type(void) const            { return m_event.type; }
    unsigned int          PeripheralIndex(void) const { return m_event.peripheral_index; }
    unsigned int          DriverIndex(void) const     { return m_event.driver_index; }
    JOYSTICK_STATE_BUTTON ButtonState(void) const     { return m_event.driver_button_state; }
    JOYSTICK_STATE_HAT    HatState(void) const        { return m_event.driver_hat_state; }
    JOYSTICK_STATE_AXIS   AxisState(void) const       { return m_event.driver_axis_state; }

    void SetType(PERIPHERAL_EVENT_TYPE type)         { m_event.type                = type; }
    void SetPeripheralIndex(unsigned int index)      { m_event.peripheral_index    = index; }
    void SetDriverIndex(unsigned int index)          { m_event.driver_index        = index; }
    void SetButtonState(JOYSTICK_STATE_BUTTON state) { m_event.driver_button_state = state; }
    void SetHatState(JOYSTICK_STATE_HAT state)       { m_event.driver_hat_state    = state; }
    void SetAxisState(JOYSTICK_STATE_AXIS state)     { m_event.driver_axis_state   = state; }

    void ToStruct(PERIPHERAL_EVENT& event) const
    {
      event = m_event;
    }

    static void FreeStruct(PERIPHERAL_EVENT& event)
    {
      (void)event;
    }

  private:
    PERIPHERAL_EVENT m_event;
  };

  typedef PeripheralVector<PeripheralEvent, PERIPHERAL_EVENT> PeripheralEvents;

  /*!
   * ADDON::Joystick
   *
   * Wrapper class providing additional joystick information not provided by
   * ADDON::Peripheral.
   */
  class Joystick : public Peripheral
  {
  public:
    Joystick(const std::string& provider = "", const std::string& strName = "") :
      Peripheral(PERIPHERAL_TYPE_JOYSTICK, strName),
      m_provider(provider),
      m_requestedPort(NO_PORT_REQUESTED),
      m_buttonCount(0),
      m_hatCount(0),
      m_axisCount(0)
    {
    }

    Joystick(const Joystick& other)
    {
      *this = other;
    }

    Joystick(const JOYSTICK_INFO& info) :
      Peripheral(info.peripheral),
      m_provider(info.provider ? info.provider : ""),
      m_requestedPort(info.requested_port),
      m_buttonCount(info.button_count),
      m_hatCount(info.hat_count),
      m_axisCount(info.axis_count)
    {
    }

    virtual ~Joystick(void) { }

    Joystick& operator=(const Joystick& rhs)
    {
      if (this != &rhs)
      {
        Peripheral::operator=(rhs);

        m_provider      = rhs.m_provider;
        m_requestedPort = rhs.m_requestedPort;
        m_buttonCount   = rhs.m_buttonCount;
        m_hatCount      = rhs.m_hatCount;
        m_axisCount     = rhs.m_axisCount;
      }
      return *this;
    }

    const std::string& Provider(void) const      { return m_provider; }
    int                RequestedPort(void) const { return m_requestedPort; }
    unsigned int       ButtonCount(void) const   { return m_buttonCount; }
    unsigned int       HatCount(void) const      { return m_hatCount; }
    unsigned int       AxisCount(void) const     { return m_axisCount; }

    void SetProvider(const std::string& provider)     { m_provider      = provider; }
    void SetRequestedPort(int requestedPort)          { m_requestedPort = requestedPort; }
    void SetButtonCount(unsigned int buttonCount)     { m_buttonCount   = buttonCount; }
    void SetHatCount(unsigned int hatCount)           { m_hatCount      = hatCount; }
    void SetAxisCount(unsigned int axisCount)         { m_axisCount     = axisCount; }

    void ToStruct(JOYSTICK_INFO& info) const
    {
      Peripheral::ToStruct(info.peripheral);

      info.provider       = new char[m_provider.size() + 1];
      info.requested_port = m_requestedPort;
      info.button_count   = m_buttonCount;
      info.hat_count      = m_hatCount;
      info.axis_count     = m_axisCount;

      std::strcpy(info.provider, m_provider.c_str());
    }

    static void FreeStruct(JOYSTICK_INFO& info)
    {
      Peripheral::FreeStruct(info.peripheral);

      PERIPHERAL_SAFE_DELETE_ARRAY(info.provider);
    }

  private:
    std::string                   m_provider;
    int                           m_requestedPort;
    unsigned int                  m_buttonCount;
    unsigned int                  m_hatCount;
    unsigned int                  m_axisCount;
  };

  typedef PeripheralVector<Joystick, JOYSTICK_INFO> Joysticks;

  /*!
   * ADDON::DriverPrimitive
   *
   * Base class for joystick driver primitives. A driver primitive can be:
   *
   *   1) a button
   *   2) a hat direction
   *   3) a semiaxis (either the positive or negative half of an axis)
   *
   * The type determines the fields in use:
   *
   *    Button:
   *       - driver index
   *
   *    Hat direction:
   *       - driver index
   *       - hat direction
   *
   *    Semiaxis:
   *       - driver index
   *       - semiaxis direction
   */
  class DriverPrimitive
  {
  public:
    /*!
     * \brief Construct an invalid driver primitive
     */
    DriverPrimitive(void) :
      m_type(JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN),
      m_driverIndex(0),
      m_hatDirection(JOYSTICK_DRIVER_HAT_UNKNOWN),
      m_semiAxisDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_UNKNOWN)
    {
    }

    /*!
     * \brief Construct a driver primitive representing a button
     */
    DriverPrimitive(unsigned int buttonIndex) :
      m_type(JOYSTICK_DRIVER_PRIMITIVE_TYPE_BUTTON),
      m_driverIndex(buttonIndex),
      m_hatDirection(JOYSTICK_DRIVER_HAT_UNKNOWN),
      m_semiAxisDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_UNKNOWN)
    {
    }

    /*!
     * \brief Construct a driver primitive representing one of the four direction
     *        arrows on a dpad
     */
    DriverPrimitive(unsigned int hatIndex, JOYSTICK_DRIVER_HAT_DIRECTION direction) :
      m_type(JOYSTICK_DRIVER_PRIMITIVE_TYPE_HAT_DIRECTION),
      m_driverIndex(hatIndex),
      m_hatDirection(direction),
      m_semiAxisDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_UNKNOWN)
    {
    }

    /*!
     * \brief Construct a driver primitive representing the positive or negative
     *        half of an axis
     */
    DriverPrimitive(unsigned int axisIndex, JOYSTICK_DRIVER_SEMIAXIS_DIRECTION direction) :
      m_type(JOYSTICK_DRIVER_PRIMITIVE_TYPE_SEMIAXIS),
      m_driverIndex(axisIndex),
      m_hatDirection(JOYSTICK_DRIVER_HAT_UNKNOWN),
      m_semiAxisDirection(direction)
    {
    }

    DriverPrimitive(const JOYSTICK_DRIVER_PRIMITIVE& primitive) :
      m_type(primitive.type),
      m_driverIndex(0),
      m_hatDirection(JOYSTICK_DRIVER_HAT_UNKNOWN),
      m_semiAxisDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_UNKNOWN)
    {
      switch (m_type)
      {
        case JOYSTICK_DRIVER_PRIMITIVE_TYPE_BUTTON:
        {
          m_driverIndex = primitive.button.index;
          break;
        }
        case JOYSTICK_DRIVER_PRIMITIVE_TYPE_HAT_DIRECTION:
        {
          m_driverIndex  = primitive.hat.index;
          m_hatDirection = primitive.hat.direction;
          break;
        }
        case JOYSTICK_DRIVER_PRIMITIVE_TYPE_SEMIAXIS:
        {
          m_driverIndex       = primitive.semiaxis.index;
          m_semiAxisDirection = primitive.semiaxis.direction;
          break;
        }
        default:
          break;
      }
    }

    JOYSTICK_DRIVER_PRIMITIVE_TYPE     Type(void) const { return m_type; }
    unsigned int                       DriverIndex(void) const { return m_driverIndex; }
    JOYSTICK_DRIVER_HAT_DIRECTION      HatDirection(void) const { return m_hatDirection; }
    JOYSTICK_DRIVER_SEMIAXIS_DIRECTION SemiAxisDirection(void) const { return m_semiAxisDirection; }

    bool Equals(const DriverPrimitive& other) const
    {
      if (m_type == other.m_type)
      {
        switch (m_type)
        {
          case JOYSTICK_DRIVER_PRIMITIVE_TYPE_BUTTON:
          {
            return m_driverIndex == other.m_driverIndex;
          }
          case JOYSTICK_DRIVER_PRIMITIVE_TYPE_HAT_DIRECTION:
          {
            return m_driverIndex  == other.m_driverIndex &&
                   m_hatDirection == other.m_hatDirection;
          }
          case JOYSTICK_DRIVER_PRIMITIVE_TYPE_SEMIAXIS:
          {
            return m_driverIndex       == other.m_driverIndex &&
                   m_semiAxisDirection == other.m_semiAxisDirection;
          }
          default:
            break;
        }
      }
      return false;
    }

    void ToStruct(JOYSTICK_DRIVER_PRIMITIVE& driver_primitive) const
    {
      driver_primitive.type = m_type;
      switch (m_type)
      {
        case JOYSTICK_DRIVER_PRIMITIVE_TYPE_BUTTON:
        {
          driver_primitive.button.index = m_driverIndex;
          break;
        }
        case JOYSTICK_DRIVER_PRIMITIVE_TYPE_HAT_DIRECTION:
        {
          driver_primitive.hat.index     = m_driverIndex;
          driver_primitive.hat.direction = m_hatDirection;
          break;
        }
        case JOYSTICK_DRIVER_PRIMITIVE_TYPE_SEMIAXIS:
        {
          driver_primitive.semiaxis.index     = m_driverIndex;
          driver_primitive.semiaxis.direction = m_semiAxisDirection;
          break;
        }
        default:
          break;
      }
    }

  private:
    JOYSTICK_DRIVER_PRIMITIVE_TYPE     m_type;
    unsigned int                       m_driverIndex;
    JOYSTICK_DRIVER_HAT_DIRECTION      m_hatDirection;
    JOYSTICK_DRIVER_SEMIAXIS_DIRECTION m_semiAxisDirection;
  };

  /*!
   * ADDON::JoystickFeature
   *
   * Base class for joystick features. In kodi_peripheral_types.h, the various
   * driver types are stored as an union. Here, we use polymorphism to allow for
   * extra driver information.
   */
  class JoystickFeature
  {
  public:
    JoystickFeature(void)
    {
    }

    JoystickFeature(const std::string& name) :
      m_name(name)
    {
    }

    JoystickFeature(const JOYSTICK_FEATURE& feature) :
      m_name(feature.name ? feature.name : "")
    {
    }

    virtual ~JoystickFeature(void) { }

    virtual JoystickFeature* Clone(void) const { return new JoystickFeature(*this); }

    virtual bool Equals(const JoystickFeature* other) const
    {
      return other &&
             m_name == other->m_name &&
             Type() == other->Type();
    }

    virtual JOYSTICK_FEATURE_TYPE Type(void) const { return JOYSTICK_FEATURE_TYPE_UNKNOWN; }

    const std::string& Name(void) const { return m_name; }

    void SetName(const std::string& name) { m_name = name; }

    virtual void ToStruct(JOYSTICK_FEATURE& feature) const
    {
      feature.name = new char[m_name.length() + 1];
      feature.type = Type();

      std::strcpy(feature.name, m_name.c_str());
    }

    static void FreeStruct(JOYSTICK_FEATURE& feature)
    {
      PERIPHERAL_SAFE_DELETE_ARRAY(feature.name);
    }

  private:
    std::string  m_name;
  };

  typedef PeripheralVector<JoystickFeature, JOYSTICK_FEATURE> JoystickFeatures;

  /*!
   * ADDON::PrimitiveFeature
   *
   * When a feature can be represented by a single driver primitive, it is
   * called a primitive feature.
   *
   *   - This includes buttons and triggers, because they can be mapped to a
   *     single button/hat/semiaxis
   *
   *   - This does not include analog sticks, because they require two axes
   *     and four driver primitives (one for each semiaxis)
   *
   */
  class PrimitiveFeature : public JoystickFeature
  {
  public:
    PrimitiveFeature(void) { }

    PrimitiveFeature(const std::string& name, const DriverPrimitive& primitive) :
      JoystickFeature(name),
      m_primitive(primitive)
    {
    }

    PrimitiveFeature(const JOYSTICK_FEATURE& feature) :
      JoystickFeature(feature),
      m_primitive(feature.primitive.primitive)
    {
    }

    virtual ~PrimitiveFeature(void) { }

    virtual JoystickFeature* Clone(void) const { return new PrimitiveFeature(*this); }

    virtual bool Equals(const JoystickFeature* other) const
    {
      return JoystickFeature::Equals(other) &&
             m_primitive.Equals(static_cast<const PrimitiveFeature*>(other)->m_primitive);
    }

    virtual JOYSTICK_FEATURE_TYPE Type(void) const      { return JOYSTICK_FEATURE_TYPE_PRIMITIVE; }
    const DriverPrimitive&        Primitive(void) const { return m_primitive; }

    void SetDriver(const DriverPrimitive& primitive) { m_primitive = primitive; }

    virtual void ToStruct(JOYSTICK_FEATURE& feature) const
    {
      JoystickFeature::ToStruct(feature);
      m_primitive.ToStruct(feature.primitive.primitive);
    }

  private:
    DriverPrimitive m_primitive;
  };

  /*!
   * ADDON::AnalogStick
   *
   * Analog sticks are defined by two axes. Each semiaxis is stored as two
   * driver primitives.
   *
   * These primitives don't have to be semiaxes. If four buttons or hat
   * directions are used, the frontend will calculate values for both axes
   * based on the state of the buttons/hats.
   */
  class AnalogStick : public JoystickFeature
  {
  public:
    AnalogStick(void) { }

    AnalogStick(const std::string& name,
                const DriverPrimitive& up,
                const DriverPrimitive& down,
                const DriverPrimitive& right,
                const DriverPrimitive& left) :
      JoystickFeature(name),
      m_up(up),
      m_down(down),
      m_right(right),
      m_left(left)
    {
    }

    AnalogStick(const JOYSTICK_FEATURE& feature) :
      JoystickFeature(feature),
      m_up(feature.analog_stick.up),
      m_down(feature.analog_stick.down),
      m_right(feature.analog_stick.right),
      m_left(feature.analog_stick.left)
    {
    }

    virtual ~AnalogStick(void) { }

    virtual JoystickFeature* Clone(void) const { return new AnalogStick(*this); }

    virtual bool Equals(const JoystickFeature* other) const
    {
      return JoystickFeature::Equals(other) &&
             m_up.Equals(static_cast<const AnalogStick*>(other)->m_up) &&
             m_down.Equals(static_cast<const AnalogStick*>(other)->m_down) &&
             m_right.Equals(static_cast<const AnalogStick*>(other)->m_right) &&
             m_left.Equals(static_cast<const AnalogStick*>(other)->m_left);
    }

    virtual JOYSTICK_FEATURE_TYPE Type(void) const  { return JOYSTICK_FEATURE_TYPE_ANALOG_STICK; }
    const DriverPrimitive&        Up(void) const    { return m_up; }
    const DriverPrimitive&        Down(void) const  { return m_down; }
    const DriverPrimitive&        Right(void) const { return m_right; }
    const DriverPrimitive&        Left(void) const  { return m_left; }

    void SetUp(const DriverPrimitive& driverPrimitive)    { m_up    = driverPrimitive; }
    void SetDown(const DriverPrimitive& driverPrimitive)  { m_down  = driverPrimitive; }
    void SetRight(const DriverPrimitive& driverPrimitive) { m_right = driverPrimitive; }
    void SetLeft(const DriverPrimitive& driverPrimitive)  { m_left  = driverPrimitive; }

    virtual void ToStruct(JOYSTICK_FEATURE& feature) const
    {
      JoystickFeature::ToStruct(feature);
      m_up.ToStruct(feature.analog_stick.up);
      m_down.ToStruct(feature.analog_stick.down);
      m_right.ToStruct(feature.analog_stick.right);
      m_left.ToStruct(feature.analog_stick.left);
    }

  private:
    DriverPrimitive m_up;
    DriverPrimitive m_down;
    DriverPrimitive m_right;
    DriverPrimitive m_left;
  };

  /*!
   * ADDON::Accelerometer
   *
   * Accelerometers are defined by three axes. The positive semiaxis of each
   * axis is assigned to a driver primitive.
   *
   * The driver primitives should be semiaxes. The frontend may not support
   * turning digital events from buttons/hats into analog accelerations.
   */
  class Accelerometer : public JoystickFeature
  {
  public:
    Accelerometer(void) { }

    Accelerometer(const std::string& name, const DriverPrimitive& positiveX,
                                           const DriverPrimitive& positiveY,
                                           const DriverPrimitive& positiveZ) :
      JoystickFeature(name),
      m_positiveX(positiveX),
      m_positiveY(positiveY),
      m_positiveZ(positiveZ)
    {
    }

    Accelerometer(const JOYSTICK_FEATURE& feature) :
      JoystickFeature(feature),
      m_positiveX(feature.accelerometer.positive_x),
      m_positiveY(feature.accelerometer.positive_y),
      m_positiveZ(feature.accelerometer.positive_z)
    {
    }

    virtual ~Accelerometer(void) { }

    virtual JoystickFeature* Clone(void) const { return new Accelerometer(*this); }

    virtual bool Equals(const JoystickFeature* other) const
    {
      return JoystickFeature::Equals(other) &&
             m_positiveX.Equals(static_cast<const Accelerometer*>(other)->m_positiveX) &&
             m_positiveY.Equals(static_cast<const Accelerometer*>(other)->m_positiveY) &&
             m_positiveZ.Equals(static_cast<const Accelerometer*>(other)->m_positiveZ);
    }

    virtual JOYSTICK_FEATURE_TYPE Type(void) const      { return JOYSTICK_FEATURE_TYPE_ACCELEROMETER; }
    const DriverPrimitive&        PositiveX(void) const { return m_positiveX; }
    const DriverPrimitive&        PositiveY(void) const { return m_positiveY; }
    const DriverPrimitive&        PositiveZ(void) const { return m_positiveZ; }

    void SetPositiveX(const DriverPrimitive& primitive) { m_positiveX = primitive; }
    void SetPositiveY(const DriverPrimitive& primitive) { m_positiveY = primitive; }
    void SetPositiveZ(const DriverPrimitive& primitive) { m_positiveZ = primitive; }

    virtual void ToStruct(JOYSTICK_FEATURE& feature) const
    {
      JoystickFeature::ToStruct(feature);
      m_positiveX.ToStruct(feature.accelerometer.positive_x);
      m_positiveY.ToStruct(feature.accelerometer.positive_y);
      m_positiveZ.ToStruct(feature.accelerometer.positive_z);
    }

  private:
    DriverPrimitive m_positiveX;
    DriverPrimitive m_positiveY;
    DriverPrimitive m_positiveZ;
  };

  /*!
   * ADDON::JoystickFeatureFactory
   *
   * Utility class to create joystick features polymorphically
   */
  class JoystickFeatureFactory
  {
  public:
    static JoystickFeature* Create(const JOYSTICK_FEATURE& feature)
    {
      switch (feature.type)
      {
      case JOYSTICK_FEATURE_TYPE_PRIMITIVE:     return new PrimitiveFeature(feature);
      case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:  return new AnalogStick(feature);
      case JOYSTICK_FEATURE_TYPE_ACCELEROMETER: return new Accelerometer(feature);
      default:
        break;
      }
      return NULL;
    }
  };
}
