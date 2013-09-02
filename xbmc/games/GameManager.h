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

#include "addons/AddonDatabase.h"
#include "addons/AddonManager.h"
#include "GameClient.h"
#include "FileItem.h"
#include "threads/CriticalSection.h"
#include "threads/Thread.h"
#include "utils/Observer.h"

#include <map>
#include <set>
#include <string>

namespace GAMES
{
  /**
   * The main function of CGameManager is resolving file items into CGameClients.
   *
   * A manager is needed for resolving game clients as they are selected by the
   * file extensions they support. This is determined by loading the DLL and
   * querying it directly, so it is desirable to only do this once and cache the
   * information.
   */
  class CGameManager :
    public ADDON::IAddonMgrCallback, public IAddonDatabaseCallback, public Observer, protected CThread
  {
  protected:
    CGameManager();

  public:
    static CGameManager &Get();
    virtual ~CGameManager() { Stop(); }

    virtual void Start();
    virtual void Stop();

    //** Functions to notify CGameManager about stuff it manages

    /**
     * Create and maintain a cache of game client add-on information. If a file
     * has been placed in the queue via QueueFile(), it will be launched if a
     * compatible emulator is registered.
     */
    void RegisterAddons(const ADDON::VECADDONS &addons);
    bool RegisterAddon(const GameClientPtr &clientAddon);
    void UnregisterAddonByID(const std::string &strId);

    /**
     * Register the supported extensions of remote add-ons for use by IsGame().
     * m_gameExtensions is cleared first, then populated with extensions of
     * tracked game clients and the addon vector.
     */
    void RegisterRemoteAddons(const ADDON::VECADDONS &addons);

    // Inherited from IAddonDatabaseCallback
    virtual void AddonEnabled(ADDON::AddonPtr addon, bool bDisabled);
    virtual void AddonDisabled(ADDON::AddonPtr addon);

    // Inherited from Observer
    virtual void Notify(const Observable &obs, const ObservableMessage msg);

    /**
     * Queue a file to be launched when the next game client is installed.
     */
    void QueueFile(const CFileItem &file);
    void UnqueueFile();

    //** Functions to get info for stuff that CGameManager manages

    virtual bool GetClient(const std::string &strClientId, GameClientPtr &addon) const;
    virtual bool GetConnectedClient(const std::string &strClientId, GameClientPtr &addon) const;
    virtual bool IsConnectedClient(const std::string &strClientId) const;
    virtual bool IsConnectedClient(const ADDON::AddonPtr addon) const;

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
    void GetGameClientIDs(const CFileItem& file, std::vector<std::string> &candidates) const;

    void GetExtensions(std::vector<std::string> &exts);

    /**
     * Returns true if the file extension is supported by an add-on in an enabled
     * repository.
     *
     * This function causes the queued file to be reset. The purpose of this is
     * to only invoke the file inside the add-on manager, and IsGame() is called
     * often enough that leaving the add-on manager will eventually reset it.
     * This fuzzy approach should be approximate enough to what the user expects
     * (whether or not the game should be launched).
     */
     bool IsGame(CStdString path);

    //** Functions that operate on the clients

    virtual bool StopClient(ADDON::AddonPtr client, bool bRestart);

    // Inherited from IAddonMgrCallback
    virtual bool RequestRestart(ADDON::AddonPtr addon, bool datachanged) { return StopClient(addon, true); }
    virtual bool RequestRemoval(ADDON::AddonPtr addon)                   { return StopClient(addon, false); }

  protected:
    // Inherited from CThread
    virtual void Process(void);

    /**
     * If we can launch the file with gameClient, ask the user if they would
     * like to do so.
     */
    void LaunchFile(CFileItem file, const CStdString &strGameClient) const;

    void LoadExtensionsFromDB();

    virtual bool UpdateAddons(void) { return true; }
    virtual bool UpdateAndInitialiseClients();

    std::map<std::string, GameClientPtr> m_gameClients; // may not contain empty shared pointers
    std::set<std::string>                m_gameExtensions;
    CFileItemPtr                         m_queuedFile;
    CCriticalSection                     m_critSection;
    CAddonDatabase                       m_addonDb;
  };
} // namespace GAMES
