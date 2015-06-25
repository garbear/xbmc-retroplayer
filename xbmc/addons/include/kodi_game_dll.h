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
#ifndef KODI_GAME_DLL_H_
#define KODI_GAME_DLL_H_

#include "kodi_game_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// --- Game API operations -----------------------------------------------------

/*!
 * \brief Return GAME_API_VERSION_STRING
 *
 * The add-on is backwards compatible with the frontend if this API version is
 * is at least the frontend's minimum API version.
 *
 * \return Must be GAME_API_VERSION_STRING
 */
const char* GetGameAPIVersion(void);

/*!
 * \brief Return GAME_MIN_API_VERSION_STRING
 *
 * The add-on is forwards compatible with the frontend if this minimum version
 * is no more than the frontend's API version.
 *
 * \return Must be GAME_MIN_API_VERSION_STRING
 */
const char* GetMininumGameAPIVersion(void);

// --- Game operations ---------------------------------------------------------

/*!
 * \brief Load a game
 *
 * \param url The URL to load
 *
 * return the error, or GAME_ERROR_NO_ERROR if the game was loaded
 */
GAME_ERROR LoadGame(const char* url);

/*!
 * \brief Load a game that requires multiple files
 *
 * \param type The game stype
 * \param urls An array of urls
 * \param urlCount The number of urls in the array
 *
 * \return the error, or GAME_ERROR_NO_ERROR if the game was loaded
 */
GAME_ERROR LoadGameSpecial(SPECIAL_GAME_TYPE type, const char** urls, size_t urlCount);

/*!
 * \brief Begin playing without a game file
 *
 * If the add-on supports standalone mode, it must add the <supports_standalone>
 * tag to the extension point in addon.xml:
 *
 *     <supports_no_game>false</supports_no_game>
 *
 * \return the error, or GAME_ERROR_NO_ERROR if the game add-on was loaded
 */
GAME_ERROR LoadStandalone(void);

/*!
 * \brief Unload the current game
 *
 * \return the error, or GAME_ERROR_NO_ERROR if the game was unloaded
 */
/*! Unloads a currently loaded game */
GAME_ERROR UnloadGame(void);

/*!
 * \brief Get information about the loaded game
 *
 * \param info The info structure to fill
 *
 * \return the error, or GAME_ERROR_NO_ERROR if info was filled
 */
GAME_ERROR GetGameInfo(game_system_av_info* info);

/*!
 * \brief Get region of the loaded game
 *
 * \return the region, or GAME_REGION_UNKNOWN if unknown or no game is loaded
 */
GAME_REGION GetRegion(void);

/*!
 * \brief Notify the add-on that the frontend is rendering a new frame
 */
void FrameEvent(void);

/*!
 * \brief Reset the current game
 *
 * \return the error, or GAME_ERROR_NO_ERROR if the game was reset
 */
GAME_ERROR Reset(void);

// --- Hardware rendering operations -------------------------------------------

/*!
 * \brief Invalidates the current HW context and reinitializes GPU resources
 *
 * Any GL state is lost, and must not be deinitialized explicitly.
 *
 * \return the error, or GAME_ERROR_NO_ERROR if the HW context was reset
 */
GAME_ERROR HwContextReset(void);

/*!
 * \brief Called before the context is destroyed
 *
 * Resources can be deinitialized at this step.
 *
 * \return the error, or GAME_ERROR_NO_ERROR if the HW context was destroyed
 */
GAME_ERROR HwContextDestroy(void);

// --- Input operations --------------------------------------------------------

/*!
 * \brief Notify the add-on of a status change on an open port
 *
 * Ports can be opened using the OpenPort() callback
 *
 * \param port The port number passed to OpenPort()
 * \param collected True if a controller was connected, false if disconnected
 * \param controller The connected controller
 */
void UpdatePort(unsigned int port, bool connected, const game_controller* controller);

/*!
 * \brief Notify the add-on of an input event
 *
 * \param port The port number passed to OpenPort()
 * \param event The input event
 *
 * \return true if the event was handled, false otherwise
 */
bool InputEvent(unsigned int port, const game_input_event* event);

// --- Disk operations ---------------------------------------------------------

// TODO: Rewrite disk interface

/*!
 * Interface to eject and insert disk images. This is used for games which
 * consist of multiple images and must be manually swapped out by the user
 * (e.g. PSX) during runtime. If the implementation can do this
 * automatically, it should strive to do so. However, there are cases where
 * the user must manually do so.
 *
 * Overview: To swap a disk image, eject the disk image with
 * DiskControlSetEjectState(true). Set the disk index with
 * DiskControlSetImageIndex(index). Insert the disk again with
 * DiskControlSetEjectState(false).
 */

/*!
 * If ejected is true, "ejects" the virtual disk tray. When ejected, the
 * disk image index can be set.
 */
GAME_ERROR DiskSetEjectState(GAME_EJECT_STATE ejected);

/*! Gets current eject state. The initial state is not ejected. */
GAME_EJECT_STATE DiskGetEjectState(void);

/*!
 * Gets current disk index. First disk is index 0. If return value
 * is >= DiskControlGetNumImages(), no disk is currently inserted.
 */
unsigned DiskGetImageIndex(void);

/*!
 * Sets image index. Can only be called when disk is ejected. The
 * implementation supports setting "no disk" (empty tray) by setting index to
 * GAME_NO_DISK.
 */
GAME_ERROR DiskSetImageIndex(unsigned int index);

/*! Gets total number of images which are available to use */
unsigned DiskGetNumImages(void);

/*!
 * Replaces the disk image associated with index. Arguments to pass in info
 * have same requirements as Load(). Virtual disk tray must be
 * ejected when calling this. Replacing a disk image with info = NULL will
 * remove the disk image from the internal list. As a result, calls to
 * DiskControlGetImageIndex() can change.
 *
 * E.g. DiskControlReplaceImageIndex(1, NULL), and previous
 * DiskControlGetImageIndex() returned 4 before. Index 1 will be
 * removed, and the new index is 3.
 */
GAME_ERROR DiskReplaceImageIndex(unsigned int index, const char* url);

/*!
 * Adds a new valid index (DiskControlGetNumImages()) to the internal disk list.
 * This will increment subsequent return values from DiskControlGetNumImages() by
 * 1. This image index cannot be used until a disk image has been set with
 * replace_image_index.
 */
GAME_ERROR DiskAddImageIndex(void);

// --- Camera operations -------------------------------------------------------

/*!
 * \brief Notify the add-on that the camera has been initialized
 */
GAME_ERROR CameraInitialized(void);

/*!
 * \brief Notify the add-on that the camera has been deinitialized
 * */
GAME_ERROR CameraDeinitialized(void);

/*!
 * \brief Transfer raw framebuffer data
 *
 * \param buffer points to an XRGB8888 buffer
 * \param width The frame width
 * \param height The frame height
 * \param stride The number of bytes in one row of pixels
 *
 * \return the error, or GAME_ERROR_NO_ERROR if the data was received
 */
GAME_ERROR CameraFrameRawBuffer(const uint32_t* buffer, unsigned int width, unsigned int height, size_t stride);

/*!
 * \brief Transfer OpenGL texture
 *
 * The state or content of the OpenGL texture should be considered immutable,
 * except for things like texture filtering and clamping.
 *
 * The texture target can include e.g. GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE, and
 * possibly more depending on extensions.
 *
 * The affine matrix is used to apply an affine transform to texture coordinates:
 *
 *     (affine_matrix * vec3(coord_x, coord_y, 1.0))
 *
 * After transform, normalized texture coord (0, 0) should be bottom-left and
 * (1, 1) should be top-right (or (width, height) for RECTANGLE).
 *
 * \param textureId A texture owned by camera driver.
 * \param textureTarget The texture target for the GL texture
 * \param affine Points to a packed 3x3 column-major matrix
 *
 * \return the error, or GAME_ERROR_NO_ERROR if the texture was received
 */
GAME_ERROR CameraFrameOpenglTexture(unsigned int textureId, unsigned int textureTarget, const float* affine);

// --- Serialization operations ------------------------------------------------

/*!
 * \brief Get the number of bytes required to serialize the game
 *
 * \return the number of bytes, or 0 if serialization is not supported
 */
size_t SerializeSize(void);

/*!
 * \brief Serialize the state of the game
 *
 * \param data The buffer receiving the serialized game data
 * \param size The size of the buffer
 *
 * \return the error, or GAME_ERROR_NO_ERROR if the game was serialized into the buffer
 */
GAME_ERROR Serialize(uint8_t* data, size_t size);

/*!
 * \brief Deserialize the game from the given state
 *
 * \param data A buffer containing the game's new state
 * \param size The size of the buffer
 *
 * \return the error, or GAME_ERROR_NO_ERROR if the game deserialized
 */
GAME_ERROR Deserialize(const uint8_t* data, size_t size);

// --- Cheat operations --------------------------------------------------------

/*!
 * \brief Reset the cheat system
 *
 * \return the error, or GAME_ERROR_NO_ERROR if the cheat system was reset
 */
GAME_ERROR CheatReset(void);

/*!
 * \brief Get a region of memory
 *
 * \param type The type of memory to retrieve
 * \param data Set to the region of memory; must remain valid until UnloadGame() is called
 * \param size Set to the size of the region of memory
 *
 * \return the error, or GAME_ERROR_NO_ERROR if data was set to a valid buffer
 */
GAME_ERROR GetMemory(GAME_MEMORY type, const uint8_t** data, size_t* size);

/*!
 * \brief Set a cheat code
 *
 * \param index
 * \param enabled
 * \param code
 *
 * \return the error, or GAME_ERROR_NO_ERROR if the cheat was set
 */
GAME_ERROR SetCheat(unsigned int index, bool enabled, const char* code);

// --- Add-on helper implementation --------------------------------------------

/*!
 * \brief Called by Kodi to assign the function pointers of this add-on to pClient
 *
 * Note that get_addon() is defined here, so it will be available in all
 * compiled game clients.
 */
void __declspec(dllexport) get_addon(GameClient* pClient)
{
  pClient->GetGameAPIVersion        = GetGameAPIVersion;
  pClient->GetMininumGameAPIVersion = GetMininumGameAPIVersion;
  pClient->LoadGame                 = LoadGame;
  pClient->LoadGameSpecial          = LoadGameSpecial;
  pClient->LoadStandalone           = LoadStandalone;
  pClient->UnloadGame               = UnloadGame;
  pClient->GetGameInfo              = GetGameInfo;
  pClient->GetRegion                = GetRegion;
  pClient->FrameEvent               = FrameEvent;
  pClient->Reset                    = Reset;
  pClient->HwContextReset           = HwContextReset;
  pClient->HwContextDestroy         = HwContextDestroy;
  pClient->UpdatePort               = UpdatePort;
  pClient->InputEvent               = InputEvent;
  pClient->DiskSetEjectState        = DiskSetEjectState;
  pClient->DiskGetEjectState        = DiskGetEjectState;
  pClient->DiskGetImageIndex        = DiskGetImageIndex;
  pClient->DiskSetImageIndex        = DiskSetImageIndex;
  pClient->DiskGetNumImages         = DiskGetNumImages;
  pClient->DiskReplaceImageIndex    = DiskReplaceImageIndex;
  pClient->DiskAddImageIndex        = DiskAddImageIndex;
  pClient->CameraInitialized        = CameraInitialized;
  pClient->CameraDeinitialized      = CameraDeinitialized;
  pClient->CameraFrameRawBuffer     = CameraFrameRawBuffer;
  pClient->CameraFrameOpenglTexture = CameraFrameOpenglTexture;
  pClient->SerializeSize            = SerializeSize;
  pClient->Serialize                = Serialize;
  pClient->Deserialize              = Deserialize;
  pClient->CheatReset               = CheatReset;
  pClient->GetMemory                = GetMemory;
  pClient->SetCheat                 = SetCheat;
}

#ifdef __cplusplus
}
#endif

#endif // KODI_GAME_DLL_H_
