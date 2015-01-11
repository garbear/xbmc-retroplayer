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

#include "AddonJoystickButtonMap.h"
#include "peripherals/Peripherals.h"

using namespace PERIPHERALS;

CAddonJoystickButtonMap::CAddonJoystickButtonMap(CPeripheral* device, const std::string& strControllerId)
  : m_addon(g_peripherals.GetAddon(device)),
    m_buttonMapRO(device, m_addon, strControllerId),
    m_buttonMapWO(device, m_addon, strControllerId)
{
  if (m_addon)
    m_addon->RegisterButtonMap(device, this);
}

CAddonJoystickButtonMap::~CAddonJoystickButtonMap(void)
{
  if (m_addon)
    m_addon->UnregisterButtonMap(this);
}

bool CAddonJoystickButtonMap::Load(void)
{
  return m_buttonMapRO.Load() && m_buttonMapWO.Load();
}

bool CAddonJoystickButtonMap::GetFeature(const CJoystickDriverPrimitive& primitive, std::string& feature)
{
  return m_buttonMapRO.GetFeature(primitive, feature);
}

bool CAddonJoystickButtonMap::GetButton(const std::string& feature, CJoystickDriverPrimitive& button)
{
  return m_buttonMapRO.GetButton(feature, button);
}

bool CAddonJoystickButtonMap::MapButton(const std::string& feature, const CJoystickDriverPrimitive& primitive)
{
  return m_buttonMapWO.MapButton(feature, primitive);
}

bool CAddonJoystickButtonMap::GetAnalogStick(const std::string& feature,
                                             int& horizIndex, bool& horizInverted,
                                             int& vertIndex,  bool& vertInverted)
{
  return m_buttonMapRO.GetAnalogStick(feature, horizIndex, horizInverted,
                                               vertIndex, vertInverted);
}

bool CAddonJoystickButtonMap::MapAnalogStick(const std::string& feature,
                                             int horizIndex, bool horizInverted,
                                             int vertIndex,  bool vertInverted)
{
  return m_buttonMapWO.MapAnalogStick(feature, horizIndex, horizInverted,
                                               vertIndex, vertInverted);
}

bool CAddonJoystickButtonMap::GetAccelerometer(const std::string& feature,
                                               int& xIndex, bool& xInverted,
                                               int& yIndex, bool& yInverted,
                                               int& zIndex, bool& zInverted)
{
  return m_buttonMapRO.GetAccelerometer(feature, xIndex, xInverted,
                                                 yIndex, yInverted,
                                                 zIndex, zInverted);
}

bool CAddonJoystickButtonMap::MapAccelerometer(const std::string& feature,
                                               int xIndex, bool xInverted,
                                               int yIndex, bool yInverted,
                                               int zIndex, bool zInverted)
{
  return m_buttonMapWO.MapAccelerometer(feature, xIndex, xInverted,
                                                 yIndex, yInverted,
                                                 zIndex, zInverted);
}
