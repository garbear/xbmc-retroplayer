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

#include "JoystickEasterEgg.h"
#include "utils/Variant.h"
#include "Application.h"
#include "FileItem.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)  (sizeof(x) / sizeof(x[0]))
#endif

using namespace JOYSTICK;

const char* CJoystickEasterEgg::m_sequence[] = {
  "up",
  "up",
  "down",
  "down",
  "left",
  "right",
  "left",
  "right",
  "b",
  "a",
};

CJoystickEasterEgg::CJoystickEasterEgg(void)
  : m_state(0)
{
}

bool CJoystickEasterEgg::OnButtonPress(const JoystickFeature& feature)
{
  bool bHandled = false;

  // Update state
  if (feature == m_sequence[m_state])
    m_state++;
  else
    m_state = 0;

  // Capture input when finished with arrows (2 x up/down/left/right)
  if (m_state > 8)
  {
    bHandled = true;

    if (m_state >= ARRAY_SIZE(m_sequence))
    {
      OnFinish();
      m_state = 0;
    }
  }

  return bHandled;
}

void CJoystickEasterEgg::OnFinish(void)
{
  CFileItem item("", false);
  item.SetProperty("Addon.ID", "game.libretro.2048");
  g_application.PlayMedia(item);
}
