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
#ifndef __PERIPHERAL_TYPES_H__
#define __PERIPHERAL_TYPES_H__

#undef ATTRIBUTE_PACKED
#undef PRAGMA_PACK_BEGIN
#undef PRAGMA_PACK_END

#if defined(__GNUC__)
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define ATTRIBUTE_PACKED __attribute__ ((packed))
#define PRAGMA_PACK 0
#endif
#endif

#if !defined(ATTRIBUTE_PACKED)
#define ATTRIBUTE_PACKED
#define PRAGMA_PACK 1
#endif

/* current Peripheral API version */
#define XBMC_PERIPHERAL_API_VERSION "1.0.0"

/* min. Peripheral API version */
#define XBMC_PERIPHERAL_MIN_API_VERSION "1.0.0"

#ifdef __cplusplus
extern "C"
{
#endif

  /*!
   * Peripheral types
   */
  ///{
  typedef enum PERIPHERAL_ERROR
  {
    PERIPHERAL_NO_ERROR                      =  0, // no error occurred
    PERIPHERAL_ERROR_UNKNOWN                 = -1, // an unknown error occurred
    PERIPHERAL_ERROR_FAILED                  = -2, // the command failed
    PERIPHERAL_ERROR_INVALID_PARAMETERS      = -3, // the parameters of the method are invalid for this operation
    PERIPHERAL_ERROR_NOT_IMPLEMENTED         = -4, // the method that the frontend called is not implemented
    PERIPHERAL_ERROR_NOT_CONNECTED           = -5, // no peripherals are connected
    PERIPHERAL_ERROR_CONNECTION_FAILED       = -6, // peripherals are connected, but command was interrupted
  } PERIPHERAL_ERROR;

  /*!
   * @brief Properties passed to the Create() method of an add-on.
   */
  typedef struct PERIPHERAL_PROPERTIES
  {
    const char* user_path;              /*!< @brief path to the user profile */
    const char* addon_path;             /*!< @brief path to this add-on */
  } PERIPHERAL_PROPERTIES;

  /*!
   * @brief Peripheral add-on capabilities. All capabilities are set to "false" by default.
   * If a capability is set to true, then the corresponding methods from
   * xbmc_peripheral_dll.h need to be implemented.
   */
  typedef struct PERIPHERAL_CAPABILITIES
  {
    bool bProvidesJoysticks;            /*!< @brief true if the add-on provides joysticks */
  } ATTRIBUTE_PACKED PERIPHERAL_CAPABILITIES;
  ///}

  /*!
   * Joystick types
   *
   * The joystick abstraction is split into four distinct elements:
   *   - Button:  an element with a pressed/unpressed state
   *   - Hat:     an element with four directions; opposing directions are mutually exclusive
   *   - Trigger: an element with a value strictly in the interval [0, 1], inclusive
   *   - Stick:    an element with a value in the interval [-1, 1], inclusive
   */
  ///{
  /*!
   * @brief Button abstraction
   */
  typedef enum JOYSTICK_BUTTON_STATE
  {
    JOYSTICK_BUTTON_STATE_UP   = 0x0,    /*!< @brief button is unpressed */
    JOYSTICK_BUTTON_STATE_DOWN = 0x1,    /*!< @brief button is pressed */
  } JOYSTICK_BUTTON_STATE;

  typedef enum JOYSTICK_ID_BUTTON
  {
    JOYSTICK_ID_BUTTON_UNKNOWN = 0,      /*!< @brief no data exists to associate button with ID */
    JOYSTICK_ID_BUTTON_1,                /*!< @brief corresponds to A (generic) or Cross (Sony) */
    JOYSTICK_ID_BUTTON_2,                /*!< @brief corresponds to B (generic) or Circle (Sony) */
    JOYSTICK_ID_BUTTON_3,                /*!< @brief corresponds to C or X (generic), Square (Sony), C-down (N64) or One (Wii)*/
    JOYSTICK_ID_BUTTON_4,                /*!< @brief corresponds to Y (generic), Triangle (Sony), C-left (N64) or Two (Wii) */
    JOYSTICK_ID_BUTTON_5,                /*!< @brief corresponds to Black (Xbox) or C-right (N64) */
    JOYSTICK_ID_BUTTON_6,                /*!< @brief corresponds to White (Xbox) or C-up (N64) */
    JOYSTICK_ID_BUTTON_START,            /*!< @brief corresponds to Start (generic) */
    JOYSTICK_ID_BUTTON_SELECT,           /*!< @brief corresponds to Select (generic) or Back (Xbox) */
    JOYSTICK_ID_BUTTON_HAT_UP,           /*!< @brief corresponds to Up on the directional pad */
    JOYSTICK_ID_BUTTON_HAT_DOWN,         /*!< @brief corresponds to Down on the directional pad */
    JOYSTICK_ID_BUTTON_HAT_LEFT,         /*!< @brief corresponds to Left on the directional pad */
    JOYSTICK_ID_BUTTON_HAT_RIGHT,        /*!< @brief corresponds to Right on the directional pad */
    JOYSTICK_ID_BUTTON_L,                /*!< @brief corresponds to Left shoulder button (generic) */
    JOYSTICK_ID_BUTTON_R,                /*!< @brief corresponds to Right shoulder button (generic) */
    JOYSTICK_ID_BUTTON_L2,               /*!< @brief corresponds to L2 (Sony) or ZL (Wii) */
    JOYSTICK_ID_BUTTON_R2,               /*!< @brief corresponds to R2 (Sony) or ZR (Wii) */
    JOYSTICK_ID_BUTTON_L3,               /*!< @brief corresponds to Left stick (Xbox, Sony) */
    JOYSTICK_ID_BUTTON_R3,               /*!< @brief corresponds to Right stick (Xbox, Sony) */
    JOYSTICK_ID_BUTTON_TURBO,            /*!< @brief corresponds to Turbo button seen on some joysticks */
  } JOYSTICK_ID_BUTTON;

  typedef struct JOYSTICK_BUTTON
  {
    unsigned int          index;         /*!< @brief unique to each joystick, usually the index reported by the driver */
    JOYSTICK_ID_BUTTON    button_id;     /*!< @brief the standardized button ID expected by the game */
    char*                 label;         /*!< @brief the label, e.g. "X" or "Square" for JOYSTICK_ID_BUTTON_3 */
    JOYSTICK_BUTTON_STATE state;         /*!< @brief the button state */
  } ATTRIBUTE_PACKED JOYSTICK_BUTTON;

  /*!
   * @brief Hat abstraction
   */
  typedef enum JOYSTICK_HAT_STATE
  {
    JOYSTICK_HAT_STATE_UNPRESSED  = 0x0,    /*!< @brief no directions are pressed */
    JOYSTICK_HAT_STATE_UP         = 0x1,    /*!< @brief only up is pressed */
    JOYSTICK_HAT_STATE_DOWN       = 0x2,    /*!< @brief only down is pressed */
    JOYSTICK_HAT_STATE_LEFT       = 0x4,    /*!< @brief only left is pressed */
    JOYSTICK_HAT_STATE_RIGHT      = 0x8,    /*!< @brief only right is pressed */
    JOYSTICK_HAT_STATE_UP_LEFT    = JOYSTICK_HAT_STATE_UP   | JOYSTICK_HAT_STATE_LEFT,
    JOYSTICK_HAT_STATE_UP_RIGHT   = JOYSTICK_HAT_STATE_UP   | JOYSTICK_HAT_STATE_RIGHT,
    JOYSTICK_HAT_STATE_DOWN_LEFT  = JOYSTICK_HAT_STATE_DOWN | JOYSTICK_HAT_STATE_LEFT,
    JOYSTICK_HAT_STATE_DOWN_RIGHT = JOYSTICK_HAT_STATE_DOWN | JOYSTICK_HAT_STATE_RIGHT,
  } JOYSTICK_HAT_STATE;

  typedef struct JOYSTICK_HAT
  {
    unsigned int          index;       /*!< @brief unique to each joystick */
    JOYSTICK_HAT_STATE    state;       /*!< @brief the hat state */
  } ATTRIBUTE_PACKED JOYSTICK_HAT;

  /*!
   * @brief Raw axis information
   */
  typedef struct JOYSTICK_AXIS
  {
    unsigned int          index;       /*!< @brief unique to each joystick, usually the index reported by the driver */
    float                 state;       /*!< @brief value in the interval [-1, 1] inclusive */
  } ATTRIBUTE_PACKED JOYSTICK_AXIS;

  /*!
   * @brief Analog trigger abstraction
   */
  typedef enum JOYSTICK_ID_TRIGGER
  {
    JOYSTICK_ID_TRIGGER_UNKNOWN,     /*!< @brief trigger isn't associated with an ID */
    JOYSTICK_ID_TRIGGER_L,           /*!< @brief corresponds to Left trigger (generic) or L2 (Sony) */
    JOYSTICK_ID_TRIGGER_R,           /*!< @brief corresponds to Right trigger (generic) or R2 (Sony) */
    JOYSTICK_ID_TRIGGER_Z,           /*!< @brief corresponds to single trigger (generic) or Z (N64) */
  } JOYSTICK_ID_TRIGGER;

  typedef enum JOYSTICK_AXIS_SIGN
  {
    JOYSTICK_AXIS_NEGATIVE,
    JOYSTICK_AXIS_POSITIVE,
  } JOYSTICK_AXIS_SIGN;

  typedef struct JOYSTICK_TRIGGER
  {
    unsigned int          index;          /*!< @brief unique to each joystick, set by the frontend */
    unsigned int          axis_index;     /*!< @brief axis associated with trigger motion */
    JOYSTICK_AXIS_SIGN    axis_sign;      /*!< @brief sign of the axis that corresponds to positive trigger state */
    JOYSTICK_ID_TRIGGER   trigger_id;     /*!< @brief the standardized trigger ID expected by the game */
    char*                 label;          /*!< @brief the label, e.g. "Z" for JOYSTICK_ID_TRIGGER_Z */
    float                 state;          /*!< @brief value strictly in the interval [0, 1], inclusive */
  } ATTRIBUTE_PACKED JOYSTICK_TRIGGER;

  /*!
   * @brief Analog stick abstraction
   */
  typedef enum JOYSTICK_DIRECTION
  {
    JOYSTICK_DIRECTION_UP_RIGHT,   /*!< @brief positive axis value is up for vertical or right for horizontal */
    JOYSTICK_DIRECTION_DOWN_LEFT,  /*!< @brief positive axis value is down for vertical or left for horizontal */
  } JOYSTICK_DIRECTION;

  typedef enum JOYSTICK_ID_ANALOG_STICK
  {
    JOYSTICK_ID_ANALOG_STICK_UNKNOWN,      /*!< @brief analog stick isn't associated with an ID */
    JOYSTICK_ID_ANALOG_STICK_LEFT,         /*!< @brief corresponds to Left stick moving horizontal; -1 is left, +1 is right */
    JOYSTICK_ID_ANALOG_STICK_RIGHT,        /*!< @brief corresponds to Left stick moving vertical; -1 is down, +1 is up */
  } JOYSTICK_ID_ANALOG_STICK;

  typedef struct JOYSTICK_ANALOG_STICK
  {
    unsigned int             index;                   /*!< @brief unique to each joystick, set by the frontend */
    unsigned int             horiz_axis_index;        /*!< @brief axis associated with horizontal motion */
    JOYSTICK_DIRECTION       horiz_axis_positive_dir; /*!< @brief direction of positive motion for the horizontal axis */
    unsigned int             vert_axis_index;         /*!< @brief axis associated with vertical motion */
    JOYSTICK_DIRECTION       vert_axis_positive_dir;  /*!< @brief direction of positive motion for the vertical axis */
    JOYSTICK_ID_ANALOG_STICK analog_stick_id;         /*!< @brief the standardized stick ID expected by the game */
    char*                    label;                   /*!< @brief the label, e.g. "Left stick" for the left stick */
    float                    horiz_state;             /*!< @brief Value strictly in the interval [0, 1], inclusive */
    float                    vert_state;              /*!< @brief Value strictly in the interval [0, 1], inclusive */
  } ATTRIBUTE_PACKED JOYSTICK_ANALOG_STICK;

  /*!
   * @brief Joystick abstraction
   */
  typedef struct JOYSTICK
  {
    unsigned int           index;            /*!< @brief joystick index unique to the add-on */
    unsigned int           requested_player; /*!< @brief 0 if unused, or joystick's player number (e.g. 1-4 for Xbox 360/PS3/Wii) */
    char*                  name;             /*!< @brief Joystick's display name */
    char*                  icon_path;        /*!< @brief Unused; reserved for future use */
    unsigned int           button_count;
    unsigned int           hat_count;
    unsigned int           axis_count;
    unsigned int           trigger_count;
    unsigned int           analog_stick_count;
    JOYSTICK_BUTTON*       buttons;
    JOYSTICK_HAT*          hats;
    JOYSTICK_AXIS*         axes;
    JOYSTICK_TRIGGER*      triggers;
    JOYSTICK_ANALOG_STICK* analog_sticks;
  } ATTRIBUTE_PACKED JOYSTICK;
  ///}

  // TODO: Mouse, light gun, multitouch

  /*!
   * @brief Structure to transfer the methods from xbmc_peripheral_dll.h to the frontend
   */
  typedef struct PeripheralAddon
  {
    const char*      (__cdecl* GetPeripheralAPIVersion)(void);
    const char*      (__cdecl* GetMinimumPeripheralAPIVersion)(void);
    PERIPHERAL_ERROR (__cdecl* GetAddonCapabilities)(PERIPHERAL_CAPABILITIES*);

    /// @name Joystick operations
    ///{
    PERIPHERAL_ERROR (__cdecl* PerformJoystickScan)(unsigned int*, JOYSTICK**);
    void             (__cdecl* FreeJoysticks)(unsigned int, JOYSTICK*);
    PERIPHERAL_ERROR (__cdecl* SetButton)(unsigned int, JOYSTICK_ID_BUTTON, const char*);
    PERIPHERAL_ERROR (__cdecl* AddTrigger)(JOYSTICK_TRIGGER*);
    PERIPHERAL_ERROR (__cdecl* RemoveTrigger)(unsigned int);
    PERIPHERAL_ERROR (__cdecl* AddAnalogStick)(JOYSTICK_ANALOG_STICK*);
    PERIPHERAL_ERROR (__cdecl* RemoveAnalogStick)(unsigned int);
    ///}
  } PeripheralAddon;

#ifdef __cplusplus
}
#endif

#endif // __PERIPHERAL_TYPES_H__
