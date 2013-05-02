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
#pragma once

#include "FileItem.h"
#include "guilib/GUIDialog.h"
#include "utils/StdString.h"
#include "view/GUIViewControl.h"

class CGUIDialogGameSaves : public CGUIDialog
{
public:
  CGUIDialogGameSaves();
  virtual ~CGUIDialogGameSaves();
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction &action);
  virtual void OnWindowLoaded();
  virtual void OnWindowUnload();

protected:
  bool OnContextMenu(const CFileItemPtr &savefile);
  void Update();
  void CreateSaveState();
  void LoadSaveState(const CFileItemPtr &savefile);
  void RenameSaveState(const CFileItemPtr &saveFile);
  void DeleteSaveState(const CStdString &saveStatePath);
  void ClearSaveStates();
  void ResumePlaying();

  CGUIControl *GetFirstFocusableControl(int id);

  CFileItemList* m_vecItems;
  CGUIViewControl m_viewControl;

  // Game file path, use current playing game if unspecified on window creation
  CStdString m_gamePath;
  // Game client mask, used if game is playing to avoid showing save states for
  // other game clients
  CStdString m_gameClient;
  // Restore play speed on dialog close
  int playSpeed;
};
