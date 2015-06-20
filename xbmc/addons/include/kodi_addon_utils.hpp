#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
 *      http://www.xbmc.org
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

/*!
 * @file xbmc_addon_utils.hpp  C++ wrappers for add-on callbacks
 */

#include "libXBMC_addon.h"

#include <string>

class AddonUtils
{
public:
  /*!
   * @brief Get a localized message
   * @param XBMC       The add-on callback helper
   * @param dwCode     The code of the message to get
   * @param strDefault The default message, also helps to identify the code that is used
   * @return The localized message, or strDefault if the add-on helper fails to return a message
   */
  static std::string GetLocalizedString(ADDON::CHelper_libXBMC_addon* XBMC, int dwCode, const std::string& strDefault = "")
  {
    std::string strReturn(strDefault);
    if (XBMC)
    {
      char* strMsg = XBMC->GetLocalizedString(dwCode);
      // XBMC->GetLocalizedString() will return an empty string on invalid ID (such as 31000)
      // If the default is non-empty, we were probably expecting a non-empty message, so
      // return the default in this case
      if (strMsg)
      {
        if (strlen(strMsg))
          strReturn = strMsg;
        XBMC->FreeString(strMsg);
      }
    }
    return strReturn;
  }
};
