/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include "utils/StdString.h"

#include <set>

// Forward declarations
class CFileItem;
struct retro_game_info;
namespace ADDON
{
  struct GameClientConfig;
}

namespace GAMES
{
  /**
    * Loading a file in libretro cores is a complicated process. Game clients
    * support different extensions, some support loading from the VFS, and
    * some have the ability to load ROMs from within zips. Game clients have
    * a tendency to lie about their capabilities. Furthermore, different ROMs
    * can have different results, so it is desirable to try different
    * strategies upon failure.
    */
  class CGameFileLoader
  {
  public:
    CGameFileLoader() : m_useVfs(false) { }

    virtual ~CGameFileLoader() { }

    /**
      * Returns true if this strategy is a viable option. CGameFileLoader::strPath
      * is filled with the file that should be loaded, either the original file or
      * a preferred substitute file.
      */
    virtual bool CanLoad(const ADDON::GameClientConfig &gc, const CFileItem& file) = 0;

    /**
      * Populates retro_game_info with results.
      */
    bool GetGameInfo(retro_game_info &info) const;

    /**
      * Perform the gamut of checks on the file: "gameclient" property, platform,
      * extension, and a positive match on at least one of the CGameFileLoader
      * strategies. If config.bAllowVFS and config.bRequireZip are provided, then
      * useStrategies=true can be used to allow more lenient/accurate testing,
      * especially for files inside zips (when .zip isn't supported) and files on
      * the VFS.
      */
    static bool CanOpen(const CFileItem &file, const ADDON::GameClientConfig &config, bool useStrategies = false);

    /**
      * HELPER FUNCTION: If zipPath is a zip file, this will enumerate its contents
      * and return the first file inside with a valid extension. If this returns
      * false, effectivePath will be set to zipPath.
      */
    static bool GetEffectiveRomPath(const CStdString &zipPath, const std::set<CStdString> &validExts, CStdString &effectivePath);

    /**
      * HELPER FUNCTION: If the game client was a bad boy and provided no
      * extensions, this will optimistically return true.
      */
    static bool IsExtensionValid(const CStdString &ext, const std::set<CStdString> &setExts);

  protected:
    // Member variables populated with results from CanLoad()
    CStdString m_path;
    bool       m_useVfs;
  };

  /**
    * Load the file from the local hard disk.
    */
  class CGameFileLoaderUseHD : public CGameFileLoader
  {
  public:
    virtual bool CanLoad(const ADDON::GameClientConfig &gc, const CFileItem& file);
  };

  /**
    * Use the VFS to load the file.
    */
  class CGameFileLoaderUseVFS : public CGameFileLoader
  {
  public:
    virtual bool CanLoad(const ADDON::GameClientConfig &gc, const CFileItem& file);
  };

  /**
    * If the game client blocks extracting, we don't want to load a file from
    * within a zip. In this case, we try to use the container zip (parent
    * folder on the vfs).
    */
  class CGameFileLoaderUseParentZip : public CGameFileLoader
  {
  public:
    virtual bool CanLoad(const ADDON::GameClientConfig &gc, const CFileItem& file);
  };

  /**
    * If a zip fails to load, try loading the ROM inside from the zip:// vfs.
    * Try to avoid recursion clashes with the above strategy.
    */
  class CGameFileLoaderEnterZip : public CGameFileLoader
  {
  public:
    virtual bool CanLoad(const ADDON::GameClientConfig &gc, const CFileItem& file);
  };
} // namespace GAMES
