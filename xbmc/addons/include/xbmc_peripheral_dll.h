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
#ifndef __PERIPHERAL_DLL_H__
#define __PERIPHERAL_DLL_H__

#include "xbmc_peripheral_types.h"

#define PERIPHERAL_ADDON_JOYSTICKS // temp

#ifdef __cplusplus
extern "C"
{
#endif

  /// @name Peripheral operations
  ///{
  /*!
   * @brief  Get the PERIPHERAL_API_VERSION_STRING used to compile this peripheral add-on
   * @return XBMC_PERIPHERAL_API_VERSION from xbmc_peripheral_types
   * @remarks Valid implementation required
   *
   * Used to check if the implementation is compatible with the frontend.
   */
  const char* GetPeripheralAPIVersion(void);

  /*!
   * @brief Get the PERIPHERAL_MIN_API_VERSION_STRING used to compile this peripheral add-on
   * @return XBMC_PERIPHERAL_MIN_API_VERSION from xbmc_peripheral_types
   * @remarks Valid implementation required
   *
   * Used to check if the implementation is compatible with the frontend.
   */
  const char* GetMinimumPeripheralAPIVersion(void);

  /*!
   * @brief Get the list of features that this add-on provides
   * @param pCapabilities The add-on's capabilities.
   * @return PERIPHERAL_NO_ERROR if the properties were fetched successfully.
   * @remarks Valid implementation required.
   *
   * Called by the frontend to query the add-on's capabilities and supported
   * peripherals. All capabilities that the add-on supports should be set to true.
   *
   */
  PERIPHERAL_ERROR GetAddonCapabilities(PERIPHERAL_CAPABILITIES *pCapabilities);
  ///}

  /*!
   * Joystick operations
   *
   * @note #define PERIPHERAL_ADDON_JOYSTICKS before including xbmc_peripheral_dll.h
   * in the add-on if the add-on provides joysticks and add provides_joysticks="true"
   * to the xbmc.peripheral extension point node in addon.xml.
   */
  ///{
#ifdef PERIPHERAL_ADDON_JOYSTICKS
  /*!
   * @brief Perform a scan for joysticks
   * @return PERIPHERAL_NO_ERROR if successful; joysticks must be freed using
   * FreeJoysticks() in this case
   *
   * The frontend calls this when a hardware change is detected. If an add-on
   * detects a hardware change, it can trigger this function using the
   * TriggerScan() callback.
   */
  PERIPHERAL_ERROR PerformJoystickScan(unsigned int* joystick_count, JOYSTICK_CONFIGURATION** joysticks);
  PERIPHERAL_ERROR FreeJoysticks(unsigned int joystick_count, JOYSTICK_CONFIGURATION* joysticks);

  /*!
   * @brief Assign a button index to a button ID and label
   */
  PERIPHERAL_ERROR RegisterButton(unsigned int joystick_index, JOYSTICK_MAP_BUTTON* button);
  PERIPHERAL_ERROR UnregisterButton(unsigned int joystick_index, unsigned int button_index);

  /*!
   * @brief Assign an axis index and direction to a trigger ID and label
   */
  PERIPHERAL_ERROR RegisterTrigger(unsigned int joystick_index, JOYSTICK_MAP_TRIGGER* trigger);
  PERIPHERAL_ERROR UnregisterTrigger(unsigned int joystick_index, unsigned int trigger_index);

  /*!
   * @brief Assign two axes to an analog stick ID and label
   */
  PERIPHERAL_ERROR RegisterAnalogStick(unsigned int joystick_index, JOYSTICK_MAP_ANALOG_STICK* analog_stick);
  PERIPHERAL_ERROR UnregisterAnalogStick(unsigned int joystick_index, unsigned int analog_stick_index);

  /*!
   * @brief Get all events that have occurred since the last call to GetEvents()
   * @return PERIPHERAL_NO_ERROR if successful; events must be freed using
   * FreeEvents() in this case
   */
  PERIPHERAL_ERROR GetEvents(unsigned int* event_count, JOYSTICK_EVENT** events);
  PERIPHERAL_ERROR FreeEvents(unsigned int event_count, JOYSTICK_EVENT* events);
#endif
  ///}

  /*!
   * Called by the frontend to assign the function pointers of this add-on to
   * pClient. Note that get_addon() is defined here, so it will be available in
   * all compiled peripheral add-ons.
   */
  void __declspec(dllexport) get_addon(struct PeripheralAddon* pClient)
  {
    pClient->GetPeripheralAPIVersion        = GetPeripheralAPIVersion;
    pClient->GetMinimumPeripheralAPIVersion = GetMinimumPeripheralAPIVersion;
    pClient->GetAddonCapabilities           = GetAddonCapabilities;

#ifdef PERIPHERAL_ADDON_JOYSTICKS
    pClient->PerformJoystickScan            = PerformJoystickScan;
    pClient->FreeJoysticks                  = FreeJoysticks;
    pClient->RegisterButton                 = RegisterButton;
    pClient->UnregisterButton               = UnregisterButton;
    pClient->RegisterTrigger                = RegisterTrigger;
    pClient->UnregisterTrigger              = UnregisterTrigger;
    pClient->RegisterAnalogStick            = RegisterAnalogStick;
    pClient->UnregisterAnalogStick          = UnregisterAnalogStick;
    pClient->GetEvents                      = GetEvents;
    pClient->FreeEvents                     = FreeEvents;
#endif
  };

#ifdef __cplusplus
};
#endif

#endif // __PERIPHERAL_DLL_H__
