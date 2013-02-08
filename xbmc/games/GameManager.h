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

#include "GameClient.h"

// Systems that XBMC is currently aware of
enum GameSystemType
{
  SYSTEM_UNKNOWN, // Unknown to XBMC and all addons
  SYSTEM_OTHER,   // Unknown to XBMC, but known by an addon
  SYSTEM_GameBoy,
  SYSTEM_GameBoyColor,
  SYSTEM_GameBoyAdvance,
  SYSTEM_SuperNintendo
};

/*!
 * The main functions of CGameManager are resolving file names into a
 * GameSystemType and a CGameClient.
 *
 * A manager is needed for resolving game clients as they are selected by the
 * file types they support. This is determined by loading the DLL and querying
 * it directly, so it is desirable to only do this once. Lacking file type
 * info, the system type from the <system> tag in addon.xml will be used.
 */
class CGameManager
{
private:
  CGameManager() { }

public:
  static CGameManager &Get();

  /*!
   * Create and maintain a cache of game client add-on information.
   */
  void RegisterAddons(const ADDON::VECADDONS &addons);
  void RegisterAddon(const ADDON::GameClientPtr &clientAddon);
  void UnregisterAddon(const CStdString &ID);

  /*!
   * Resolve a file path to a game client add-on. The extension is used to
   * screen for compatible game clients. If a game client specifies no
   * extensions, but its system matches the system belonging to the file's
   * extension, then it is added as a possible game client.
   *
   * Currently, this returns only the first match. TODO: hand off an array
   */
  ADDON::GameClientPtr GetGameClient(const CStdString &strFile);

  /*!
   * Resolve a file name to a system type enum by the file's extension.
   * @return SYSTEM_UNKNOWN if no system was found, SYSTEM_OTHER if the
   * extension isn't recognized but a game client still supports it.
   */
  GameSystemType GetSystemType(const CStdString &strFile);
  
  /*!
   * Return the number of controller ports the specified system supports.
   * Currently this is determined by an internal lookup table. In the future,
   * this should be accomplished by querying the game client DLL directly.
   * @return 0 if unknown
   */
  static unsigned int GetPlayers(GameSystemType system);

private:
  /*!
   * Returns true if strExt is in strExtensionList.
   * @param strExtensionList Like "gba|agb|elf|mb"
   * @param strExt           Like "gba", may contain a period or upper case
   */
  static bool ContainsExtesion(const CStdString &strExtensionList, const CStdString &strExt);

private:
  // Pertinent information captured by the game client add-on. We use these
  // objects as a cache to avoid loading the DLL every time.
  struct GameClientObject
  {
    CStdString id;
    CStdString extensions;
    std::vector<GameSystemType> systems;
  };

  std::vector<GameClientObject> m_gameClients;

};
