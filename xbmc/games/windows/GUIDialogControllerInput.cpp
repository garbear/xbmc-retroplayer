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

#include "GUIDialogControllerInput.h"
#include "games/addons/GamePeripheral.h"
#include "guilib/Geometry.h"
#include "guilib/GUIButtonControl.h"
#include "guilib/GUIControl.h"
#include "guilib/GUIControlGroupList.h"
#include "guilib/GUIFocusPlane.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/WindowIDs.h"
#include "input/joysticks/JoystickDriverPrimitive.h"
#include "input/Key.h"
#include "peripherals/Peripherals.h"
#include "peripherals/addons/AddonJoystickButtonMapper.h" // TODO
#include "peripherals/devices/Peripheral.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

using namespace GAME;
using namespace PERIPHERALS;

#define GROUP_LIST             996
#define BUTTON_TEMPLATE       1000
#define BUTTON_START          1001

#define AXIS_THRESHOLD  0.5f

// --- CGUIJoystickDriverHandler -----------------------------------------------

CGUIJoystickDriverHandler::CGUIJoystickDriverHandler(CGUIDialogControllerInput* dialog, CPeripheral* device)
  : m_dialog(dialog),
    m_device(device)
{
  assert(m_dialog);
  assert(m_device);

  m_device->RegisterJoystickDriverHandler(this);
}

CGUIJoystickDriverHandler::~CGUIJoystickDriverHandler(void)
{
  m_device->UnregisterJoystickDriverHandler(this);
}

void CGUIJoystickDriverHandler::OnButtonMotion(unsigned int buttonIndex, bool bPressed)
{
  if (bPressed)
    m_dialog->OnButton(m_device, buttonIndex);
}

void CGUIJoystickDriverHandler::OnHatMotion(unsigned int hatIndex, HatDirection direction)
{
  if (direction == HatDirectionUp    ||
      direction == HatDirectionRight ||
      direction == HatDirectionDown  ||
      direction == HatDirectionLeft)
  {
    m_dialog->OnHat(m_device, hatIndex, direction);
  }
}

void CGUIJoystickDriverHandler::OnAxisMotion(unsigned int axisIndex, float position)
{
  if (position >= AXIS_THRESHOLD)
    m_dialog->OnAxis(m_device, axisIndex);
}

// --- CGUIDialogControllerInput -----------------------------------------------

CGUIDialogControllerInput::CGUIDialogControllerInput(void)
  : CGUIDialog(WINDOW_DIALOG_CONTROLLER_INPUT, "DialogControllerInput.xml"),
    CThread("CtrlrInput"),
    m_focusControl(NULL),
    m_promptIndex(-1)
{
  m_loadType = KEEP_IN_MEMORY;
}

void CGUIDialogControllerInput::Process(void)
{
  AddDriverHandlers();

  AbortableWait(m_inputEvent);

  ClearDriverHandlers();
}

bool CGUIDialogControllerInput::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_CLICKED:
    {
      if (OnClick(message.GetSenderId()))
        return true;
      break;
    }
    case GUI_MSG_FOCUSED:
    {
      int focusedControl = message.GetControlId();
      OnFocus(focusedControl);
      break;
    }
    case GUI_MSG_LOSTFOCUS:
    case GUI_MSG_UNFOCUS_ALL:
    {
      OnFocus(-1);
      break;
    }
  }

  return CGUIDialog::OnMessage(message);
}

bool CGUIDialogControllerInput::OnAction(const CAction& action)
{
  if (action.GetID() == ACTION_CONTEXT_MENU)
  {
    Close();
    return true;
  }

  return CGUIDialog::OnAction(action);
}

void CGUIDialogControllerInput::OnInitWindow(void)
{
  CGUIDialog::OnInitWindow();

  // disable the template button control
  CGUIButtonControl* pButtonTemplate = GetButtonTemplate();
  if (pButtonTemplate)
    pButtonTemplate->SetVisible(false);

  SetFocusedControl(GROUP_LIST, m_lastControlID);
}

void CGUIDialogControllerInput::OnDeinitWindow(int nextWindowID)
{
  if (m_focusControl)
    m_focusControl->Unfocus();

  // save selected item for next time
  if (m_peripheral)
  {
    int iFocusedControl = GetFocusedControl(GROUP_LIST);
    if (iFocusedControl >= BUTTON_START)
      m_lastControlIds[m_peripheral] = iFocusedControl;
  }

  CGUIDialog::OnDeinitWindow(nextWindowID);
}

void CGUIDialogControllerInput::DoModal(const GamePeripheralPtr& peripheral, CGUIFocusPlane* focusControl)
{
  if (IsDialogRunning())
    return;

  Initialize();

  if (SetupButtons(peripheral, focusControl))
    CGUIDialog::DoModal();

  CleanupButtons();
}

void CGUIDialogControllerInput::OnButton(PERIPHERALS::CPeripheral* device, unsigned int buttonIndex)
{
  if (IsPrompting())
  {
    CAddonJoystickButtonMapper mapper(device, m_peripheral->Addon()->ID()); // TODO

    if (mapper.Load())
      mapper.MapButton(m_promptIndex, CJoystickDriverPrimitive(buttonIndex));

    CancelPrompt();
  }
}

void CGUIDialogControllerInput::OnHat(PERIPHERALS::CPeripheral* device, unsigned int hatIndex, HatDirection direction)
{
  if (IsPrompting())
  {
    CAddonJoystickButtonMapper mapper(device, m_peripheral->Addon()->ID()); // TODO

    if (mapper.Load())
      mapper.MapButton(m_promptIndex, CJoystickDriverPrimitive(hatIndex, direction));

    CancelPrompt();
  }
}

void CGUIDialogControllerInput::OnAxis(PERIPHERALS::CPeripheral* device, unsigned int axisIndex)
{
  // TODO
  CancelPrompt();
}

void CGUIDialogControllerInput::OnFocus(int iFocusedControl)
{
  if (m_peripheral && m_focusControl)
  {
    const std::vector<GAME::Button>& buttons = m_peripheral->Buttons();

    int iFocusedIndex = iFocusedControl - BUTTON_START;

    if (IsPrompting() && iFocusedIndex != m_promptIndex)
      CancelPrompt();

    if (0 <= iFocusedIndex && iFocusedIndex < (int)buttons.size())
      m_focusControl->SetFocus(buttons[iFocusedIndex].focusArea);
    else
      m_focusControl->Unfocus();
  }
}

bool CGUIDialogControllerInput::OnClick(int iSelectedControl)
{
  if (m_peripheral && m_focusControl && iSelectedControl >= BUTTON_START)
  {
    PromptForInput(iSelectedControl - BUTTON_START);
    return true;
  }

  return false;
}

void CGUIDialogControllerInput::PromptForInput(unsigned int buttonIndex)
{
  if (IsPrompting())
    return;

  const std::vector<GAME::Button>& buttons = m_peripheral->Buttons();
  if (buttonIndex < buttons.size())
  {
    const GAME::Button& button = buttons[buttonIndex];

    // Update label
    std::string promptMsg = g_localizeStrings.Get(35051); // "Press %s"
    std::string prompt = StringUtils::Format(promptMsg.c_str(), button.strLabel.c_str());
    SET_CONTROL_LABEL(BUTTON_START + buttonIndex, prompt);

    m_promptIndex = buttonIndex;

    m_inputEvent.Reset();
    Create();
  }
}

void CGUIDialogControllerInput::CancelPrompt(void)
{
  m_inputEvent.Set();

  if (!IsPrompting())
    return;

  const std::vector<GAME::Button>& buttons = m_peripheral->Buttons();
  if (m_promptIndex < (int)buttons.size())
  {
    const GAME::Button& button = buttons[m_promptIndex];

    // Change label back
    SET_CONTROL_LABEL(BUTTON_START + m_promptIndex, button.strLabel);

    m_promptIndex = -1;
  }
}

int CGUIDialogControllerInput::GetFocusedControl(int iControl)
{
  CGUIMessage msg(GUI_MSG_ITEM_SELECTED, GetID(), iControl);

  if (CGUIWindow::OnMessage(msg))
    return msg.GetParam1() >= 0 ? msg.GetParam1() : -1;

  return -1;
}

void CGUIDialogControllerInput::SetFocusedControl(int iControl, int iFocusedControl)
{
  CGUIMessage msg(GUI_MSG_ITEM_SELECT, GetID(), iControl, iFocusedControl);
  OnMessage(msg);
}

bool CGUIDialogControllerInput::SetupButtons(const GamePeripheralPtr& peripheral, CGUIFocusPlane* focusControl)
{
  if (!peripheral || !focusControl)
    return false;

  CGUIButtonControl* pButtonTemplate = GetButtonTemplate();
  CGUIControlGroupList* pGroupList = dynamic_cast<CGUIControlGroupList*>(GetControl(GROUP_LIST));

  if (!pButtonTemplate || !pGroupList)
    return false;

  const std::vector<GAME::Button>& buttons = peripheral->Buttons();

  unsigned int buttonId = BUTTON_START;
  for (std::vector<GAME::Button>::const_iterator it = buttons.begin(); it != buttons.end(); ++it)
  {
    CGUIButtonControl* pButton = MakeButton(it->strLabel, buttonId++, pButtonTemplate);

    // Try inserting context buttons at position specified by template button,
    // if template button is not in grouplist fallback to adding new buttons at
    // the end of grouplist
    if (!pGroupList->InsertControl(pButton, pButtonTemplate))
      pGroupList->AddControl(pButton);
  }

  // Update our default control
  m_defaultControl = GROUP_LIST;
  m_lastControlID = BUTTON_START;

  m_peripheral = peripheral;
  m_focusControl = focusControl;

  // Restore last selected control
  std::map<GAME::GamePeripheralPtr, unsigned int>::const_iterator it = m_lastControlIds.find(m_peripheral);
  if (it != m_lastControlIds.end())
    m_lastControlID = it->second;

  return true;
}

void CGUIDialogControllerInput::CleanupButtons(void)
{
  CGUIControlGroupList* pGroupList = dynamic_cast<CGUIControlGroupList*>(GetControl(GROUP_LIST));
  if (pGroupList)
    pGroupList->ClearAll();

  m_peripheral = NULL;
  m_focusControl = NULL;
}

CGUIButtonControl* CGUIDialogControllerInput::GetButtonTemplate(void)
{
  CGUIButtonControl* pButtonTemplate = dynamic_cast<CGUIButtonControl*>(GetFirstFocusableControl(BUTTON_TEMPLATE));
  if (!pButtonTemplate)
    pButtonTemplate = dynamic_cast<CGUIButtonControl*>(GetControl(BUTTON_TEMPLATE));
  return pButtonTemplate;
}

CGUIButtonControl* CGUIDialogControllerInput::MakeButton(const std::string& strLabel,
                                                         unsigned int       id,
                                                         CGUIButtonControl* pButtonTemplate)
{
  CGUIButtonControl* pButton = new CGUIButtonControl(*pButtonTemplate);

  // Set the button's ID and position
  pButton->SetID(id);
  pButton->SetVisible(true);
  pButton->SetLabel(strLabel);
  pButton->SetPosition(pButtonTemplate->GetXPosition(), pButtonTemplate->GetYPosition());

  return pButton;
}

void CGUIDialogControllerInput::AddDriverHandlers(void)
{
  std::vector<CPeripheral*> peripherals = ScanPeripherals();
  for (std::vector<CPeripheral*>::iterator it = peripherals.begin(); it != peripherals.end(); ++it)
    m_driverHandlers.push_back(new CGUIJoystickDriverHandler(this, *it));
}

void CGUIDialogControllerInput::ClearDriverHandlers(void)
{
  for (std::vector<CGUIJoystickDriverHandler*>::iterator it = m_driverHandlers.begin(); it != m_driverHandlers.end(); ++it)
    delete *it;
  m_driverHandlers.clear();
}

std::vector<CPeripheral*> CGUIDialogControllerInput::ScanPeripherals(void)
{
  std::vector<CPeripheral*> peripherals;

  g_peripherals.GetPeripheralsWithFeature(peripherals, FEATURE_JOYSTICK);
  g_peripherals.GetPeripheralsWithFeature(peripherals, FEATURE_KEYBOARD);

  return peripherals;
}
