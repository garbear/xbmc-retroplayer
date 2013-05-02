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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIDialogGameSaves.h"
#include "Application.h"
#include "cores/RetroPlayer/RetroPlayer.h"
#include "dialogs/GUIDialogContextMenu.h"
#include "dialogs/GUIDialogOK.h"
#include "dialogs/GUIDialogYesNo.h"
#include "filesystem/File.h"
#include "guilib/GUIKeyboardFactory.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "SavestateDatabase.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "view/ViewState.h"

#define CONTROL_ADD_SAVESTATE          2
#define CONTROL_CLEAR_SAVESTATES       3

#define CONTROL_THUMBS                11

using namespace XFILE;

CGUIDialogGameSaves::CGUIDialogGameSaves()
    : CGUIDialog(WINDOW_DIALOG_GAME_SAVES, "VideoOSDGameSaves.xml"), playSpeed(0)
{
  m_vecItems = new CFileItemList;
  m_loadType = KEEP_IN_MEMORY;
}

CGUIDialogGameSaves::~CGUIDialogGameSaves()
{
  delete m_vecItems;
}

bool CGUIDialogGameSaves::OnMessage(CGUIMessage& message)
{
  // TODO: Listen for the following announcements:
  // On game play - enable "create save state" button if applicable
  // On game stop - disable "create save state" button

  switch (message.GetMessage())
  {
  case GUI_MSG_WINDOW_INIT:
    {
      CGUIWindow::OnMessage(message);

      if (g_application.IsPlayingGame())
      {
        bool wasFullscreen = (g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO);
        if (wasFullscreen)
        {
          // TODO: Invalidate playSpeed if GetPlaySpeed() changes while dialog is open
          playSpeed = !g_application.IsPaused() ? g_application.GetPlaySpeed() : 0;
          if (playSpeed)
            g_application.m_pPlayer->Pause();
        }
        
        // Message parameter (if any) is game path. If this matches the current
        // file, use current file's game client ID as the mask. RetroPlayer may
        // have entered or exited a zip, so match against both g_application's
        // and RetroPlayer's file path.
        boost::shared_ptr<CRetroPlayer> rp = boost::dynamic_pointer_cast<CRetroPlayer>(g_application.m_pPlayer);
        if (message.GetNumStringParams())
        {
          CStdString pathParam(message.GetStringParam());
          if (pathParam.Equals(g_application.CurrentFile()) || (rp && pathParam.Equals(rp->GetFilePath())))
          {
            m_gamePath = rp->GetFilePath();
            m_gameClient = rp->GetGameClient();
          }
          else
          {
            m_gamePath = pathParam;
            m_gameClient.clear();
          }
        }
        else if (rp)
        {
          m_gamePath = rp->GetFilePath();
          m_gameClient = rp->GetGameClient();
        }
      }
      else
      {
        // No game is playing, dialog was probably opened from the context menu
        // in MyGames, path is the parameter set by ActivateWindow()
        m_gamePath = message.GetStringParam();
        m_gameClient.clear();
      }

      // Notice, a game client ID was only stored if the dialog was opened for
      // the game currently being played. We want the availability of our
      // "Create save state" button to mirror this fact.
      if (!m_gameClient.empty())
      {
        // Always focus on "Create save state" button by default
        CGUIMessage msg_enable(GUI_MSG_ENABLED, GetID(), CONTROL_ADD_SAVESTATE);
        OnMessage(msg_enable);
        CGUIMessage msg_focus(GUI_MSG_SETFOCUS, GetID(), CONTROL_ADD_SAVESTATE);
        OnMessage(msg_focus);
      }
      else
      {
        CGUIMessage msg_disable(GUI_MSG_DISABLED, GetID(), CONTROL_ADD_SAVESTATE);
        OnMessage(msg_disable);
      }

      Update();
    }
    return true;

  case GUI_MSG_WINDOW_DEINIT:
    {
      m_viewControl.Clear();
      m_vecItems->Clear();
      ResumePlaying();
      m_gamePath.clear();
    }
    break;

  case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();
      if (iControl == CONTROL_ADD_SAVESTATE)
      {
        CreateSaveState();
      }
      else if (iControl == CONTROL_CLEAR_SAVESTATES)
      {
        if (m_vecItems->Size())
          ClearSaveStates();
      }
      else if (m_viewControl.HasControl(iControl))  // list/thumb control
      {
        int iItem = m_viewControl.GetSelectedItem();
        int iAction = message.GetParam1();
        if (0 <= iItem && iItem < m_vecItems->Size())
        {
          switch (iAction)
          {
          case ACTION_CONTEXT_MENU:
            OnContextMenu((*m_vecItems)[iItem]);
            break;
          case ACTION_DELETE_ITEM:
            DeleteSaveState((*m_vecItems)[iItem]->GetPath());
            break;
          case ACTION_SELECT_ITEM:
          case ACTION_MOUSE_LEFT_CLICK:
            LoadSaveState((*m_vecItems)[iItem]);
            break;
          }

          // Don't use IsDialogRunning(), still returns true if closing
          if (IsActive())
          {
            // Restore selected item position
            if (iItem && iItem >= m_vecItems->Size())
              iItem--;
            m_viewControl.SetSelectedItem(iItem);
          }
        }
      }
    }
    break;

  case GUI_MSG_SETFOCUS:
    {
      if (m_viewControl.HasControl(message.GetControlId()) &&
        m_viewControl.GetCurrentControl() != message.GetControlId())
      {
        m_viewControl.SetFocused();
        return true;
      }
    }
    break;
  }

  if (m_closing || !IsActive())
    ResumePlaying();

  return CGUIDialog::OnMessage(message);
}

bool CGUIDialogGameSaves::OnAction(const CAction &action)
{
  if (g_application.IsPlayingGame())
  {
    // Intercept SAVE and LOAD keys so that we can keep our window up to date
    if (ACTION_SAVE <= action.GetID() && action.GetID() <= ACTION_SAVE9)
    {
      if (g_application.m_pPlayer->OnAction(action.GetID()))
        Update();
      return true;
    }
    if (ACTION_LOAD <= action.GetID() && action.GetID() <= ACTION_LOAD9)
    {
      if (g_application.m_pPlayer->OnAction(action.GetID()))
        Close();
      return true;
    }
  }
  return CGUIDialog::OnAction(action);
}

bool CGUIDialogGameSaves::OnContextMenu(const CFileItemPtr &savefile)
{
  // Save file is an auto-save if there is no underscore in the name (e.g. feba62c2.sav)
  //bool isAuto = (URIUtils::GetFileName(savefile->GetPath()).Find("_") < 0);

  CContextButtons choices;
  choices.Add(0, 15044); // Load save state
  choices.Add(1, 118); // Rename
  choices.Add(2, 117); // Delete

  int btnid = CGUIDialogContextMenu::ShowAndGetChoice(choices);

  if (btnid < 0 || btnid > 2)
    return false;

  if (btnid == 0)
    LoadSaveState(savefile);
  else if (btnid == 1)
    RenameSaveState(savefile);
  else if (btnid == 2)
    DeleteSaveState(savefile->GetPath());
  return true;
}

void CGUIDialogGameSaves::Update()
{
  m_vecItems->Clear();

  if (!m_gamePath.empty())
  {
    CSavestateDatabase db;
    db.Open();

    // For now, always refresh IDs, because adding objects can shuffle these around (DynamicDatabase TODO)
    int dbGamePathId;
    int dbGameClientId;
    if (db.GetItemID("gamepath", m_gamePath, dbGamePathId))
    {
      std::map<std::string, long> predicates;
      predicates["gamepath"] = dbGamePathId;

      if (!m_gameClient.empty())
      {
        // Invalidate results if game client doesn't exist in database
        if (db.GetItemID("gameclient", m_gameClient, dbGameClientId))
          predicates["gameclient"] = dbGameClientId;
        else
          predicates["gameclient"] = -1;
      }

      db.GetObjectsNav(*m_vecItems, predicates);
      m_vecItems->Sort(SORT_METHOD_DATE, SortOrderDescending);
    }
  }

  // Lock our display, as this window is rendered from the player thread
  g_graphicsContext.Lock();
  m_viewControl.SetCurrentView(DEFAULT_VIEW_ICONS);

  m_viewControl.Clear();
  m_viewControl.SetItems(*m_vecItems);

  // If a disabled button has focus, focus on something else
  CGUIControl *focusedControl;
  if (GetFocusedControlID() == 0 || !(focusedControl = GetFocusedControl()) || focusedControl->IsDisabled())
  {
    CGUIMessage msg_focus(GUI_MSG_SETFOCUS, GetID(), CONTROL_THUMBS);
    OnMessage(msg_focus);
  }
  
  g_graphicsContext.Unlock();
}

void CGUIDialogGameSaves::CreateSaveState()
{
  CStdString label(CDateTime::GetCurrentDateTime().GetAsLocalizedDateTime());
  if (CGUIKeyboardFactory::ShowAndGetInput(label, g_localizeStrings.Get(528), true)) // Enter title
  {
    boost::shared_ptr<CRetroPlayer> rp = boost::dynamic_pointer_cast<CRetroPlayer>(g_application.m_pPlayer);
    if (rp && rp->Save(label))
      Update(); // If Save() fails, no changes were committed to the database
  }
}

void CGUIDialogGameSaves::LoadSaveState(const CFileItemPtr &savefile)
{
  // If RetroPlayer is already playing a game, we can avoid doing the heavy
  // lifting through CApplication::PlayFile()
  boost::shared_ptr<CRetroPlayer> rp = boost::dynamic_pointer_cast<CRetroPlayer>(g_application.m_pPlayer);
  if (g_application.IsPlayingGame() && rp)
  {
    // If game path and game client match, just load the save state
    if (m_gamePath.Equals(rp->GetFilePath()) &&
        m_gameClient.Equals(rp->GetGameClient()))
    {
      if (rp->Load(savefile->GetPath()))
        Close();
      return;
    }
  }

  CFileItem game(m_gamePath, false);
  if (!m_gameClient.empty())
    game.SetProperty("gameclient", m_gameClient);
  else
    game.SetProperty("gameclient", savefile->GetProperty("gameclient"));
  game.m_startSaveState = savefile->GetPath();

  // Don't want to modify playspeed after starting new game
  playSpeed = 0;
  Close();
  g_application.PlayFile(game);
}

void CGUIDialogGameSaves::RenameSaveState(const CFileItemPtr &saveFile)
{
  CStdString label(saveFile->GetLabel());
  if (CGUIKeyboardFactory::ShowAndGetInput(label, g_localizeStrings.Get(528), true)) // Enter title
  {
    CSavestateDatabase db;
    if (db.Open() && db.RenameSaveState(saveFile->GetPath(), label))
      Update();
  }   
}

void CGUIDialogGameSaves::DeleteSaveState(const CStdString &saveStatePath)
{
  CGUIDialogYesNo* pDialog = dynamic_cast<CGUIDialogYesNo*>(g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO));
  if (pDialog)
  {
    pDialog->SetHeading(117); // Delete
    pDialog->SetLine(0, 122); // Confirm file delete?
    pDialog->SetLine(1, "");
    pDialog->SetLine(2, "");
    pDialog->DoModal();
    if (pDialog->IsConfirmed())
    {
      CSavestateDatabase db;

      if (!db.Open() || !db.DeleteSaveState(saveStatePath))
      {
        CGUIDialogOK* pDialogOK = dynamic_cast<CGUIDialogOK*>(g_windowManager.GetWindow(WINDOW_DIALOG_OK));
        if (pDialogOK)
        {
          pDialogOK->SetHeading(117); // Delete
          pDialogOK->SetLine(0, 16205); // Delete failed
          pDialogOK->SetLine(1, "");
          pDialogOK->SetLine(2, "");
          pDialogOK->DoModal();
        }
      }
      Update();
    }
  }
}

void CGUIDialogGameSaves::ClearSaveStates()
{
  CGUIDialogYesNo* pDialog = dynamic_cast<CGUIDialogYesNo*>(g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO));
  if (pDialog)
  {
    pDialog->SetHeading(122); // Confirm file delete?
    pDialog->SetLine(0, 125); // Delete these files? - Deleting files cannot be undone!
    pDialog->SetLine(1, "");
    pDialog->SetLine(2, "");
    pDialog->DoModal();
    if (pDialog->IsConfirmed())
    {
      CSavestateDatabase db;
      bool success = db.Open();
      if (success)
      {
        for (int i = 0; i < m_vecItems->Size(); i++)
          if (!db.DeleteSaveState((*m_vecItems)[i]->GetPath()))
            success = false;
      }

      if (!success)
      {
        CGUIDialogOK* pDialogOK = dynamic_cast<CGUIDialogOK*>(g_windowManager.GetWindow(WINDOW_DIALOG_OK));
        if (pDialogOK)
        {
          pDialogOK->SetHeading(117); // Delete
          pDialogOK->SetLine(0, 16206); // Failed to delete at least one file
          pDialogOK->SetLine(1, "");
          pDialogOK->SetLine(2, "");
          pDialogOK->DoModal();
        }
      }

      Update();
    }
  }
}

void CGUIDialogGameSaves::ResumePlaying()
{
  if (playSpeed && g_application.IsPlayingGame())
  {
    // If the player is paused we first need to unpause
    if (g_application.IsPaused())
      g_application.m_pPlayer->Pause();
    g_application.SetPlaySpeed(playSpeed);
  }
  playSpeed = 0;
}

void CGUIDialogGameSaves::OnWindowLoaded()
{
  CGUIDialog::OnWindowLoaded();
  m_viewControl.Reset();
  m_viewControl.SetParentWindow(GetID());
  m_viewControl.AddView(GetControl(CONTROL_THUMBS));
}

void CGUIDialogGameSaves::OnWindowUnload()
{
  CGUIDialog::OnWindowUnload();
  m_viewControl.Reset();
}

CGUIControl *CGUIDialogGameSaves::GetFirstFocusableControl(int id)
{
  if (m_viewControl.HasControl(id))
    id = m_viewControl.GetCurrentControl();
  return CGUIWindow::GetFirstFocusableControl(id);
}
