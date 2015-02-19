/*
 *      Copyright (C) 2014 Team XBMC
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

class CGUIGamePeripheral : public CGUIImage
{
public:
  CGUIGamePeripheral(int parentID, int controlID, float posX, float posY, float width, float height);
  CGUIGamePeripheral(const CGUIGamePeripheral &from);
  virtual ~CGUIGamePeripheral(void) { }
  virtual CGUIGamePeripheral *Clone() const { return new CGUIGamePeripheral(*this); }

  virtual void Render();

  void ActivatePeripheral(const GAME::GamePeripheralPtr& peripheral);

private:
  GAME::GamePeripheralPtr m_currentPeripheral;
  CCriticalSection        m_mutex;
};
