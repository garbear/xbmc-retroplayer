/*
 *      Copyright (C) 2014 Team XBMC
 *      http://xbmc.org
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

#include "addons/Addon.h"
#include "games/GameTypes.h"
#include "guilib/GUIWindow.h"
#include "FileItem.h"

#include <map>

class CGUIWindowGamePeripherals : public CGUIWindow
{
public:
  CGUIWindowGamePeripherals(void);
  virtual ~CGUIWindowGamePeripherals(void) { }

  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction& action);

  virtual void OnDeinitWindow(int nextWindowID);

protected:
  GAME::ControllerLayoutPtr GetLayout(const ADDON::AddonPtr& peripheral) const;
  GAME::ControllerLayoutPtr LoadLayout(const ADDON::AddonPtr& peripheral);

  virtual void OnInitWindow(void);

private:
  bool OnClick(int iItem);
  bool OnSelect(int iItem);

  int GetSelectedItem(int iControl);

  GAME::ControllerLayoutVector m_layouts;
  CFileItemList                m_items;
  int                          m_selectedItem;
};
