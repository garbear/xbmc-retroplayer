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
#include "input/joysticks/generic/GenericJoystickDriverHandler.h"
#include "input/joysticks/generic/GenericJoystickKeyboardHandler.h"
#include "input/joysticks/IJoystickInputHandler.h"
#include "input/Key.h"
#include "peripherals/addons/AddonJoystickButtonMap.h"

#include <algorithm>

using namespace PERIPHERALS;

CPeripheralKeyboard::CPeripheralKeyboard(const PeripheralScanResult& scanResult) :
  CPeripheral(scanResult)
{
  m_features.push_back(FEATURE_KEYBOARD);
}

CPeripheralKeyboard::~CPeripheralKeyboard(void)
{
  CInputManager::Get().UnregisterKeyboardHandler(this);
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
  m_keyboardHandlers.push_back(new CGenericJoystickKeyboardHandler(handler)); // TODO: leaks
}

void CPeripheralKeyboard::UnregisterJoystickDriverHandler(IJoystickDriverHandler* handler)
{
  // TODO
}

void CPeripheralKeyboard::RegisterJoystickInputHandler(IJoystickInputHandler* handler)
{
  IJoystickButtonMap* buttonMap = new CAddonJoystickButtonMap(this, handler->DeviceID()); // TODO: leaks
  if (buttonMap->Load())
    RegisterJoystickDriverHandler(new CGenericJoystickDriverHandler(handler, buttonMap)); // TODO: leaks
  else
    SAFE_DELETE(buttonMap);
}

void CPeripheralKeyboard::UnregisterJoystickInputHandler(IJoystickInputHandler* handler)
{
  for (std::vector<IKeyboardHandler*>::iterator it = m_keyboardHandlers.begin(); it != m_keyboardHandlers.end(); ++it)
  {
    // TODO
    // if ((*it)->InputHandler() == handler)
    // {
    //   delete (*it)->ButtonMap();
    //   delete (*it);
    //   m_driverHandlers.erase(it);
    //   break;
    // }
  }
}

bool CPeripheralKeyboard::OnKeyPress(const CKey& key)
{
  bool bHandled = false;

  for (std::vector<IKeyboardHandler*>::iterator it = m_keyboardHandlers.begin(); it != m_keyboardHandlers.end(); ++it)
    bHandled |= (*it)->OnKeyPress(key);

  return bHandled;
}

void CPeripheralKeyboard::OnKeyRelease(const CKey& key)
{
  for (std::vector<IKeyboardHandler*>::iterator it = m_keyboardHandlers.begin(); it != m_keyboardHandlers.end(); ++it)
    (*it)->OnKeyRelease(key);
}
