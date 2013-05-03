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

#include "GameClient.h"
#include "FileItem.h"
#include "threads/CriticalSection.h"

#include <set>
#include <vector>

/**
 * The main function of CGameManager is resolving file items into CGameClients.
 *
 * A manager is needed for resolving game clients as they are selected by the
 * file extensions they support. This is determined by loading the DLL and
 * querying it directly, so it is desirable to only do this once and cache the
 * information.
 */
class CGameManager
{
private:
  CGameManager() { }

public:
  static CGameManager &Get();

  /**
   * Create and maintain a cache of game client add-on information. If a file
   * has been placed in the queue via QueueFile(), it will be launched if a
   * compatible emulator is registered.
   */
  void RegisterAddons(const ADDON::VECADDONS &addons);
  void RegisterAddon(ADDON::GameClientPtr clientAddon, bool launchQueued = true);
  void UnregisterAddonByID(const CStdString &ID);

  /**
   * Register the supported extensions of remote add-ons for the purpose of
   * IsGame() calls. fromDatabase is only used for logging purposes.
   */
  void RegisterRemoteAddons(const ADDON::VECADDONS &addons, bool fromDatabase = false);

  /**
   * Returns true if the file extension is supported by an add-on in an enabled
   * repository.
   *
   * This function causes the queued file to be reset. The purpose of this is
   * to only invoke the file inside the add-on manager, and IsGame() is called
   * often enough that leaving the add-on manager will eventually reset it.
   */
   bool IsGame(CStdString path);

  /**
   * Queue a file to be launched when the current game client is installed.
   */
  void QueueFile(const CFileItem &file);

  /**
   * Resolve a file item to a list of game client IDs. If the file forces a
   * particular game client via file.SetProperty("gameclient", id), the result
   * will contain no more than one possible candidate. If the file's game info
   * tag provides a "platform", the available game clients will be filtered by
   * this platform (given the <platform> tag in their addon.xml). If file is a
   * zip file, the contents of that zip will be used to find suitable
   * candidates (which may yield multiple if there are several different kinds
   * of ROMs inside).
   */
  void GetGameClientIDs(const CFileItem& file, CStdStringArray &candidates) const;

  void GetExtensions(std::vector<CStdString> &exts);

private:
  /**
   * If we can launch the file with gameClient, ask the user if they would
   * like to do so.
   */
  void LaunchFile(CFileItem file, const CStdString &strGameClient) const;

  void LoadExtensionsFromDB();

  std::vector<ADDON::GameClientConfig> m_gameClients;
  CCriticalSection m_critSection;
  std::set<CStdString> m_gameExtensions;
  CFileItem m_queuedFile;
};
