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

#include "GUIViewStateWindowGames.h"
#include "games/GameManager.h"
#include "FileItem.h"
#include "guilib/GraphicContext.h" // include before ViewState.h
#include "guilib/Key.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/WindowIDs.h"
#include "settings/Settings.h"
#include "utils/StringUtils.h"
#include "view/ViewState.h"
#include "view/ViewStateSettings.h"


CGUIViewStateWindowGames::CGUIViewStateWindowGames(const CFileItemList& items) : CGUIViewState(items)
{
  AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "%I", "%L", ""));  // Filename, Size | Label, empty
  SetSortMethod(SORT_METHOD_NONE);

  SetViewAsControl(DEFAULT_VIEW_LIST);

  SetSortOrder(SortOrderNone);

  LoadViewState(items.GetPath(), WINDOW_GAMES);
}

void CGUIViewStateWindowGames::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_GAMES, CViewStateSettings::Get().Get("games"));
}

CStdString CGUIViewStateWindowGames::GetLockType()
{
  return "games";
}

CStdString CGUIViewStateWindowGames::GetExtensions()
{
  CStdStringArray exts;
  CGameManager::Get().GetExtensions(exts);
  return StringUtils::JoinString(exts, "|");
}

VECSOURCES& CGUIViewStateWindowGames::GetSources()
{
  m_sources.clear();
  
  // Files
  {
    CMediaSource share;
    share.strPath = "sources://games/";
    share.strName = g_localizeStrings.Get(744);
    share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    share.m_strThumbnailImage = "DefaultFolder.png";
    m_sources.push_back(share);
  }

  // Add-ons
  AddAddonsSource("game", g_localizeStrings.Get(1052), "DefaultAddonGame.png");
  return CGUIViewState::GetSources();
}
