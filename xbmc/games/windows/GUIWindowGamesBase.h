#pragma once

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

#include "system.h"
#include "windows/GUIMediaWindow.h"
#include "games/GameDatabase.h"
//#include "PlayListPlayer.h"
//#include "ThumbLoader.h"
//#include "utils/StreamDetails.h"

class CGUIDialogProgress;

class CGUIWindowGamesBase : public CGUIMediaWindow//, public IBackgroundLoaderObserver//, public IStreamDetailsObserver
{
public:
  CGUIWindowGamesBase(int id, const CStdString &xmlFile);
  virtual ~CGUIWindowGamesBase(void) { }

  // Message processing and dispatching inherited from CGUIMediaWindow
public:
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction &action) { return CGUIMediaWindow::OnAction(action); }
  virtual bool OnFileAction(int iItem, int action);
protected:
  virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);

  // Install the messages
protected:
  virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
          void GetNonContextButtons(int itemNumber, CContextButtons &buttons) { }

  // Message Handling
protected:
  virtual bool OnClick(int iItem) { return CGUIMediaWindow::OnClick(iItem); }
  virtual bool OnPlayMedia(int iItem) { return CGUIMediaWindow::OnPlayMedia(iItem); }
  virtual void OnDeleteItem(int iItem) { CGUIMediaWindow::OnPlayMedia(iItem); }
          void OnRenameItem(int iItem) { CGUIMediaWindow::OnPlayMedia(iItem); }
          bool OnSelect(int iItem);

  // Browsing
  virtual bool GetDirectory(const CStdString &strDirectory, CFileItemList &items) { return CGUIMediaWindow::GetDirectory(strDirectory, items); }
  virtual void OnPrepareFileItems(CFileItemList &items) { }
  virtual CStdString GetStartFolder(const CStdString &dir);

          void AddToDatabase(int iItem);

protected:
  CGameDatabase       m_database;
  CGUIDialogProgress *m_dlgProgress;
};
