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

#include "AddonJoystickButtonMapper.h"

CAddonJoystickButtonMapper::CAddonJoystickButtonMapper(IJoystickActionHandler *handler, const JoystickIdentifier& identifier, const PERIPHERALS::PeripheralAddonPtr& addon)
  : IGenericJoystickButtonMapper(handler, identifier),
    m_addon(addon)
{

}
bool CAddonJoystickButtonMapper::OnRawButtonPress(unsigned int id, bool bPressed)
{
  return false; // TODO
}

bool CAddonJoystickButtonMapper::OnRawHatMotion(unsigned int id, HatDirection direction)
{
  return false; // TODO
}

bool CAddonJoystickButtonMapper::OnRawAxisMotion(unsigned int id, float position)
{
  return false; // TODO
}
