/*
 *      Copyright (C) 2005-2012 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIWindowGamesBase.h"
#include "filesystem/GameDatabaseDirectory.h"
#include "dialogs/GUIDialogProgress.h"
#include "guilib/GUIWindowManager.h"
#include "settings/Settings.h"
#include "GUIUserMessages.h" // For GUI_MSG_SEARCH
#include "Application.h"
#include "games/tags/GameInfoTag.h"
#include "utils/URIUtils.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"
/*
#include "Util.h"
#include "video/VideoInfoDownloader.h"
#include "utils/RegExp.h"
#include "utils/Variant.h"
#include "addons/AddonManager.h"
#include "addons/GUIDialogAddonInfo.h"
#include "addons/IAddon.h"
#include "video/dialogs/GUIDialogVideoInfo.h"
#include "GUIWindowVideoNav.h"
#include "video/dialogs/GUIDialogVideoScan.h"
#include "dialogs/GUIDialogSmartPlaylistEditor.h"
#include "dialogs/GUIDialogYesNo.h"
#include "playlists/PlayListFactory.h"
#include "NfoFile.h"
#include "PlayListPlayer.h"
#include "GUIPassword.h"
#include "filesystem/ZipManager.h"
#include "filesystem/StackDirectory.h"
#include "filesystem/MultiPathDirectory.h"
#include "video/dialogs/GUIDialogFileStacking.h"
#include "dialogs/GUIDialogMediaSource.h"
#include "windows/GUIWindowFileManager.h"
#include "filesystem/VideoDatabaseDirectory.h"
#include "PartyModeManager.h"
#include "dialogs/GUIDialogOK.h"
#include "dialogs/GUIDialogSelect.h"
#include "guilib/GUIKeyboardFactory.h"
#include "filesystem/Directory.h"
#include "playlists/PlayList.h"
#include "settings/GUISettings.h"
#include "settings/GUIDialogContentSettings.h"
#include "guilib/LocalizeStrings.h"
#include "utils/StringUtils.h"
#include "utils/FileUtils.h"
#include "interfaces/AnnouncementManager.h"
#include "pvr/PVRManager.h"
#include "pvr/recordings/PVRRecordings.h"
#include "addons/Skin.h"
#include "storage/MediaManager.h"
#include "Autorun.h"
#include "URL.h"
*/

using namespace std;
using namespace XFILE;
using namespace GAMEDATABASEDIRECTORY;
//using namespace PLAYLIST;
//using namespace VIDEO;
//using namespace ADDON;
//using namespace PVR;

#define CONTROL_BTNVIEWASICONS     2
#define CONTROL_BTNSORTBY          3
#define CONTROL_BTNSORTASC         4

CGUIWindowGamesBase::CGUIWindowGamesBase(int id, const CStdString &xmlFile)
    : CGUIMediaWindow(id, xmlFile)
{
  //m_thumbLoader.SetObserver(this);
  //m_thumbLoader.SetStreamDetailsObserver(this);
  //m_stackingAvailable = true;
}

bool CGUIWindowGamesBase::OnFileAction(int iItem, int action)
{
  CFileItemPtr item = m_vecItems->Get(iItem);

  // Reset the current start offset. The actual resume
  // option is set in the switch, based on the action passed.
  item->m_lStartOffset = 0;
  
  switch (action)
  {
  case SELECT_ACTION_PLAY:
  default:
    break;
  }
  return OnClick(iItem);
}

bool CGUIWindowGamesBase::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
  case GUI_MSG_WINDOW_DEINIT:
    m_database.Close();
    break;

  case GUI_MSG_WINDOW_INIT:
    m_database.Open();
    break;

  case GUI_MSG_CLICKED:
  case GUI_MSG_SEARCH:
  default:
    break;
  }
  return CGUIMediaWindow::OnMessage(message);
}

bool CGUIWindowGamesBase::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
  CFileItemPtr item;
  if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
    item = m_vecItems->Get(itemNumber);

  switch (button)
  {
  case CONTEXT_BUTTON_PLAY_ITEM:
    OnClick(itemNumber);
    return true;

  case CONTEXT_BUTTON_DELETE:
    OnDeleteItem(itemNumber);
    return true;

  case CONTEXT_BUTTON_RENAME:
    OnRenameItem(itemNumber);
    return true;

  default:
    break;
  }
  return CGUIMediaWindow::OnContextButton(itemNumber, button);
}

void CGUIWindowGamesBase::AddToDatabase(int iItem)
{
  if (iItem < 0 || iItem >= m_vecItems->Size())
    return;

  CFileItemPtr pItem = m_vecItems->Get(iItem);
  if (pItem->IsParentFolder() || pItem->m_bIsFolder)
    return;

  // Query the user for game info (CGUIWindowVideoBase::AddToDatabase(int iItem))
  // Add to database
}

bool CGUIWindowGamesBase::OnSelect(int iItem)
{
  if (iItem < 0 || iItem >= m_vecItems->Size())
    return false;

  CFileItemPtr item = m_vecItems->Get(iItem);

  CStdString path = item->GetPath();
  if (!item->m_bIsFolder && path != "add" && path != "addons://more/games" && path.Left(16) != "newcollection://")
    return OnFileAction(iItem, g_guiSettings.GetInt("myvideos.selectaction"));

  return CGUIMediaWindow::OnSelect(iItem);
}

void CGUIWindowGamesBase::GetContextButtons(int itemNumber, CContextButtons &buttons)
{
  CFileItemPtr item;
  if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
    item = m_vecItems->Get(itemNumber);

  // contextual buttons
  if (item && !item->GetProperty("pluginreplacecontextitems").asBoolean())
  {
    if (!item->IsParentFolder())
    {
      CStdString path(item->GetPath());
      if (item->IsGameDb() && item->HasGameInfoTag())
        path = item->GetGameInfoTag()->GetURL();

      if (!m_vecItems->IsPlugin() && (item->IsPlugin() || item->IsScript()))
        buttons.Add(CONTEXT_BUTTON_INFO, 24003); // Add-on info

      if (!item->m_bIsFolder)
      {
        // get players
        VECPLAYERCORES vecCores;
        if (item->IsGameDb())
        {
          CFileItem item2(item->GetGameInfoTag()->GetURL(), false);
          CPlayerCoreFactory::GetPlayers(item2, vecCores);
        }
        else
          CPlayerCoreFactory::GetPlayers(*item, vecCores);
        if (vecCores.size() > 1)
          buttons.Add(CONTEXT_BUTTON_PLAY_WITH, 15213);
      }
    }
  }
  CGUIMediaWindow::GetContextButtons(itemNumber, buttons);
}

CStdString CGUIWindowGamesBase::GetStartFolder(const CStdString &dir)
{
  if (dir.Equals("Plugins") || dir.Equals("Addons"))
    return "addons://sources/games/";
  return CGUIMediaWindow::GetStartFolder(dir);
}
