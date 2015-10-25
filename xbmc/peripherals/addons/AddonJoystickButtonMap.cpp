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

bool CAddonJoystickButtonMap::GetFeature(const CDriverPrimitive& primitive, JoystickFeature& feature)
{
  return m_buttonMapRO.GetFeature(primitive, feature);
}

bool CAddonJoystickButtonMap::GetPrimitiveFeature(const JoystickFeature& feature, CDriverPrimitive& primitive)
{
  return m_buttonMapRO.GetPrimitiveFeature(feature, primitive);
}

bool CAddonJoystickButtonMap::AddPrimitiveFeature(const JoystickFeature& feature, const CDriverPrimitive& primitive)
{
  return m_buttonMapWO.AddPrimitiveFeature(feature, primitive);
}

bool CAddonJoystickButtonMap::GetAnalogStick(const JoystickFeature& feature,
                                             CDriverPrimitive& up,
                                             CDriverPrimitive& down,
                                             CDriverPrimitive& right,
                                             CDriverPrimitive& left)
{
  return m_buttonMapRO.GetAnalogStick(feature, up, down, right, left);
}

bool CAddonJoystickButtonMap::AddAnalogStick(const JoystickFeature& feature,
                                             const CDriverPrimitive& up,
                                             const CDriverPrimitive& down,
                                             const CDriverPrimitive& right,
                                             const CDriverPrimitive& left)
{
  return m_buttonMapWO.AddAnalogStick(feature, up, down, right, left);
}

bool CAddonJoystickButtonMap::GetAccelerometer(const JoystickFeature& feature,
                                               CDriverPrimitive& positiveX,
                                               CDriverPrimitive& positiveY,
                                               CDriverPrimitive& positiveZ)
{
  return m_buttonMapRO.GetAccelerometer(feature, positiveX, positiveY, positiveZ);
}

bool CAddonJoystickButtonMap::AddAccelerometer(const JoystickFeature& feature,
                                               const CDriverPrimitive& positiveX,
                                               const CDriverPrimitive& positiveY,
                                               const CDriverPrimitive& positiveZ)
{
  return m_buttonMapWO.AddAccelerometer(feature, positiveX, positiveY, positiveZ);
}
