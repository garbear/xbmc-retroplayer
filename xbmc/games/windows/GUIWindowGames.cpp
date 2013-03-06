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

#include "GUIWindowGames.h"
#include "addons/GUIDialogAddonInfo.h"
#include "dialogs/GUIDialogProgress.h"
#include "FileItem.h"
#include "guilib/GUIWindowManager.h"
#include "GUIPassword.h"
#include "settings/GUISettings.h"
#include "Util.h"

#define CONTROL_BTNVIEWASICONS      2
#define CONTROL_BTNSORTBY           3
#define CONTROL_BTNSORTASC          4
//#define CONTROL_LABELFILES         12

using namespace XFILE;

CGUIWindowGames::CGUIWindowGames() : CGUIMediaWindow(WINDOW_GAMES, "MyGames.xml")
{
}

CGUIWindowGames::~CGUIWindowGames()
{
}

void CGUIWindowGames::OnInitWindow()
{
  CGUIMediaWindow::OnInitWindow();
}

bool CGUIWindowGames::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
  case GUI_MSG_WINDOW_INIT:
    {
      m_rootDir.AllowNonLocalSources(false);

      // Is this the first time the window is opened?
      if (m_vecItems->GetPath() == "?" && message.GetStringParam().empty())
        m_vecItems->SetPath("");

      m_dlgProgress = dynamic_cast<CGUIDialogProgress*>(g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS));

      return CGUIMediaWindow::OnMessage(message);
    }
    break;

  case GUI_MSG_WINDOW_DEINIT:
    {
      return CGUIMediaWindow::OnMessage(message);
    }
    break;

  case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();
      if (m_viewControl.HasControl(iControl))  // list/thumb control
      {
        int iItem = m_viewControl.GetSelectedItem();
        int iAction = message.GetParam1();

        CFileItemPtr pItem = m_vecItems->Get(iItem);

        if (iAction == ACTION_DELETE_ITEM)
        {
          // is delete allowed?
          if (g_guiSettings.GetBool("filelists.allowfiledeletion"))
            OnDeleteItem(iItem);
          else
            return false;
        }
        else if (iAction == ACTION_PLAYER_PLAY)
        {
          return OnClick(iItem);
        }
        else if (iAction == ACTION_SHOW_INFO)
        {
          if (pItem)
          {
            if (!m_vecItems->IsPlugin() && (pItem->IsPlugin() || pItem->IsScript()))
            {
              CGUIDialogAddonInfo::ShowForItem(pItem);
              return true;
            }
          }
          return true;
        }
      }
    }
    break;
  }
  return CGUIMediaWindow::OnMessage(message);
}

void CGUIWindowGames::GetContextButtons(int itemNumber, CContextButtons &buttons)
{
  CFileItemPtr item = m_vecItems->Get(itemNumber);

  if (item && !item->GetProperty("pluginreplacecontextitems").asBoolean())
  {
    if (m_vecItems->IsVirtualDirectoryRoot())
    {
      buttons.Add(CONTEXT_BUTTON_SWITCH_MEDIA, 523); // Switch media
    }
    else if (m_vecItems->IsSourcesPath())
    {
      CGUIDialogContextMenu::GetContextButtons("games", item, buttons);
    }
    else
    {
      buttons.Add(CONTEXT_BUTTON_GOTO_ROOT, 20128);
      buttons.Add(CONTEXT_BUTTON_SWITCH_MEDIA, 523);

      if (item)
      {
        if (!m_vecItems->IsPlugin() && (item->IsPlugin() || item->IsScript()))
          buttons.Add(CONTEXT_BUTTON_INFO, 24003); // Add-on info
        /*
        if (!(item->m_bIsFolder || item->IsZIP() || item->IsRAR() || item->IsCBZ() || item->IsCBR() || item->IsScript()))
          buttons.Add(CONTEXT_BUTTON_INFO, 13406); // picture info
        */
        if (g_guiSettings.GetBool("filelists.allowfiledeletion") && !item->IsReadOnly())
        {
          buttons.Add(CONTEXT_BUTTON_DELETE, 117);
          buttons.Add(CONTEXT_BUTTON_RENAME, 118);
        }
        if (item->IsPlugin() || item->IsScript() || m_vecItems->IsPlugin())
          buttons.Add(CONTEXT_BUTTON_PLUGIN_SETTINGS, 1045);
      }
    }
  }
  CGUIMediaWindow::GetContextButtons(itemNumber, buttons);
  if (item && !item->GetProperty("pluginreplacecontextitems").asBoolean())
    buttons.Add(CONTEXT_BUTTON_SETTINGS, 5);                  // Settings
}

bool CGUIWindowGames::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
  CFileItemPtr item = m_vecItems->Get(itemNumber);
  if (item && m_vecItems->IsSourcesPath())
  {
    if (CGUIDialogContextMenu::OnContextButton("games", item, button))
    {
      Update("sources://games/");
      return true;
    }
  }
  switch (button)
  {
  case CONTEXT_BUTTON_INFO:
    OnInfo(itemNumber);
    return true;
  case CONTEXT_BUTTON_DELETE:
    OnDeleteItem(itemNumber);
    return true;
  case CONTEXT_BUTTON_RENAME:
    OnRenameItem(itemNumber);
    return true;
  case CONTEXT_BUTTON_SETTINGS:
    g_windowManager.ActivateWindow(WINDOW_SETTINGS_MYGAMES); // TODO
    return true;
  case CONTEXT_BUTTON_GOTO_ROOT:
    Update("");
    return true;
  case CONTEXT_BUTTON_SWITCH_MEDIA:
    CGUIDialogContextMenu::SwitchMedia("games", m_vecItems->GetPath());
    return true;
  default:
    break;
  }
  return CGUIMediaWindow::OnContextButton(itemNumber, button);
}

void CGUIWindowGames::OnInfo(int itemNumber)
{
  CFileItemPtr item = m_vecItems->Get(itemNumber);
  if (!item)
    return;

  if (!m_vecItems->IsPlugin() && (item->IsPlugin() || item->IsScript()))
  {
    CGUIDialogAddonInfo::ShowForItem(item);
    return;
  }
  /*
  if (item->m_bIsFolder || item->IsZIP() || item->IsRAR() || item->IsCBZ() || item->IsCBR() || !item->IsPicture())
    return;
  CGUIDialogGameInfo *pictureInfo = dynamic_cast<CGUIDialogGameInfo*>(g_windowManager.GetWindow(WINDOW_DIALOG_GAME_INFO));
  if (pictureInfo)
  {
    pictureInfo->SetPicture(item.get());
    pictureInfo->DoModal();
  }
  */
}

CStdString CGUIWindowGames::GetStartFolder(const CStdString &dir)
{
  if (dir.Equals("Plugins") || dir.Equals("Addons"))
    return "addons://sources/game/";

  SetupShares();
  VECSOURCES shares;
  m_rootDir.GetSources(shares);
  bool bIsSourceName = false;
  int iIndex = CUtil::GetMatchingSource(dir, shares, bIsSourceName);
  if (iIndex >= 0)
  {
    if (iIndex < (int)shares.size() && shares[iIndex].m_iHasLock == 2)
    {
      CFileItem item(shares[iIndex]);
      if (!g_passwordManager.IsItemUnlocked(&item, "games"))
        return "";
    }
    if (bIsSourceName)
      return shares[iIndex].strPath;
    return dir;
  }
  return CGUIMediaWindow::GetStartFolder(dir);
}
