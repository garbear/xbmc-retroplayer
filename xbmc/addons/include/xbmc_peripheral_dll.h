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

#ifdef __cplusplus
extern "C"
{
#endif

  /*!
   * Peripheral operations
   */
  ///{
  /*!
   * Get the PERIPHERAL_API_VERSION_STRING that was used to compile this
   * peripheral add-on. Used to check if the implementation is compatible with
   * the frontend.
   */
  const char* GetPeripheralAPIVersion(void);

  /*!
   * Get the PERIPHERAL_MIN_API_VERSION_STRING that was used to compile this
   * peripheral add-on. Used to check if the implementation is compatible with
   * the frontend.
   */
  const char* GetMinimumPeripheralAPIVersion(void);

  /*!
   * Get the list of features that this add-on provides.
   * Called by the frontend to query the add-on's capabilities and supported
   * peripherals. All capabilities that the add-on supports should be set to true.
   * @param pCapabilities The add-on's capabilities.
   * @return PERIPHERAL_NO_ERROR if the properties were fetched successfully.
   * @remarks Valid implementation required.
   */
  PERIPHERAL_ERROR GetAddonCapabilities(PERIPHERAL_CAPABILITIES *pCapabilities);
  ///}

  /*!
   * Joystick operations
   */
  ///{
  /*!
   * @brief Perform a scan for joysticks
   * The frontend calls this when a hardware change is detected. If an add-on
   * detects a hardware change, it can trigger this function using the
   * TriggerScan() callback.
   *
   * @return PERIPHERAL_NO_ERROR if successful. Joysticks must be freed using FreeJoysticks() in this case.
   */
  PERIPHERAL_ERROR PerformJoystickScan(unsigned int* joystick_count, JOYSTICK** joysticks);
  void             FreeJoysticks(unsigned int joystick_count, JOYSTICK* joysticks);

  /*!
   * @brief Assign the specified button a standardized ID and label
   */
  PERIPHERAL_ERROR SetButton(unsigned int button_index, JOYSTICK_ID_BUTTON new_id, const char* new_label);

  /*!
   * @brief Add/remove a trigger
   */
  PERIPHERAL_ERROR AddTrigger(JOYSTICK_TRIGGER* trigger);
  PERIPHERAL_ERROR RemoveTrigger(unsigned int trigger_index);

  /*!
   * @brief Add/remove an analog stick
   */
  PERIPHERAL_ERROR AddStick(JOYSTICK_ANALOG_STICK* stick);
  PERIPHERAL_ERROR RemoveStick(unsigned int stick_index);
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
    pClient->PerformJoystickScan            = PerformJoystickScan;
    pClient->FreeJoysticks                  = FreeJoysticks;
    pClient->SetButton                      = SetButton;
    pClient->AddTrigger                     = AddTrigger;
    pClient->RemoveTrigger                  = RemoveTrigger;
    pClient->AddStick                       = AddStick;
    pClient->RemoveStick                    = RemoveStick;
  };

#ifdef __cplusplus
};
#endif

#endif // __PERIPHERAL_DLL_H__
