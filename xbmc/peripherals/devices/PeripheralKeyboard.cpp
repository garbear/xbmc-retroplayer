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

#include "PeripheralKeyboard.h"
#include "input/InputManager.h"
#include "input/joysticks/generic/GenericJoystickKeyboardHandler.h"
#include "input/Key.h"
#include "threads/SingleLock.h"

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
  CSingleLock lock(m_handlerMutex);

  bool bFound = false;

  for (KeyboardHandlerVector::iterator it = m_keyboardHandlers.begin(); !bFound && it != m_keyboardHandlers.end(); ++it)
  {
    if (it->first == handler)
      bFound = true;
  }

  if (!bFound)
    m_keyboardHandlers.insert(m_keyboardHandlers.begin(),
                              std::make_pair(handler, new CGenericJoystickKeyboardHandler(handler)));
}

void CPeripheralKeyboard::UnregisterJoystickDriverHandler(IJoystickDriverHandler* handler)
{
  CSingleLock lock(m_handlerMutex);

  for (KeyboardHandlerVector::iterator it = m_keyboardHandlers.begin(); it != m_keyboardHandlers.end(); ++it)
  {
    if (it->first == handler)
    {
      delete it->second;
      m_keyboardHandlers.erase(it);
      break;
    }
  }
}

bool CPeripheralKeyboard::OnKeyPress(const CKey& key)
{
  CSingleLock lock(m_handlerMutex);

  bool bHandled = false;

  for (KeyboardHandlerVector::iterator it = m_keyboardHandlers.begin(); it != m_keyboardHandlers.end(); ++it)
    bHandled = bHandled || it->second->OnKeyPress(key);

  return bHandled;
}

void CPeripheralKeyboard::OnKeyRelease(const CKey& key)
{
  CSingleLock lock(m_handlerMutex);

  for (KeyboardHandlerVector::iterator it = m_keyboardHandlers.begin(); it != m_keyboardHandlers.end(); ++it)
    it->second->OnKeyRelease(key);
}
