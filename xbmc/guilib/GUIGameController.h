/*
 *      Copyright (C) 2014-2015 Team XBMC
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
#pragma once

#include "GUIImage.h"
#include "games/GameTypes.h"
#include "threads/CriticalSection.h"

class CGUIGameController : public CGUIImage
{
public:
  CGUIGameController(int parentID, int controlID, float posX, float posY, float width, float height);
  CGUIGameController(const CGUIGameController &from);
  virtual ~CGUIGameController(void) { }
  virtual CGUIGameController *Clone() const { return new CGUIGameController(*this); }

  virtual void Render();

  void ActivateController(const GAME::GameControllerPtr& controller);

private:
  GAME::GameControllerPtr m_currentController;
  CCriticalSection        m_mutex;
};
