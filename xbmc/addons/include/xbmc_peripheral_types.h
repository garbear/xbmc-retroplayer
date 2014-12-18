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

#ifdef TARGET_WINDOWS
  #include <windows.h>
#else
  #ifndef __cdecl
    #define __cdecl
  #endif
  #ifndef __declspec
    #define __declspec(X)
  #endif
#endif

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
#define PERIPHERAL_API_VERSION "1.0.0"

/* min. Peripheral API version */
#define PERIPHERAL_MIN_API_VERSION "1.0.0"

#ifdef __cplusplus
extern "C"
{
#endif

  /// @name Peripheral types
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

  /// @name Joystick types
  ///{
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
    JOYSTICK_ID_BUTTON_UP,               /*!< @brief corresponds to Up on the directional pad */
    JOYSTICK_ID_BUTTON_DOWN,             /*!< @brief corresponds to Down on the directional pad */
    JOYSTICK_ID_BUTTON_LEFT,             /*!< @brief corresponds to Left on the directional pad */
    JOYSTICK_ID_BUTTON_RIGHT,            /*!< @brief corresponds to Right on the directional pad */
    JOYSTICK_ID_BUTTON_L,                /*!< @brief corresponds to Left shoulder button (generic) */
    JOYSTICK_ID_BUTTON_R,                /*!< @brief corresponds to Right shoulder button (generic) */
    JOYSTICK_ID_BUTTON_L2,               /*!< @brief corresponds to L2 (Sony) or ZL (Wii) */
    JOYSTICK_ID_BUTTON_R2,               /*!< @brief corresponds to R2 (Sony) or ZR (Wii) */
    JOYSTICK_ID_BUTTON_L_STICK,          /*!< @brief corresponds to Left stick (Xbox, Sony) */
    JOYSTICK_ID_BUTTON_R_STICK,          /*!< @brief corresponds to Right stick (Xbox, Sony) */
  } JOYSTICK_ID_BUTTON;

  typedef enum JOYSTICK_ID_TRIGGER
  {
    JOYSTICK_ID_TRIGGER_UNKNOWN,     /*!< @brief trigger isn't associated with an ID */
    JOYSTICK_ID_TRIGGER_L,           /*!< @brief corresponds to Left trigger (generic) or L2 (Sony) */
    JOYSTICK_ID_TRIGGER_R,           /*!< @brief corresponds to Right trigger (generic) or R2 (Sony) */
  } JOYSTICK_ID_TRIGGER;

  typedef enum JOYSTICK_ID_ANALOG_STICK
  {
    JOYSTICK_ID_ANALOG_STICK_UNKNOWN,      /*!< @brief analog stick isn't associated with an ID */
    JOYSTICK_ID_ANALOG_STICK_LEFT,         /*!< @brief corresponds to Left analog stick */
    JOYSTICK_ID_ANALOG_STICK_RIGHT,        /*!< @brief corresponds to Right analog stick */
  } JOYSTICK_ID_ANALOG_STICK;

  typedef struct JOYSTICK_MAP_BUTTON
  {
    unsigned int          index;  /*!< @brief the button index in the joystick configuration */
    JOYSTICK_ID_BUTTON    id;     /*!< @brief the standardized button ID */
    char*                 label;  /*!< @brief the label, e.g. "X" or "Square" for JOYSTICK_ID_BUTTON_3 */
  } ATTRIBUTE_PACKED JOYSTICK_MAP_BUTTON;

  typedef enum JOYSTICK_AXIS_SIGN
  {
    JOYSTICK_AXIS_NEGATIVE,     /*!< @brief axis has a negative value in the interval [-1, 0) */
    JOYSTICK_AXIS_POSITIVE,     /*!< @brief axis has a positive value in the interval (0, 1] */
  } JOYSTICK_AXIS_SIGN;

  typedef struct JOYSTICK_MAP_TRIGGER
  {
    unsigned int          axis_index;     /*!< @brief the axis index in the joystick configuration */
    JOYSTICK_AXIS_SIGN    axis_sign;      /*!< @brief sign of the axis that corresponds to positive trigger state */
    JOYSTICK_ID_TRIGGER   id;             /*!< @brief the standardized trigger ID */
    char*                 label;          /*!< @brief the label, e.g. "Z" for JOYSTICK_ID_TRIGGER_Z */
  } ATTRIBUTE_PACKED JOYSTICK_MAP_TRIGGER;

  typedef enum JOYSTICK_POSITIVE_DIR
  {
    JOYSTICK_POSITIVE_DIR_UP_RIGHT,   /*!< @brief positive axis value is up for vertical or right for horizontal */
    JOYSTICK_POSITIVE_DIR_DOWN_LEFT,  /*!< @brief positive axis value is down for vertical or left for horizontal */
  } JOYSTICK_POSITIVE_DIR;

  typedef struct JOYSTICK_MAP_ANALOG_STICK
  {
    unsigned int             horiz_axis_index;   /*!< @brief axis associated with horizontal motion */
    JOYSTICK_POSITIVE_DIR    horiz_positive_dir; /*!< @brief direction of positive motion for the horizontal axis */
    unsigned int             vert_axis_index;    /*!< @brief axis associated with vertical motion */
    JOYSTICK_POSITIVE_DIR    vert_positive_dir;  /*!< @brief direction of positive motion for the vertical axis */
    JOYSTICK_ID_ANALOG_STICK id;                 /*!< @brief the standardized stick ID */
    char*                    label;              /*!< @brief the label, e.g. "Left stick" for Left stick */
  } ATTRIBUTE_PACKED JOYSTICK_MAP_ANALOG_STICK;

  typedef struct JOYSTICK_LAYOUT
  {
    unsigned int               button_count;       /*!< @brief the number of buttons in this joystick map */
    unsigned int               trigger_count;      /*!< @brief the number of triggers in this joystick map */
    unsigned int               analog_stick_count; /*!< @brief the number of analog sticks in this joystick map */
    JOYSTICK_MAP_BUTTON*       buttons;            /*!< @brief the button mappings */
    JOYSTICK_MAP_TRIGGER*      triggers;           /*!< @brief the trigger mappings */
    JOYSTICK_MAP_ANALOG_STICK* analog_sticks;      /*!< @brief the analog stick mappings */
  } ATTRIBUTE_PACKED JOYSTICK_LAYOUT;

  typedef struct JOYSTICK_CONFIGURATION
  {
    unsigned int    index;            /*!< @brief joystick index unique to the add-on */
    unsigned int    requested_player; /*!< @brief 0 if unused, or joystick's requested player number (e.g. 1-4 for Xbox 360/PS3/Wii) */
    char*           name;             /*!< @brief Joystick's display name */
    char*           icon_path;        /*!< @brief unused; reserved for future use */
    unsigned int    button_count;     /*!< @brief number of buttons in this joystick configuration */
    unsigned int    hat_count;        /*!< @brief number of hats in this joystick configuration */
    unsigned int    axis_count;       /*!< @brief number of axes in this joystick configuration */
    unsigned int*   buttons;          /*!< @brief array of unique button indexes, usually the index reported by the driver */
    unsigned int*   hats;             /*!< @brief array of unique hat indexes */
    unsigned int*   axes;             /*!< @brief array of unique axis indexes, usually the index reported by the driver */
    JOYSTICK_LAYOUT layout;           /*!< @brief the mapping of raw buttons, hats and axes to higher-level elements */
  } ATTRIBUTE_PACKED JOYSTICK_CONFIGURATION;

  typedef enum JOYSTICK_STATE_BUTTON
  {
    JOYSTICK_STATE_BUTTON_UP   = 0x0,    /*!< @brief button is unpressed */
    JOYSTICK_STATE_BUTTON_DOWN = 0x1,    /*!< @brief button is pressed */
  } JOYSTICK_STATE_BUTTON;

  typedef enum JOYSTICK_STATE_HAT
  {
    JOYSTICK_STATE_HAT_UNPRESSED  = 0x0,    /*!< @brief no directions are pressed */
    JOYSTICK_STATE_HAT_UP         = 0x1,    /*!< @brief only up is pressed */
    JOYSTICK_STATE_HAT_DOWN       = 0x2,    /*!< @brief only down is pressed */
    JOYSTICK_STATE_HAT_LEFT       = 0x4,    /*!< @brief only left is pressed */
    JOYSTICK_STATE_HAT_RIGHT      = 0x8,    /*!< @brief only right is pressed */
    JOYSTICK_STATE_HAT_UP_LEFT    = JOYSTICK_STATE_HAT_UP   | JOYSTICK_STATE_HAT_LEFT,
    JOYSTICK_STATE_HAT_UP_RIGHT   = JOYSTICK_STATE_HAT_UP   | JOYSTICK_STATE_HAT_RIGHT,
    JOYSTICK_STATE_HAT_DOWN_LEFT  = JOYSTICK_STATE_HAT_DOWN | JOYSTICK_STATE_HAT_LEFT,
    JOYSTICK_STATE_HAT_DOWN_RIGHT = JOYSTICK_STATE_HAT_DOWN | JOYSTICK_STATE_HAT_RIGHT,
  } JOYSTICK_STATE_HAT;

  typedef float                 JOYSTICK_STATE_AXIS;     /*!< @brief value in the interval [-1, 1], inclusive */

  typedef JOYSTICK_STATE_AXIS   JOYSTICK_STATE_TRIGGER;  /*!< @brief value in the interval [0, 1], inclusive */

  typedef struct JOYSTICK_STATE_ANALOG_STICK
  {
    JOYSTICK_STATE_AXIS   state_horiz;                   /*!< @brief state of the horizontal axis */
    JOYSTICK_STATE_AXIS   state_vert;                    /*!< @brief state of the vertical axis */
  } ATTRIBUTE_PACKED JOYSTICK_STATE_ANALOG_STICK;

  typedef enum JOYSTICK_EVENT_TYPE
  {
    JOYSTICK_EVENT_TYPE_NONE = 0,            /*!< @brief null event */
    JOYSTICK_EVENT_TYPE_RAW_BUTTON,          /*!< @brief state changed for raw button, reported by its index */
    JOYSTICK_EVENT_TYPE_RAW_HAT,             /*!< @brief state changed for raw hat, reported by its index */
    JOYSTICK_EVENT_TYPE_RAW_AXIS,            /*!< @brief state changed for raw axis, reported by its index */
    JOYSTICK_EVENT_TYPE_MAPPED_BUTTON,       /*!< @brief state changed for button ID mapped to a raw button */
    JOYSTICK_EVENT_TYPE_MAPPED_TRIGGER,      /*!< @brief state changed for trigger ID mapped to a raw axis */
    JOYSTICK_EVENT_TYPE_MAPPED_ANALOG_STICK, /*!< @brief state changed for analog stick mapped to raw axes */
  } JOYSTICK_EVENT_TYPE;

  typedef struct JOYSTICK_EVENT_RAW_BUTTON
  {
    unsigned int                index;       /*!< @brief the button index in the joystick configuration */
    JOYSTICK_STATE_BUTTON       state;       /*!< @brief the new button state */
  } ATTRIBUTE_PACKED JOYSTICK_EVENT_RAW_BUTTON;

  typedef struct JOYSTICK_EVENT_RAW_HAT
  {
    unsigned int                index;       /*!< @brief the hat index in the joystick configuration */
    JOYSTICK_STATE_HAT          state;       /*!< @brief the new hat state */
  } ATTRIBUTE_PACKED JOYSTICK_EVENT_RAW_HAT;

  typedef struct JOYSTICK_EVENT_RAW_AXIS
  {
    unsigned int                index;       /*!< @brief the axis index in the joystick configuration */
    JOYSTICK_STATE_AXIS         state;       /*!< @brief the new axis state */
  } ATTRIBUTE_PACKED JOYSTICK_EVENT_RAW_AXIS;

  typedef struct JOYSTICK_EVENT_MAPPED_BUTTON
  {
    JOYSTICK_ID_BUTTON          id;          /*!< @brief the standardized button ID  */
    JOYSTICK_STATE_BUTTON       state;       /*!< @brief the new button state */
  } ATTRIBUTE_PACKED JOYSTICK_EVENT_MAPPED_BUTTON;

  typedef struct JOYSTICK_EVENT_MAPPED_TRIGGER
  {
    JOYSTICK_ID_TRIGGER         id;          /*!< @brief the standardized trigger ID  */
    JOYSTICK_STATE_TRIGGER      state;       /*!< @brief the new trigger state */
  } ATTRIBUTE_PACKED JOYSTICK_EVENT_MAPPED_TRIGGER;

  typedef struct JOYSTICK_EVENT_MAPPED_ANALOG_STICK
  {
    JOYSTICK_ID_ANALOG_STICK    id;          /*!< @brief the standardized analog stick ID  */
    JOYSTICK_STATE_ANALOG_STICK state;       /*!< @brief the new analog stick state */
  } ATTRIBUTE_PACKED JOYSTICK_EVENT_MAPPED_ANALOG_STICK;

  typedef struct JOYSTICK_EVENT
  {
    JOYSTICK_EVENT_TYPE   type;           /*!< @brief the event type */
    unsigned int          joystick_index; /*!< @brief the joystick associated with this event */
    void*                 event;          /*!< @brief a pointer to the event structure of the specified type */
  } ATTRIBUTE_PACKED JOYSTICK_EVENT;
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
    PERIPHERAL_ERROR (__cdecl* PerformJoystickScan)(unsigned int*, JOYSTICK_CONFIGURATION**);
    void             (__cdecl* FreeJoysticks)(unsigned int, JOYSTICK_CONFIGURATION*);
    PERIPHERAL_ERROR (__cdecl* RegisterButton)(unsigned int, JOYSTICK_MAP_BUTTON*);
    PERIPHERAL_ERROR (__cdecl* UnregisterButton)(unsigned int, unsigned int);
    PERIPHERAL_ERROR (__cdecl* RegisterTrigger)(unsigned int, JOYSTICK_MAP_TRIGGER*);
    PERIPHERAL_ERROR (__cdecl* UnregisterTrigger)(unsigned int, unsigned int);
    PERIPHERAL_ERROR (__cdecl* RegisterAnalogStick)(unsigned int, JOYSTICK_MAP_ANALOG_STICK*);
    PERIPHERAL_ERROR (__cdecl* UnregisterAnalogStick)(unsigned int, unsigned int);
    PERIPHERAL_ERROR (__cdecl* GetEvents)(unsigned int*, JOYSTICK_EVENT**);
    void             (__cdecl* FreeEvents)(unsigned int, JOYSTICK_EVENT*);
    ///}
  } PeripheralAddon;

#ifdef __cplusplus
}
#endif

#endif // __PERIPHERAL_TYPES_H__
