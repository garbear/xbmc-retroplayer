/*
 *      Copyright (C) 2012 Garrett Brown
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#pragma once

#include "addons/Addon.h"
#include "FileItem.h"
#include "GameClientDLL.h"
//#include "GameManager.h"

#define GAMECLIENT_MAX_PLAYERS  8

namespace ADDON
{
  class CGameClient;
  typedef boost::shared_ptr<CGameClient> GameClientPtr;
 
  class CGameClient : public CAddon
  {
  public:
    /**
     * Callback container. Data is passed in and out of the game client through
     * these callbacks.
     */
    struct DataReceiver
    {
      typedef void    (*VideoFrame_t)      (const void *data, unsigned width, unsigned height, size_t pitch);
      typedef void    (*AudioSample_t)     (int16_t left, int16_t right);
      typedef size_t  (*AudioSampleBatch_t)(const int16_t *data, size_t frames);
      // Actually a "data sender", but who's looking
      typedef int16_t (*GetInputState_t)   (unsigned port, unsigned device, unsigned index, unsigned id);
      typedef void    (*SetPixelFormat_t)  (retro_pixel_format format); // retro_pixel_format defined in libretro.h

      VideoFrame_t       VideoFrame;
      AudioSample_t      AudioSample;
      AudioSampleBatch_t AudioSampleBatch;
      GetInputState_t    GetInputState;
      SetPixelFormat_t   SetPixelFormat;

      DataReceiver(VideoFrame_t vf, AudioSample_t as, AudioSampleBatch_t asb, GetInputState_t is, SetPixelFormat_t spf)
        : VideoFrame(vf), AudioSample(as), AudioSampleBatch(asb), GetInputState(is), SetPixelFormat(spf) { }
    };

    CGameClient(const AddonProps &props);
    CGameClient(const cp_extension_t *props);
    virtual ~CGameClient() { DeInit(); }

    /**
     * Load the DLL and query basic parameters. After Init() is called, the
     * Get*() and CanOpen() functions may be called.
     */
    bool Init();

    /**
     * Cleanly shut down and unload the DLL.
     */
    void DeInit();

    /**
     * Returns true after Init() is called and until DeInit() is called.
     */
    bool IsInitialized() const { return m_dll.IsLoaded(); }

    /**
     * Precondition: Init() must be called first and return true.
     */
    const CStdString &GetClientName() const { return m_clientName; }

    /**
     * Precondition: Init() must be called first and return true.
     */
    const CStdString &GetClientVersion() const { return m_clientVersion; }

    const CStdStringArray &GetSystems() const { return m_systems; }

    /**
     * Returns the suggested extensions, as provided by the DLL.
     * Precondition: Init() must be called first and return true.
     * \return A string delimited by pipes i.e. "bin|rom|iso". This string can
     *         be empty if the client DLL hasn't implemented it.
     */
    const CStdString &GetExtensions() const { return m_validExtensions; }

    /**
     * Returns true if the file can be loaded by the client. If the file is on
     * the VFS, the client must support loading by memory. If the client can't
     * load files from memory, then the file path must be passed to the client;
     * thus, only local files can be used with these clients. If the client
     * specifies extensions, the file will also be checked against them.
     * Precondition: Init() must be called first and return true.
     */
    bool CanOpen(const CStdString &filePath, bool checkExtension = true) const;

    bool OpenFile(const CFileItem &file, const DataReceiver &callbacks);
    void CloseFile();

    /**
     * Find the region of a currently running game. The return value will be
     * RETRO_REGION_NTSC, RETRO_REGION_PAL or -1 for invalid.
     */
    int GetRegion() { return m_region; }

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
     * Allow the game to run and produce a video frame. Precondition:
     * OpenFile() returned true.
     */
    void RunFrame();

    /**
     * Reset the game, if running.
     */
    void Reset();

    double GetFrameRate() const { return m_frameRate; }
    double GetSampleRate() const { return m_sampleRate; }

  private:
    void Initialize();

    static bool EnvironmentCallback(unsigned cmd, void *data);
    static DataReceiver::SetPixelFormat_t SetPixelFormat; // called by EnvironmentCallback()

    GameClientDLL m_dll;
    CStdStringArray m_systems;

    bool       m_bIsInited; // Keep track of whether m_dll.retro_init() has been called
    bool       m_bIsPlaying; // This is true between retro_load_game() and retro_unload_game()
    CStdString m_clientName;
    CStdString m_clientVersion;
    CStdString m_validExtensions; // Remember, pipe separated, i.e. "bin|rom|iso"
    bool       m_bAllowVFS; // Allow files with no local path
    bool       m_bRequireZip; // Don't use VFS for zip files, pass zip path directly
    double     m_frameRate; // Video framerate
    double     m_sampleRate; // Audio frequency
    int        m_region; // Region of the loaded game

    /**
     * This callback exists to give XBMC a chance to poll for input. XBMC already
     * takes care of this, so the callback isn't needed.
     */
    static void NoopPoop() { }
  };
}
