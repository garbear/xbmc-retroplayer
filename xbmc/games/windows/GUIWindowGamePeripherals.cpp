/*
 *      Copyright (C) 2012-2014 Team XBMC
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

#include "GUIWindowGamePeripherals.h"
#include "GUIDialogControllerInput.h"
#include "addons/AddonManager.h"
#include "games/ControllerLayout.h"
#include "guilib/GUIControllerLayout.h"
#include "guilib/GUIFocusPlane.h"
#include "guilib/GUIMessage.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/WindowIDs.h"
#include "input/Key.h"
#include "utils/log.h"

#include <set>
#include <string>

using namespace ADDON;
using namespace GAME;

#define DEFAULT_GAME_CONTROLLER     "game.controller.default"

#define CONTROL_PERIPHERAL_LIST     1
#define CONTROL_FOCUS_PLANE         2
#define CONTROL_CONTROLLER_LAYOUT   3

CGUIWindowGamePeripherals::CGUIWindowGamePeripherals(void) :
  CGUIWindow(WINDOW_GAME_PERIPHERALS, "GamePeripherals.xml"),
  m_selectedItem(-1)
{
}

ControllerLayoutPtr CGUIWindowGamePeripherals::GetLayout(const AddonPtr& peripheral) const
{
  for (ControllerLayoutVector::const_iterator it = m_layouts.begin(); it != m_layouts.end(); ++it)
  {
    if ((*it)->Addon()->ID() == peripheral->ID())
      return *it;
  }
  return CControllerLayout::EmptyPtr;
}

ControllerLayoutPtr CGUIWindowGamePeripherals::LoadLayout(const AddonPtr& peripheral)
{
  const ControllerLayoutPtr& layout = GetLayout(peripheral);
  if (!layout)
  {
    CGUIControl* control = GetControl(CONTROL_CONTROLLER_LAYOUT);
    ControllerLayoutPtr newLayout = ControllerLayoutPtr(new CControllerLayout(peripheral, control));
    if (newLayout->Load())
    {
      m_layouts.push_back(newLayout);
      return *(m_layouts.end() - 1);
    }
  }
  return layout;
}

bool CGUIWindowGamePeripherals::OnClick(int iItem)
{
  if (0 <= iItem && iItem < (int)m_layouts.size())
  {
    CGUIDialogControllerInput* pMenu = dynamic_cast<CGUIDialogControllerInput*>(g_windowManager.GetWindow(WINDOW_DIALOG_CONTROLLER_INPUT));
    if (pMenu)
      pMenu->DoModal(m_layouts[iItem], dynamic_cast<CGUIFocusPlane*>(GetControl(CONTROL_FOCUS_PLANE)));
    return true;
  }
  return false;
}

bool CGUIWindowGamePeripherals::OnSelect(int iItem)
{
  if (0 <= iItem && iItem < (int)m_layouts.size() && iItem != m_selectedItem)
  {
    m_selectedItem = iItem;

    CGUIControllerLayout* pLayout = dynamic_cast<CGUIControllerLayout*>(GetControl(CONTROL_CONTROLLER_LAYOUT));
    if (pLayout)
    {
      pLayout->ActivateLayout(m_layouts[iItem]);
      return true;
    }
  }
  return false;
}

void CGUIWindowGamePeripherals::OnInitWindow()
{
  CGUIWindow::OnInitWindow();

  // TODO: Register with controller

  m_items.Clear();

  VECADDONS peripherals;
  CAddonMgr::Get().GetAddons(ADDON_GAME_PERIPHERAL, peripherals);
  for (VECADDONS::const_iterator it = peripherals.begin(); it != peripherals.end(); ++it)
  {
    if (!LoadLayout(*it))
      CLog::Log(LOGERROR, "Failed to load controller layout for %s", (*it)->ID().c_str());
  }

  for (ControllerLayoutVector::iterator it = m_layouts.begin(); it != m_layouts.end(); ++it)
  {
    CFileItemPtr item(new CFileItem((*it)->Label()));
    m_items.Add(item);
  }

  CGUIMessage msg(GUI_MSG_LABEL_BIND, GetID(), CONTROL_PERIPHERAL_LIST, 0, 0, &m_items);
  g_windowManager.SendMessage(msg);
}

void CGUIWindowGamePeripherals::OnDeinitWindow(int nextWindowID)
{
  // TODO: Unregister with controller

  CGUIWindow::OnDeinitWindow(nextWindowID);
}

bool CGUIWindowGamePeripherals::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();

      if (iControl == CONTROL_PERIPHERAL_LIST)
      {
        int iItem = GetSelectedItem(CONTROL_PERIPHERAL_LIST);
        return OnClick(iItem);
      }
      break;
    }
  }

  const bool bHandled = CGUIWindow::OnMessage(message);

  OnSelect(GetSelectedItem(CONTROL_PERIPHERAL_LIST));

  return bHandled;
}

bool CGUIWindowGamePeripherals::OnAction(const CAction& action)
{
  if (CGUIWindow::OnAction(action))
  {
    OnSelect(GetSelectedItem(CONTROL_PERIPHERAL_LIST));
    return true;
  }
  return false;
}

int CGUIWindowGamePeripherals::GetSelectedItem(int iControl)
{
  if (!m_items.IsEmpty())
  {
    CGUIMessage msg(GUI_MSG_ITEM_SELECTED, GetID(), iControl);
    if (CGUIWindow::OnMessage(msg))
      return (int)msg.GetParam1();
  }
  return -1;
}
