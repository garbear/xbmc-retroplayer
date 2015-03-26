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

#include "PeripheralKeyboard.h"
#include "input/InputManager.h"
#include "input/joysticks/generic/GenericJoystickKeyboardHandler.h"
#include "input/Key.h"

using namespace PERIPHERALS;

CPeripheralKeyboard::CPeripheralKeyboard(const PeripheralScanResult& scanResult) :
  CPeripheral(scanResult)
{
  m_features.push_back(FEATURE_KEYBOARD);
}

CPeripheralKeyboard::~CPeripheralKeyboard(void)
{
  CInputManager::Get().UnregisterKeyboardHandler(this);

  while (!m_keyboardHandlers.empty())
    UnregisterJoystickDriverHandler(m_keyboardHandlers.begin()->first);
}

bool CPeripheralKeyboard::InitialiseFeature(const PeripheralFeature feature)
{
  if (CPeripheral::InitialiseFeature(feature))
  {
    CInputManager::Get().RegisterKeyboardHandler(this);
    return true;
  }

  return false;
}

void CPeripheralKeyboard::RegisterJoystickDriverHandler(IJoystickDriverHandler* handler)
{
  std::map<IJoystickDriverHandler*, IKeyboardHandler*>::iterator it = m_keyboardHandlers.find(handler);
  if (handler && it == m_keyboardHandlers.end())
    m_keyboardHandlers[handler] = new CGenericJoystickKeyboardHandler(handler);
}

void CPeripheralKeyboard::UnregisterJoystickDriverHandler(IJoystickDriverHandler* handler)
{
  std::map<IJoystickDriverHandler*, IKeyboardHandler*>::iterator it = m_keyboardHandlers.find(handler);
  if (it != m_keyboardHandlers.end())
  {
    delete it->second;
    m_keyboardHandlers.erase(it);
  }
}

bool CPeripheralKeyboard::OnKeyPress(const CKey& key)
{
  for (std::map<IJoystickDriverHandler*, IKeyboardHandler*>::iterator it = m_keyboardHandlers.begin(); it != m_keyboardHandlers.end(); ++it)
    it->second->OnKeyPress(key);

  return true; // TODO
}

void CPeripheralKeyboard::OnKeyRelease(const CKey& key)
{
  for (std::map<IJoystickDriverHandler*, IKeyboardHandler*>::iterator it = m_keyboardHandlers.begin(); it != m_keyboardHandlers.end(); ++it)
    it->second->OnKeyRelease(key);
}
