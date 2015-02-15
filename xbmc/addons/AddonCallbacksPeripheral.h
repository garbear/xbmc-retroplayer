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

struct CB_PeripheralLib;

namespace PERIPHERALS { class CPeripheralAddon; }

namespace ADDON
{

class CAddon;

/*!
 * Callbacks for a peripheral add-on to XBMC
 */
class CAddonCallbacksPeripheral
{
public:
  CAddonCallbacksPeripheral(CAddon* addon);
  ~CAddonCallbacksPeripheral(void);

  /*!
   * @return The callback table
   */
  CB_PeripheralLib* GetCallbacks() const { return m_callbacks; }

  static void TriggerScan(void* addonData);

private:
  static PERIPHERALS::CPeripheralAddon* GetPeripheralAddon(void* addonData, const char* strFunction);

  CB_PeripheralLib*  m_callbacks; /*!< callback addresses */
  CAddon*            m_addon;     /*!< the add-on */
};

}; /* namespace ADDON */
