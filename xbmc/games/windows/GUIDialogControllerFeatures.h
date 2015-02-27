#pragma once

/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "games/GameTypes.h"
#include "guilib/GUIDialog.h"

#include <string>

class CGUIButtonControl;
class CGUIFocusPlane;
namespace GAME { class CControllerLayout; }

class CGUIDialogControllerFeatures : public CGUIDialog
{
public:
  CGUIDialogControllerFeatures(void);
  virtual ~CGUIDialogControllerFeatures(void) { }

  void DoModal(const GAME::ControllerLayoutPtr& layout, CGUIFocusPlane* focusControl);

  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction& action);

protected:
  CGUIButtonControl* GetButtonTemplate(void);
  CGUIButtonControl* MakeButton(const std::string& strLabel, unsigned int id, CGUIButtonControl* pButtonTemplate);

  bool OnMove(unsigned int iSelected);
  bool OnClick(unsigned int iSelected);

  virtual void OnInitWindow(void);

private:
  bool SetupButtons(const GAME::ControllerLayoutPtr& layout, CGUIFocusPlane* focusControl);
  void CleanupButtons(void);

  int GetSelectedItem(int iControl);

  GAME::ControllerLayoutPtr m_layout;
  CGUIFocusPlane*           m_focusControl;
};
