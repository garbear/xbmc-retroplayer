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
#pragma once

#include "GameControllerLayout.h"
#include "addons/Addon.h"
#include "games/GameTypes.h"

#include <string>

namespace GAME
{

class CGameController : public ADDON::CAddon
{
public:
  CGameController(const ADDON::AddonProps &addonprops);
  CGameController(const cp_extension_t *ext);
  virtual ~CGameController(void) { }

  static const GameControllerPtr EmptyPtr;

  // implementation of CAddon
  virtual ADDON::AddonPtr GetRunningInstance(void) const;

  std::string Label(void);
  std::string ImagePath(void) const;
  std::string OverlayPath(void) const;

  bool LoadLayout(void);

  const CGameControllerLayout& Layout(void) const { return m_layout; }

private:
  CGameControllerLayout m_layout;
  bool                  m_bLoaded;
};

}
