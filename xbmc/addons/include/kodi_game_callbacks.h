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
#ifndef KODI_GAME_CALLBACKS_H_
#define KODI_GAME_CALLBACKS_H_

#include "kodi_game_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CB_GameLib
{
  // --- Game callbacks --------------------------------------------------------

  /*!
   * \brief Requests the frontend to stop the current game
   */
  void (*CloseGame)(void* addonData);

  /*!
   * \brief Render a frame
   *
   * \param format The format of the frame being rendered
   * \param width The width, in pixels
   * \param height The height, in pixels
   * \param data The frame's data
   */
  void (*VideoFrame)(void* addonData, const uint8_t* data, unsigned int width, unsigned int height, GAME_RENDER_FORMAT format);

  /*!
   * \brief Render a chunk of audio data
   *
   * \param data The audio data
   * \param frames The number of frames; a left and right sample make one frame
   * \param format The format of the audio data
   */
  void (*AudioFrames)(void* addonData, const uint8_t* data, unsigned int frames, GAME_AUDIO_FORMAT format);

  // -- Hardware rendering callbacks -------------------------------------------

  /*!
   * \brief Set info for hardware rendering
   *
   * \param hw_info A struct of properties for the hardware rendering system
   */
  void (*HwSetInfo)(void* addonData, const game_hw_info* hw_info);

  /*!
   * \brief Get the framebuffer for rendering
   *
   * \return The framebuffer
   */
  uintptr_t (*HwGetCurrentFramebuffer)(void* addonData);

  /*!
   * \brief Get a symbol from the hardware context
   *
   * \param symbol The symbol's name
   *
   * \return A function pointer for the specified symbol
   */
  game_proc_address_t (*HwGetProcAddress)(void* addonData, const char* symbol);

  // --- Input callbacks -------------------------------------------------------

  /*!
   * \brief Begin reporting events for the specified port
   *
   * \param port The zero-indexed port number
   */
  bool (*OpenPort)(void* addonData, unsigned int port);

  /*!
   * \brief End reporting events for the specified port
   *
   * \param port The port number passed to OpenPort()
   */
  void (*ClosePort)(void* addonData, unsigned int port);

  /*!
   * \brief Set the rumble state of a controller
   *
   * \param port The number passed when opening the port
   * \param effect Apply the rumble to the strong motor or the weak motor
   * \param strength The magnitude of the feedback in the closed interval [0.0, 1.0]
   */
  void (*RumbleSetState)(void* addonData, unsigned int port, GAME_RUMBLE_EFFECT effect, float strength);

  // --- Camera callbacks ------------------------------------------------------

  /*!
   * \brief Notify the frontend
   *
   * \param width The desired width for the camera
   * \param height The desired height for the camera
   * \param caps The types of buffers that can be handled by the add-on
   *
   * Depending on the camera implementation used, camera frames will be
   * delivered as a raw framebuffer, or as an OpenGL texture directly.
  */
  void (*SetCameraInfo)(void* addonData, unsigned int width, unsigned int height, GAME_CAMERA_BUFFER caps);

  /*!
   * \brief Start the camera
   *
   * \return true if the camera was started
   */
  bool (*StartCamera)(void* addonData);

  /*!
   * \brief Stop the camera driver
   */
  void (*StopCamera)(void* addonData);

  // --- Location callbacks ----------------------------------------------------

  /*!
   * \brief Start location services
   *
   * This enables the add-on to retrieve location-based information from the
   * host device, such as current latitude / longitude. The device will start
   * listening for changes to the current location at defined intervals.
   *
   * \return true if the location services were started
   */
  bool (*StartLocation)(void* addonData);

  /*!
   * \brief Stop location services
   *
   * The device will stop listening for changes to the current location.
   */
  void (*StopLocation)(void* addonData);

  /*!
   * \brief Get the current location
   *
   * \param lat The latitude
   * \param lon The longitude
   * \param horizAccuracy The horizontal accuracy
   * \param vertAccuracy The vertical accuracy
   *
   * \return true if the current location was retrieved
   */
  bool (*GetLocation)(void* addonData, double* lat, double* lon, double* horizAccuracy, double* vertAccuracy);

  /*!
   * \brief Set the interval of time and/or distance at which to sample location
   *
   * To ensure compatibility with all location-based implementations, values for
   * both intervalMs and intervalDistance should be provided.
   *
   * \param intervalMs The interval expressed in milliseconds
   * \param intervalDistance The distance interval expressed in meters
   */
  void (*SetLocationInterval)(void* addonData, unsigned int intervalMs, unsigned int intervalDistance);

} CB_GameLib;

#ifdef __cplusplus
}
#endif

#endif // KODI_GAME_CALLBACKS_H_
