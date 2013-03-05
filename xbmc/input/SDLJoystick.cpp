/*
*      Copyright (C) 2007-2013 Team XBMC
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
*  along with XBMC; see the file COPYING.  If not, see
*  <http://www.gnu.org/licenses/>.
*
*/

#include "SDLJoystick.h"
#include "utils/log.h"

#include <SDL/SDL.h>
#include <SDL/SDL_events.h>

#define MAX_AXES          64
#define MAX_AXISAMOUNT    32768


CJoystickSDL::CJoystickSDL(std::string name, SDL_Joystick *pJoystick, unsigned int id) : m_pJoystick(pJoystick), m_state()
{
  m_state.id          = id;
  m_state.name        = name;
  m_state.buttonCount = std::min(m_state.buttonCount, (unsigned int)SDL_JoystickNumButtons(m_pJoystick));
  m_state.hatCount    = std::min(m_state.hatCount, (unsigned int)SDL_JoystickNumButtons(m_pJoystick));
  m_state.axisCount   = std::min(m_state.axisCount, (unsigned int)SDL_JoystickNumButtons(m_pJoystick));

  CLog::Log(LOGNOTICE, "Enabled Joystick: \"%s\" (SDL)", name.c_str());
  CLog::Log(LOGNOTICE, "Details: Total Axes: %u Total Hats: %u Total Buttons: %u",
    m_state.axisCount, m_state.hatCount, m_state.buttonCount);
}

/* static */
void CJoystickSDL::Initialize(JoystickArray &joysticks)
{
  DeInitialize(joysticks);

  if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0)
  {
    CLog::Log(LOGERROR, "(Re)start joystick subsystem failed : %s", SDL_GetError());
    return;
  }

  // Any joysticks connected?
  if (SDL_NumJoysticks() > 0)
  {
    // Load joystick names and open all connected joysticks
    for (int i = 0 ; i < SDL_NumJoysticks(); i++)
    {
      SDL_Joystick *joy = SDL_JoystickOpen(i);
#if defined(TARGET_DARWIN)
      // On OS X, the 360 controllers are handled externally, since the SDL code is
      // really buggy and doesn't handle disconnects.
      if (std::string(SDL_JoystickName(i)).find("360") != std::string::npos)
      {
        CLog::Log(LOGNOTICE, "Ignoring joystick: %s", SDL_JoystickName(i));
        continue;
      }
#endif
      if (joy)
      {
        joysticks.push_back(boost::shared_ptr<IJoystick>(new CJoystickSDL(SDL_JoystickName(i),
            joy, joysticks.size())));
      }
    }
  }

  // Disable joystick events, since we'll be polling them
  SDL_JoystickEventState(SDL_DISABLE);
}

/* static */
void CJoystickSDL::DeInitialize(JoystickArray &joysticks)
{
  for (int i = 0; i < (int)joysticks.size(); i++)
  {
    if (boost::dynamic_pointer_cast<CJoystickSDL>(joysticks[i]))
      joysticks.erase(joysticks.begin() + i--);
  }
  // Restart SDL joystick subsystem
  SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
  if (SDL_WasInit(SDL_INIT_JOYSTICK) !=  0)
    CLog::Log(LOGERROR, "Stopping joystick SDL subsystem failed");
}

void CJoystickSDL::Update()
{
  // Update the state of all opened joysticks
  SDL_JoystickUpdate();

  // Gamepad buttons
  for (unsigned int b = 0; b < m_state.buttonCount; b++)
    m_state.buttons[b] = (SDL_JoystickGetButton(m_pJoystick, b) ? 1 : 0);

  // Gamepad hats
  for (unsigned int h = 0; h < m_state.hatCount; h++)
  {
    m_state.hats[h].Center();
    uint8_t hat = SDL_JoystickGetHat(m_pJoystick, h);
    if      (hat & SDL_HAT_UP)    m_state.hats[h].up = 1;
    else if (hat & SDL_HAT_DOWN)  m_state.hats[h].down = 1;
    if      (hat & SDL_HAT_RIGHT) m_state.hats[h].right = 1;
    else if (hat & SDL_HAT_LEFT)  m_state.hats[h].left = 1;
  }

  // Gamepad axes
  for (unsigned int a = 0; a < m_state.axisCount; a++)
    m_state.axes[a] = NormalizeAxis((long)SDL_JoystickGetAxis(m_pJoystick, a), MAX_AXISAMOUNT);
}

void CJoystickSDL::Update(SDL_Event *joyEvent)
{
  switch(joyEvent->type)
  {
  case SDL_JOYBUTTONDOWN:
    {
      unsigned char buttonId = joyEvent->jbutton.button;
      if (buttonId < m_state.buttonCount)
      {
        m_state.buttons[buttonId] = 1;
        CLog::Log(LOGDEBUG, "Joystick %u button %u down", m_state.id, buttonId + 1);
      }
      else
      {
        CLog::Log(LOGERROR, "Joystick %u invalid button %u down, joystick only supports %u buttons",
            m_state.id, buttonId + 1, m_state.buttonCount);
      }
    }
    break;

  case SDL_JOYBUTTONUP:
    {
      unsigned char buttonId = joyEvent->jbutton.button;
      if (buttonId < m_state.buttonCount)
      {
        m_state.buttons[buttonId] = 0;
        CLog::Log(LOGDEBUG, "Joystick %u button %u up", m_state.id, buttonId);
      }
      else
      {
        CLog::Log(LOGERROR, "Joystick %u invalid button %u up, joystick only supports %u buttons",
            m_state.id, buttonId + 1, m_state.buttonCount);
      }
    }
    break;

  case SDL_JOYHATMOTION:
    {
      unsigned int hatId = joyEvent->jhat.hat;
      if (hatId < m_state.hatCount)
      {
        m_state.hats[hatId].Center();
        if      (joyEvent->jhat.value & SDL_HAT_UP)    m_state.hats[hatId].up = 1;
        else if (joyEvent->jhat.value & SDL_HAT_DOWN)  m_state.hats[hatId].down = 1;
        if      (joyEvent->jhat.value & SDL_HAT_RIGHT) m_state.hats[hatId].right = 1;
        else if (joyEvent->jhat.value & SDL_HAT_LEFT)  m_state.hats[hatId].left = 1;
        CLog::Log(LOGDEBUG, "Joystick %u hat %u new direction %s", m_state.id, hatId, m_state.hats[hatId].GetDirection());
      }
      else
        CLog::Log(LOGERROR, "Joystick %u invalid hat %u, joystick only supports %u hats", m_state.id, hatId, m_state.hatCount);
    }
    break;

  case SDL_JOYAXISMOTION:
    {
      unsigned int axisId = joyEvent->jaxis.axis;
      if (axisId < m_state.axisCount)
      {
        m_state.axes[axisId] = NormalizeAxis((long)joyEvent->jaxis.value, MAX_AXISAMOUNT);
        CLog::Log(LOGDEBUG, "Joystick %u axis %u amount %f", m_state.id, axisId, m_state.axes[axisId]);
      }
      else
      {
        CLog::Log(LOGERROR, "Joystick %u invalid axis %u, joystick only supports %u axes", m_state.id, axisId, m_state.axisCount);
      }
    }
    break;

  case SDL_JOYBALLMOTION:
  default:
    break;
  }
}
