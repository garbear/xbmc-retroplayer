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
#include "games/addons/GameController.h"
#include "games/windows/wizards/GUIControllerWizard.h"
#include "guilib/Geometry.h"
#include "guilib/GUIButtonControl.h"
#include "guilib/GUIControl.h"
#include "guilib/GUIControlGroupList.h"
#include "guilib/GUIFocusPlane.h"
#include "guilib/GUIMessage.h"
#include "guilib/WindowIDs.h"

using namespace GAME;

#define GROUP_LIST             996
#define BUTTON_TEMPLATE       1000
#define BUTTON_START          1001

CGUIDialogControllerInput::CGUIDialogControllerInput(void)
  : CGUIDialog(WINDOW_DIALOG_CONTROLLER_INPUT, "DialogControllerInput.xml"),
    m_focusControl(NULL),
    m_selectedFeature(0),
    m_wizard(NULL)
{
  // initialize CGUIWindow
  m_loadType = KEEP_IN_MEMORY;
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
      const int focusedControl = message.GetControlId();
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

void CGUIDialogControllerInput::OnInitWindow(void)
{
  CGUIDialog::OnInitWindow();

  // Disable the template button control
  CGUIButtonControl* pButtonTemplate = GetButtonTemplate();
  if (pButtonTemplate)
    pButtonTemplate->SetVisible(false);
}

void CGUIDialogControllerInput::OnDeinitWindow(int nextWindowID)
{
  if (m_focusControl)
    m_focusControl->Unfocus();

  CGUIDialog::OnDeinitWindow(nextWindowID);
}

void CGUIDialogControllerInput::Focus(unsigned int iFeature)
{
  //CGUIMessage msg(GUI_MSG_ITEM_SELECT, GetID(), GROUP_LIST, iFeature + BUTTON_START);
  CGUIMessage msg(GUI_MSG_SETFOCUS, GetID(), iFeature + BUTTON_START);
  OnMessage(msg);
}

void CGUIDialogControllerInput::SetLabel(unsigned int iFeature, const std::string& strLabel)
{
  SET_CONTROL_LABEL(iFeature + BUTTON_START, strLabel);
}

void CGUIDialogControllerInput::ResetLabel(unsigned int iFeature)
{
  if (m_controller)
  {
    const std::vector<CGameControllerFeature>& features = m_controller->Layout().Features();

    if (iFeature < features.size())
    {
      const std::string& strLabel = m_controller->GetString(features.at(iFeature).Label());
      SET_CONTROL_LABEL(iFeature + BUTTON_START, strLabel);
    }
  }
}

void CGUIDialogControllerInput::End(void)
{
  Close();
}

void CGUIDialogControllerInput::DoModal(const GameControllerPtr& controller, CGUIFocusPlane* focusControl)
{
  if (IsDialogRunning())
    return;

  // Initialize CGUIWindow
  if (!Initialize())
    return;

  if (SetupButtons(controller, focusControl))
  {
    m_wizard = new CGUIControllerWizard(this, m_controller);
    m_wizard->Run();

    CGUIDialog::DoModal();

    delete m_wizard;
    m_wizard = NULL;

    m_controller = NULL;
    m_focusControl = NULL;
  }

  CleanupButtons();
}

void CGUIDialogControllerInput::OnFocus(int iFocusedControl)
{
  if (m_controller && m_focusControl)
  {
    const std::vector<CGameControllerFeature>& features = m_controller->Layout().Features();

    const int iFocusedIndex = iFocusedControl - BUTTON_START;

    if (0 <= iFocusedIndex && iFocusedIndex < (int)features.size())
    {
      m_selectedFeature = iFocusedIndex;

      if (m_wizard)
        m_wizard->OnFocus(m_selectedFeature);

      m_focusControl->SetFocus(features[m_selectedFeature].Geometry());
    }
    else
    {
      if (m_wizard)
        m_wizard->Abort();

      m_focusControl->Unfocus();
    }
  }
}

bool CGUIDialogControllerInput::OnClick(int iSelectedControl)
{
  const int iSelectedFeature = iSelectedControl - BUTTON_START;

  if (m_wizard && iSelectedFeature == (int)m_selectedFeature)
  {
    m_wizard->Run(m_selectedFeature);
    return true;
  }

  return false;
}

bool CGUIDialogControllerInput::SetupButtons(const GameControllerPtr& controller, CGUIFocusPlane* focusControl)
{
  if (!controller || !focusControl)
    return false;

  CGUIButtonControl* pButtonTemplate = GetButtonTemplate();
  CGUIControlGroupList* pGroupList = dynamic_cast<CGUIControlGroupList*>(GetControl(GROUP_LIST));

  if (!pButtonTemplate || !pGroupList)
    return false;

  const std::vector<CGameControllerFeature>& features = controller->Layout().Features();

  unsigned int buttonId = BUTTON_START;
  for (std::vector<CGameControllerFeature>::const_iterator it = features.begin(); it != features.end(); ++it)
  {
    CGUIButtonControl* pButton = MakeButton(controller->GetString(it->Label()), buttonId++, pButtonTemplate);

    // Try inserting context buttons at position specified by template button,
    // if template button is not in grouplist fallback to adding new buttons at
    // the end of grouplist
    if (!pGroupList->InsertControl(pButton, pButtonTemplate))
      pGroupList->AddControl(pButton);
  }

  // Configure CGUIControlGroup
  m_defaultControl = GROUP_LIST;

  // Configure CGUIWindow
  m_lastControlID = BUTTON_START;

  // Configure CGUIDialogControllerInput
  m_selectedFeature = 0;

  // Success
  m_controller = controller;
  m_focusControl = focusControl;

  return true;
}

void CGUIDialogControllerInput::CleanupButtons(void)
{
  CGUIControlGroupList* pGroupList = dynamic_cast<CGUIControlGroupList*>(GetControl(GROUP_LIST));
  if (pGroupList)
    pGroupList->ClearAll();

  m_controller = NULL;
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
