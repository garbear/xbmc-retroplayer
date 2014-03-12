/*
 *      Copyright (C) 2012-2014 Team XBMC
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

/*
 * Adding new functions and callbacks to the XBMC Game API
 *
 * The Game API is spread out across various files. Adding new functions and
 * callbacks is an adventure that spans many layers of abstraction. Necessary
 * steps can easily be omitted, so the process of adding functions and
 * callbacks is documented here.
 *
 * The Game API is layed out in three files:
 *   - xbmc_game_dll.h        (API function declarations)
 *   - xbmc_game_callbacks.h  (API callback pointers)
 *   - xbmc_game_types.h      (API enums and structs)
 *
 * To add a new API function:
 *   1.  Declare the function in xbmc_game_dll.h with some helpful documentation
 *   2.  Assign the function pointer in get_addon() of the same file. get_addon()
 *       (aliased to GetAddon()) is called in AddonDll.h immediately after
 *       loading the shared library.
 *   3.  Add the function to the GameClient struct in xbmc_game_types.h. This
 *       struct contains pointers to all the API functions. It is populated in
 *       get_addon(). CGameClient invokes API functions through this struct.
 *   4.  Define the function in the cpp file of the game client project
 *
 * To add a new API callback:
 *   1.  Declare the callback as a function pointer in the CB_GameLib struct of
 *       xbmc_game_callbacks.h with some helpful documentation. The first
 *       parameter, addonData, is the CAddonCallbacksGame object associated with
 *       the game client instance.
 *   2.  Declare the callback as a static member function of CAddonCallbacksGame
 *   3.  Define the function in AddonCallbacksGame.cpp
 *   4.  Expose the function to the game client in libXBMC_game.cpp. This shared
 *       library allows for ABI compatibility if the API is unchanged across
 *       releases.
 *   5.  Add the callback to the helper class in libXBMC_game.h. Requires three
 *       modifications: register the symbol exported from the shared library,
 *       expose the callback using a member function wrapper, and declare the
 *       function pointer as a protected member variable.
 */

#include "addons/Addon.h"
#include "addons/AddonDll.h"
#include "addons/DllGameClient.h"
#include "addons/include/xbmc_addon_types.h"

#include "GameFileLoader.h"
//#include "games/tags/GameInfoTagLoader.h"
//#include "SerialState.h"
//#include "threads/CriticalSection.h"

#include <boost/shared_ptr.hpp>
//#include <set>
#include <string>
#include <vector>

#define GAMECLIENT_MAX_PLAYERS  8

class CFileItem;

namespace GAME
{
  class CGameClient;
  typedef boost::shared_ptr<CGameClient> GameClientPtr;
  typedef std::vector<CGameClient>       GameClientVector;

  class CGameClient : public ADDON::CAddonDll<DllGameClient, GameClient, game_client_properties>
  {
  public:
    CGameClient(const ADDON::AddonProps &props);
    CGameClient(const cp_extension_t *props);
    virtual ~CGameClient(void);

    /*!
     * @brief Initialise the instance of this add-on
     */
    ADDON_STATUS Create();

    /*!
     * @brief Destroy the instance of this add-on
     */
    void Destroy(void);

    /*!
     * @return True if this instance is initialised, false otherwise.
     */
    bool ReadyToUse(void) const { return m_bReadyToUse; }

    const std::string&                   GetClientName() const { return m_strClientName; }
    const std::string&                   GetClientVersion() const { return m_strClientVersion; }
    const std::set<std::string>&         GetExtensions() const { return m_extensions; }
    bool                                 SupportsVFS() const { return m_bSupportsVFS; }
    const GAME_INFO::GamePlatformVector& GetPlatforms() const { return m_platforms; }
    const std::string&                   GetFilePath() const { return m_gameFile.Path(); }
    /**
     * Find the region of a currently running game. The return value will be
     * RETRO_REGION_NTSC, RETRO_REGION_PAL or -1 for invalid.
     */
    int                                GetRegion() const { return m_region; }

    bool OpenFile(const CFileItem &file);
    void CloseFile();

    /**
     * Perform the gamut of checks on the file: "gameclient" property, platform,
     * extension, and a positive match on at least one of the CGameFileLoader
     * strategies.
     */
    bool CanOpen(const CFileItem &file) const;

  private:
    /*!
     * @brief Resets all class members to their defaults. Called by the constructors.
     */
    void ResetProperties();
    
    bool GetAddonProperties(void);

    bool LogError(const GAME_ERROR error, const char *strMethod) const;
    void LogException(const char *strFunctionName) const;

    ADDON::AddonVersion    m_apiVersion;
    bool                   m_bReadyToUse;          /*!< true if this add-on is connected to the backend, false otherwise */

    // Returned from DLL
    std::string                   m_strClientName;
    std::string                   m_strClientVersion;
    bool                          m_bSupportsVFS;
    double                        m_frameRate; // Video framerate
    double                        m_frameRateCorrection; // Framerate correction factor (to sync to audio)
    double                        m_sampleRate; // Audio frequency
    int                           m_region; // Region of the loaded game
    std::set<std::string>         m_extensions;
    GAME_INFO::GamePlatformVector m_platforms;

    bool                          m_bIsPlaying; // This is true between OpenFile() and CloseFile()
    CGameFile                     m_gameFile; // the current playing file

    CCriticalSection              m_critSection;
    unsigned int                  m_serialSize;
    bool                          m_bRewindEnabled;
    CSerialState                  m_serialState;

    // If rewinding is disabled, use a buffer to avoid re-allocation when saving games
    std::vector<uint8_t>         m_savestateBuffer;
    /**
     * Settings and strings are handled slightly differently with game client
     * because they all share the possibility of having a system directory.
     * Trivial saves are skipped to avoid unnecessary file creations, and
     * strings simply use g_localizeStrings.
     * \sa LoadSettings
     */
    virtual void SaveSettings();
    virtual CStdString GetString(uint32_t id);



    /**
     * Each port (or player, if you will) must be associated with a device. The
     * default device is RETRO_DEVICE_JOYPAD. For a list of valid devices, see
     * libretro.h.
     *
     * Do not exceed the number of devices that the game client supports. A
     * quick analysis of SNES9x Next v2 showed that a third port will overflow
     * a buffer. Currently, there is no way to determine the number of ports a
     * client will support, so stick with 1.
     *
     * Precondition: OpenFile() must return true.
     */
    void SetDevice(unsigned int port, unsigned int device);

    /**
     * Allow the game to run and produce a video frame.
     * Precondition: OpenFile() returned true.
     * Returns false if an exception is thrown in retro_run().
     */
    bool RunFrame();

    /**
     * Rewind gameplay 'frames' frames.
     * As there is a fixed size buffer backing
     * save state deltas, it might not be possible to rewind as many
     * frames as desired. The function returns number of frames actually rewound.
     */
    unsigned int RewindFrames(unsigned int frames);

    // Returns how many frames it is possible to rewind with a call to RewindFrames().
    size_t GetAvailableFrames() const { return m_bRewindEnabled ? m_serialState.GetFramesAvailable() : 0; }

    // Returns the maximum amount of frames that can ever be rewound.
    size_t GetMaxFrames() const { return m_bRewindEnabled ? m_serialState.GetMaxFrames() : 0; }

    // Reset the game, if running.
    void Reset();

    // Video framerate is used to calculate savestate wall time
    double GetFrameRate() const { return m_frameRate * m_frameRateCorrection; }
    void SetFrameRateCorrection(double correctionFactor);
    double GetSampleRate() const { return m_sampleRate; }

    /**
     * If the game client was a bad boy and provided no extensions, this will
     * optimistically return true.
     */
    bool IsExtensionValid(const std::string &ext) const;

  protected:
    CGameClient(const CGameClient &other);
    virtual bool LoadSettings(bool bForce = false);

  private:
    void Initialize();

    /**
     * Perform the actual loading of the game by the DLL. The resulting CGameFile
     * is placed in m_gameFile.
     */
    bool OpenInternal(const CFileItem& file);

    /**
     * Calls retro_get_system_av_info() and prints the game/environment info on
     * the screen. The framerate and samplerate are stored in m_frameRate and
     * m_sampleRate.
     */
    bool LoadGameInfo();

    /**
     * Initialize the game client serialization subsystem. If successful,
     * m_bRewindEnabled and m_serialSize are set appropriately.
     */
    void InitSerialization();

    /**
     * Given the strategies above, order them in the way that respects
     * CSettings::Get().GetBool("gamesdebug.prefervfs").
     */
    static void GetStrategy(CGameFileLoaderUseHD &hd, CGameFileLoaderUseParentZip &outerzip,
        CGameFileLoaderUseVFS &vfs, CGameFileLoaderEnterZip &innerzip, CGameFileLoader *strategies[4]);

    /**
     * Parse a pipe-separated list, returned from the game client, into an
     * array. The extensions list can contain both upper and lower case
     * extensions; only lower-case extensions are stored in m_validExtensions.
     */
    void SetExtensions(const std::string &strExtensionList);
    void SetPlatforms(const std::string &strPlatformList);

  };
}
