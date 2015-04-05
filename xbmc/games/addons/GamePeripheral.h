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

#include "GamePeripheralLayout.h"
#include "addons/Addon.h"

#include <string>

namespace GAME
{

class CGamePeripheral : public ADDON::CAddon
{
public:
  CGamePeripheral(const ADDON::AddonProps &addonprops);
  CGamePeripheral(const cp_extension_t *ext);
  virtual ~CGamePeripheral(void) { }

  static const GamePeripheralPtr EmptyPtr;

  std::string Label(void);
  std::string ImagePath(void) const;
  std::string OverlayPath(void) const;

  bool LoadLayout(void);

  const CGamePeripheralLayout& Layout(void) const { return m_layout; }

private:
  CGamePeripheralLayout m_layout;
};

}
