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

#include "PeripheralBusApplication.h"

using namespace PERIPHERALS;

CPeripheralBusApplication::CPeripheralBusApplication(CPeripherals* manager) :
    CPeripheralBus("PeripBusApplication", manager, PERIPHERAL_BUS_APPLICATION)
{
}

bool CPeripheralBusApplication::PerformDeviceScan(PeripheralScanResults& results)
{
  PeripheralScanResult result(Type());
  result.m_type = PERIPHERAL_KEYBOARD;
  result.m_strDeviceName = "Keyboard"; // TODO
  result.m_strLocation   = "keyboard"; // TODO
  result.m_iVendorId     = 0;
  result.m_iProductId    = 0;
  result.m_mappedType    = PERIPHERAL_KEYBOARD;
  result.m_mappedBusType = Type();
  result.m_iSequence     = 0;

  if (!results.ContainsResult(result))
    results.m_results.push_back(result);

  return true;
}
