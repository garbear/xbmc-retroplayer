/*
 *      Copyright (C) 2012 Garrett Brown
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "RetroPlayerInput.h"
#include "games/libretro/libretro.h"
#include "guilib/GUIWindowManager.h"
#include "input/ButtonTranslator.h"
#include "input/KeyboardStat.h"
#include "utils/log.h"

// For JACTIVE_BUTTON constants
#ifdef TARGET_WINDOWS
#include "input/windows/WINJoystick.h"
#elif defined(HAS_SDL_JOYSTICK) || defined(HAS_EVENT_SERVER)
#include "input/SDLJoystick.h"
#endif


// TODO: do something less... ghetto
static CRetroPlayerInput::Gamepad g_gamepad[GAMEPAD_MAX_CONTROLLERS];

unsigned char &CRetroPlayerInput::Hat::operator[](unsigned int i)
{
  switch (i)
  {
  case 0:  return up;
  case 1:  return right;
  case 2:  return down;
  case 3:  return left;
  default: return up;
  }
}

// TODO: Combine with the above function
const unsigned char &CRetroPlayerInput::Hat::operator[](unsigned int i) const
{
  switch (i)
  {
  case 0:  return up;
  case 1:  return right;
  case 2:  return down;
  case 3:  return left;
  default: return up;
  }
}

#define MAKE_DIRECTION(n, e, s, w) ((n) << 3 | (e) << 2 | (s) << 1 | (w))
const char *CRetroPlayerInput::Hat::GetDirection() const
{
  switch (MAKE_DIRECTION(up, right, down, left))
  {
  case MAKE_DIRECTION(1, 0, 0, 0): return "N";
  case MAKE_DIRECTION(1, 1, 0, 0): return "NE";
  case MAKE_DIRECTION(0, 1, 0, 0): return "E";
  case MAKE_DIRECTION(0, 1, 1, 0): return "SE";
  case MAKE_DIRECTION(0, 0, 1, 0): return "S";
  case MAKE_DIRECTION(0, 0, 1, 1): return "SW";
  case MAKE_DIRECTION(0, 0, 0, 1): return "W";
  case MAKE_DIRECTION(1, 0, 0, 1): return "NW";
  default:                         return "CENTERED";
  }
}


CRetroPlayerInput::CRetroPlayerInput() : m_bActive(false), m_gamepad()
{
}

void CRetroPlayerInput::Begin()
{
  memset(m_joypadState, 0, sizeof(m_joypadState));
  m_gamepad = Gamepad();
  memset(g_gamepad, 0, sizeof(g_gamepad));
  m_bActive = true;
}

void CRetroPlayerInput::Finish()
{
  m_bActive = false;
}

int16_t CRetroPlayerInput::GetInput(unsigned port, unsigned device, unsigned index, unsigned id)
{
  if (!m_bActive)
    return 0;

  CSingleLock lock(m_statesGuard);

  if (port == 0)
  {
    device &= RETRO_DEVICE_MASK;

    switch (device)
    {
    case RETRO_DEVICE_JOYPAD:
      if (id < sizeof(m_joypadState) / sizeof(m_joypadState[0]))
        return m_joypadState[id];
      else
        CLog::Log(LOGDEBUG, "RetroPlayerInput: GetInput() called with invalid ID: %d", id);
      break;
    default: // Only RETRO_DEVICE_JOYPAD is supported currently
      CLog::Log(LOGDEBUG, "RetroPlayerInput: GetInput() called with invalid device: %d", device);
      break;
    }
  }
  return 0;
}

void CRetroPlayerInput::ProcessKeyDown(const CKey &key)
{
  // TODO: Use ID of current window, in case it defines joypad keys outside of
  // <FullscreenGame>. Maybe, if current window ID finds no action, then fallback
  // to WINDOW_FULLSCREEN_GAME.
  CAction action = CButtonTranslator::GetInstance().GetAction(WINDOW_FULLSCREEN_GAME, key);

  CSingleLock lock(m_statesGuard);

  int id = TranslateActionID(action.GetID());
  if (0 <= id && id < (int)(sizeof(m_joypadState) / sizeof(m_joypadState[0])))
  {
    CLog::Log(LOGDEBUG, "RetroPlayerInput: KeyDown, action=%s, ID=%d", action.GetName().c_str(), id);
    m_joypadState[id] = 1;
  }
  else
  {
    CLog::Log(LOGDEBUG, "RetroPlayerInput: Invalid KeyDown, action=%s, ID=%d", action.GetName().c_str(), action.GetID());
  }
}

void CRetroPlayerInput::ProcessKeyUp(const CKey &key)
{
  // TODO: Use ID of current window, in case it defines joypad keys outside of
  // <FullscreenGame>. Maybe, if current window ID finds no action, then fallback
  // to WINDOW_FULLSCREEN_GAME.
  CAction action = CButtonTranslator::GetInstance().GetAction(WINDOW_FULLSCREEN_GAME, key);

  CSingleLock lock(m_statesGuard);

  int id = TranslateActionID(action.GetID());
  if (0 <= id && id < (int)(sizeof(m_joypadState) / sizeof(m_joypadState[0])))
  {
    CLog::Log(LOGDEBUG, "RetroPlayerInput: KeyUp, action=%s, ID=%d", action.GetName().c_str(), id);
    m_joypadState[id] = 0;
  }
  else
  {
    CLog::Log(LOGDEBUG, "RetroPlayerInput: Invalid KeyUp, action=%s, ID=%d", action.GetName().c_str(), action.GetID());
  }
}

void CRetroPlayerInput::ProcessButtonDown(const CStdString &name, int id, unsigned char button)
{
  g_gamepad[id].name = name;
  g_gamepad[id].buttons[button] = 1;
  ProcessGamepad(g_gamepad[id]);
}

void CRetroPlayerInput::ProcessButtonUp(const CStdString &name, int id, unsigned char button)
{
  g_gamepad[id].name = name;
  g_gamepad[id].buttons[button] = 0;
  ProcessGamepad(g_gamepad[id]);
}

void CRetroPlayerInput::ProcessHatState(const CStdString &name, int id, unsigned int hat, const Hat &hatState)
{
  g_gamepad[id].name = name;
  g_gamepad[id].hats[hat] = hatState;
  ProcessGamepad(g_gamepad[id]);
}

void CRetroPlayerInput::ProcessAxisState(const CStdString &name, int id, unsigned int axis, float value)
{
  g_gamepad[id].name = name;
  g_gamepad[id].axes[axis] = value;
  ProcessGamepad(g_gamepad[id]);
}

void CRetroPlayerInput::ProcessGamepad(const Gamepad &gamepad)
{
  int window = !g_windowManager.HasModalDialog() ? g_windowManager.GetActiveWindow() : g_windowManager.GetTopMostModalDialogID();

  CSingleLock lock(m_statesGuard);

  for (unsigned int b = 0; b < gamepad.buttonCount && b < m_gamepad.buttonCount; b++)
  {
    // We only care if a change in state is detected
    if (gamepad.buttons[b] == m_gamepad.buttons[b])
      continue;

    // We only process button presses in WINDOW_FULLSCREEN_VIDEO. We check for
    // this instead of WINDOW_FULLSCREEN_GAME because FULLSCREEN_GAME is a thin
    // alias for FULLSCREEN_VIDEO, used only for translating keyboard and gamepad
    // events (this way, our translators don't have to query the active player
    // core). Later on, when translating, *then* we'll use WINDOW_FULLSCREEN_GAME.
    if (gamepad.buttons[b] && (window & WINDOW_ID_MASK) != WINDOW_FULLSCREEN_VIDEO)
      continue;

    // Record the new state
    m_gamepad.buttons[b] = gamepad.buttons[b];

    int        actionID;
    CStdString actionName;
    bool       fullrange; // unused
    // Actual button ID is b + 1
    if (!CButtonTranslator::GetInstance().TranslateJoystickString(WINDOW_FULLSCREEN_GAME,
        gamepad.name.c_str(), b + 1, JACTIVE_BUTTON, actionID, actionName, fullrange))
    {
      CLog::Log(LOGDEBUG, "RetroPlayerInput: Controller=%i, Gamepad %s untranslated button %s, action=%s, ID=%d",
        gamepad.id, gamepad.name.c_str(), gamepad.buttons[b] ? "press" : "unpress", actionName.c_str(), actionID);
      continue;
    }

    int id = TranslateActionID(actionID);
    if (0 <= id && id < (int)(sizeof(m_joypadState) / sizeof(m_joypadState[0])))
    {
      // Record the new joypad state
      m_joypadState[id] = gamepad.buttons[b];
      CLog::Log(LOGDEBUG, "RetroPlayerInput: Gamepad %s button %s, action=%s, ID=%d", gamepad.name.c_str(),
        gamepad.buttons[b] ? "press" : "unpress", actionName.c_str(), actionID);
    }
    else
    {
      CLog::Log(LOGDEBUG, "RetroPlayerInput: Gamepad %s invalid button %s, action=%s, ID=%d", gamepad.name.c_str(),
        gamepad.buttons[b] ? "press" : "unpress", actionName.c_str(), actionID);
    }
  }

  static const char *dir[] = {"UP", "RIGHT", "DOWN", "LEFT"};

  for (unsigned int h = 0; h < gamepad.hatCount && h < m_gamepad.hatCount; h++)
  {
    // We only care if a change in state is detected
    if (gamepad.hats[h] == m_gamepad.hats[h])
      continue;
    CLog::Log(LOGDEBUG, "RetroPlayerInput: Gamepad %s, new hat %d direction is %s",
      gamepad.name.c_str(), h, gamepad.hats[h].GetDirection());

    // Using ordinal directions instead of cardinal directions lets us use a for loop efficiently
    for (unsigned int i = 0; i < 4; i++)
    {
      if (gamepad.hats[h][i] == m_gamepad.hats[h][i])
        continue;

      // Don't record presses outside of fullscreen video (unpresses are ok)
      if (gamepad.hats[h][i] && (window & WINDOW_ID_MASK) != WINDOW_FULLSCREEN_VIDEO)
        continue;

      // Record the new state
      m_gamepad.hats[h][i] = gamepad.hats[h][i];

      // Compose button ID (SDL_HAT_UP is (1 << 0), SDL_HAT_RIGHT is (1 << 1), etc.)
      int        buttonID = (1 << i) << 16 | (h + 1); // Hat ID is h + 1
      int        actionID;
      CStdString actionName;
      bool       fullrange; // unused
      if (!CButtonTranslator::GetInstance().TranslateJoystickString(WINDOW_FULLSCREEN_GAME,
          gamepad.name.c_str(), buttonID, JACTIVE_HAT, actionID, actionName, fullrange))
      {
        CLog::Log(LOGDEBUG, "RetroPlayerInput: Controller=%i, Invalid hat %d %s %s", gamepad.id,
          h + 1, dir[i], gamepad.hats[h][i] ? "press" : "unpress");
        continue;
      }

      int id = TranslateActionID(actionID);
      if (0 <= id && id < (int)(sizeof(m_joypadState) / sizeof(m_joypadState[0])))
      {
        // Record the new joypad state
        m_joypadState[id] = gamepad.hats[h][i];
        CLog::Log(LOGDEBUG, "RetroPlayerInput: Hat %s %s, action=%s, ID=%d", dir[i],
          h + 1, dir[i], gamepad.hats[h][i] ? "press" : "unpress", actionName.c_str(), actionID);
      }
      else
      {
        CLog::Log(LOGDEBUG, "RetroPlayerInput: Invalid hat %s %s, action=%s, ID=%d", dir[i],
          h + 1, dir[i], gamepad.hats[h][i] ? "press" : "unpress", actionName.c_str(), actionID);
      }
    }
  }
}

int CRetroPlayerInput::TranslateActionID(int id) const
{
  int m_device = RETRO_DEVICE_JOYPAD; // Until we keep track of multiple devices

  // TODO: Perform better bounds-checking.
  switch (m_device)
  {
  case RETRO_DEVICE_JOYPAD:
    return id - ACTION_JOYBAD_B;
  case RETRO_DEVICE_MOUSE:
    return id - ACTION_MOUSE_CONTROLLER_X;
  case RETRO_DEVICE_LIGHTGUN:
    return id - ACTION_LIGHTGUN_X;
  case RETRO_DEVICE_ANALOG:
    return id - ACTION_ANALOG_CONTROLLER_LEFT;
  case RETRO_DEVICE_KEYBOARD:
    //return id - ACTION_JOYBAD_B; // Keyboard is trickier
  default:
    break;
  }
  return -1; // Invalid device ID
}
