/*
 *      Copyright (C) 2015 Team XBMC
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

#include "games/GameTypes.h"
#include "games/windows/wizards/IGUIControllerWizard.h"
#include "guilib/GUIDialog.h"

#include <string>

class CGUIButtonControl;
class CGUIFocusPlane;

class CGUIDialogControllerInput : public CGUIDialog,
                                  public GAME::IGUIControllerWizardCallbacks
{
public:
  CGUIDialogControllerInput(void);
  virtual ~CGUIDialogControllerInput(void) { }

  // implementation of CGUIControl
  virtual bool OnMessage(CGUIMessage& message);

  // implementation of IGUIControllerWizardCallbacks
  virtual void Focus(unsigned int iFeature);
  virtual void SetLabel(unsigned int iFeature, const std::string& strLabel);
  virtual void ResetLabel(unsigned int iFeature);
  virtual void End(void);

  void DoModal(const GAME::GameControllerPtr& controller, CGUIFocusPlane* focusControl);

protected:
  // implementation of CGUIWindow
  virtual void OnInitWindow(void);
  virtual void OnDeinitWindow(int nextWindowID);

  void OnFocus(int iFocusedControl);
  bool OnClick(int iSelectedControl);

private:
  bool SetupButtons(const GAME::GameControllerPtr& controller, CGUIFocusPlane* focusControl);
  void CleanupButtons(void);

  CGUIButtonControl* GetButtonTemplate(void);
  CGUIButtonControl* MakeButton(const std::string& strLabel, unsigned int id, CGUIButtonControl* pButtonTemplate);

  GAME::GameControllerPtr     m_controller; // Active controller
  CGUIFocusPlane*             m_focusControl;
  unsigned int                m_selectedFeature;
  GAME::IGUIControllerWizard* m_wizard;
};
