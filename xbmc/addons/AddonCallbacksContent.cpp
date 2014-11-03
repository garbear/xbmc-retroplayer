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

#include "Application.h"
#include "Addon.h"
#include "AddonCallbacksContent.h"
#include "utils/log.h"
#include "ContentAddon.h"

namespace ADDON
{

CAddonCallbacksContent::CAddonCallbacksContent(CAddon* addon)
{
  m_addon     = addon;
  m_callbacks = new CB_ContentLib;

  m_callbacks->SetPlaystate = CONTENTSetPlaystate;
}

CAddonCallbacksContent::~CAddonCallbacksContent()
{
  /* delete the callback table */
  delete m_callbacks;
}

CContentAddon* CAddonCallbacksContent::GetAddon(void *addonData)
{
  CAddonCallbacks *addon = static_cast<CAddonCallbacks *>(addonData);
  if (!addon || !addon->GetHelperContent())
  {
    CLog::Log(LOGERROR, "%s - called with a null pointer", __FUNCTION__);
    return NULL;
  }

  return dynamic_cast<CContentAddon *>(addon->GetHelperContent()->m_addon);
}

void CAddonCallbacksContent::CONTENTSetPlaystate(void* addonData, CONTENT_ADDON_PLAYSTATE newState)
{
  CContentAddon* addon = GetAddon(addonData);
  if (addon)
    addon->SetPlaystate(newState);
}

}; /* namespace ADDON */
