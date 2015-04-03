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
#include "input/joysticks/IJoystickDriverHandler.h"
#include "threads/Event.h"
#include "threads/Thread.h"

#include <map>
#include <string>
#include <vector>

class CGUIDialogControllerInput;
namespace PERIPHERALS { class CPeripheral; }

class CGUIJoystickDriverHandler : public IJoystickDriverHandler
{
public:
  CGUIJoystickDriverHandler(CGUIDialogControllerInput* dialog, PERIPHERALS::CPeripheral* device);

  virtual ~CGUIJoystickDriverHandler(void);

  // Implementation of IJoystickDriverHandler
  virtual void OnButtonMotion(unsigned int buttonIndex, bool bPressed);
  virtual void OnHatMotion(unsigned int hatIndex, HatDirection direction);
  virtual void OnAxisMotion(unsigned int axisIndex, float position);
  virtual void ProcessAxisMotions(void) { }

private:
  CGUIDialogControllerInput* const m_dialog;
  PERIPHERALS::CPeripheral* const  m_device;
};

class CGUIButtonControl;
class CGUIFocusPlane;

class CGUIDialogControllerInput : public CGUIDialog, protected CThread
{
public:
  CGUIDialogControllerInput(void);
  virtual ~CGUIDialogControllerInput(void) { }

  // implementation of CGUIControl
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction& action);

  void DoModal(const GAME::GamePeripheralPtr& peripheral, CGUIFocusPlane* focusControl);

  void OnButton(PERIPHERALS::CPeripheral* device, unsigned int buttonIndex);
  void OnHat(PERIPHERALS::CPeripheral* device, unsigned int hatIndex, HatDirection direction);
  void OnAxis(PERIPHERALS::CPeripheral* device, unsigned int axisIndex);

protected:
  // implementation of CThread
  virtual void Process(void);

  // implementation of CGUIWindow
  virtual void OnInitWindow(void);
  virtual void OnDeinitWindow(int nextWindowID);

  void OnFocus(int iFocusedControl);
  bool OnClick(int iSelectedControl);

private:
  void PromptForInput(unsigned int buttonIndex);
  void CancelPrompt(void);
  bool IsPrompting(void) const { return m_peripheral && m_promptIndex >= 0; }

  int GetFocusedControl(int iControl);
  void SetFocusedControl(int iControl, int iFocusedControl);

  bool SetupButtons(const GAME::GamePeripheralPtr& peripheral, CGUIFocusPlane* focusControl);
  void CleanupButtons(void);

  CGUIButtonControl* GetButtonTemplate(void);
  CGUIButtonControl* MakeButton(const std::string& strLabel, unsigned int id, CGUIButtonControl* pButtonTemplate);

  void AddDriverHandlers(void);
  void ClearDriverHandlers(void);
  std::vector<PERIPHERALS::CPeripheral*> ScanPeripherals(void);

  GAME::GamePeripheralPtr m_peripheral;
  CGUIFocusPlane*         m_focusControl;

  std::map<GAME::GamePeripheralPtr, unsigned int> m_lastControlIds; // peripheral add-on ID -> last selected control ID
  int                                             m_promptIndex; // Index of feature being prompted for input
  std::vector<CGUIJoystickDriverHandler*>         m_driverHandlers;
  CEvent                                          m_inputEvent;
};
