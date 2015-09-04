/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include "Addon.h"
#include "AddonCallbacks.h"
#include "AddonCallbacksAddon.h"
#include "AddonCallbacksCodec.h"
#include "AddonCallbacksGUI.h"
#include "AddonCallbacksInterfaces.h"
#include "AddonCallbacksPeripheral.h"
#include "AddonCallbacksPVR.h"
#include "AddonCallbacksGame.h"
#include "filesystem/SpecialProtocol.h"
#include "utils/log.h"

namespace ADDON
{

CAddonCallbacks::CAddonCallbacks(CAddon* addon)
{
  m_addon       = addon;
  m_callbacks   = new AddonCB;
  m_helperAddon = NULL;
  m_helperGUI   = NULL;
  m_helperInterfaces  = NULL;
  m_helperPeripheral = NULL;
  m_helperPVR   = NULL;
  m_helperCODEC = NULL;
  m_helperGame  = NULL;

  m_callbacks->libBasePath           = strdup(CSpecialProtocol::TranslatePath("special://xbmcbin/addons").c_str());
  m_callbacks->addonData             = this;
  m_callbacks->AddOnLib_RegisterMe   = CAddonCallbacks::AddOnLib_RegisterMe;
  m_callbacks->AddOnLib_UnRegisterMe = CAddonCallbacks::AddOnLib_UnRegisterMe;
  m_callbacks->CODECLib_RegisterMe   = CAddonCallbacks::CODECLib_RegisterMe;
  m_callbacks->CODECLib_UnRegisterMe = CAddonCallbacks::CODECLib_UnRegisterMe;
  m_callbacks->GUILib_RegisterMe     = CAddonCallbacks::GUILib_RegisterMe;
  m_callbacks->GUILib_UnRegisterMe   = CAddonCallbacks::GUILib_UnRegisterMe;
  m_callbacks->InterfacesLib_RegisterMe     = CAddonCallbacks::InterfacesLib_RegisterMe;
  m_callbacks->InterfacesLib_UnRegisterMe   = CAddonCallbacks::InterfacesLib_UnRegisterMe;
  m_callbacks->PeripheralLib_RegisterMe   = CAddonCallbacks::PeripheralLib_RegisterMe;
  m_callbacks->PeripheralLib_UnRegisterMe = CAddonCallbacks::PeripheralLib_UnRegisterMe;
  m_callbacks->PVRLib_RegisterMe     = CAddonCallbacks::PVRLib_RegisterMe;
  m_callbacks->PVRLib_UnRegisterMe   = CAddonCallbacks::PVRLib_UnRegisterMe;
  m_callbacks->GameLib_RegisterMe    = CAddonCallbacks::GameLib_RegisterMe;
  m_callbacks->GameLib_UnRegisterMe  = CAddonCallbacks::GameLib_UnRegisterMe;
}

CAddonCallbacks::~CAddonCallbacks()
{
  delete m_helperAddon;
  m_helperAddon = NULL;
  delete m_helperCODEC;
  m_helperCODEC = NULL;
  delete m_helperGUI;
  m_helperGUI = NULL;
  delete m_helperPeripheral;
  m_helperPeripheral = NULL;
  delete m_helperPVR;
  delete m_helperInterfaces;
  m_helperInterfaces = NULL;
  m_helperPVR = NULL;
  delete m_helperGame;
  m_helperGame = NULL;
  free((char*)m_callbacks->libBasePath);
  delete m_callbacks;
  m_callbacks = NULL;
}

CB_AddOnLib* CAddonCallbacks::AddOnLib_RegisterMe(void *addonData)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if (addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return NULL;
  }

  addon->m_helperAddon = new CAddonCallbacksAddon(addon->m_addon);
  return addon->m_helperAddon->GetCallbacks();
}

void CAddonCallbacks::AddOnLib_UnRegisterMe(void *addonData, CB_AddOnLib *cbTable)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if (addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return;
  }

  delete addon->m_helperAddon;
  addon->m_helperAddon = NULL;
}

CB_CODECLib* CAddonCallbacks::CODECLib_RegisterMe(void *addonData)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if (addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return NULL;
  }

  addon->m_helperCODEC = new CAddonCallbacksCodec(addon->m_addon);
  return addon->m_helperCODEC->GetCallbacks();
}

void CAddonCallbacks::CODECLib_UnRegisterMe(void *addonData, CB_CODECLib *cbTable)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if (addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return;
  }

  delete addon->m_helperCODEC;
  addon->m_helperCODEC = NULL;
}

CB_GUILib* CAddonCallbacks::GUILib_RegisterMe(void *addonData)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if (addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return NULL;
  }

  addon->m_helperGUI = new CAddonCallbacksGUI(addon->m_addon);
  return addon->m_helperGUI->GetCallbacks();
}

void CAddonCallbacks::GUILib_UnRegisterMe(void *addonData, CB_GUILib *cbTable)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if (addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return;
  }

  delete addon->m_helperGUI;
  addon->m_helperGUI = NULL;
}

CB_PeripheralLib* CAddonCallbacks::PeripheralLib_RegisterMe(void *addonData)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if (addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return NULL;
  }

  addon->m_helperPeripheral = new CAddonCallbacksPeripheral(addon->m_addon);
  return addon->m_helperPeripheral->GetCallbacks();
}

void CAddonCallbacks::PeripheralLib_UnRegisterMe(void *addonData, CB_PeripheralLib *cbTable)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if (addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return;
  }

  delete addon->m_helperPeripheral;
  addon->m_helperPeripheral = NULL;
}

CB_InterfacesLib* CAddonCallbacks::InterfacesLib_RegisterMe(void *addonData)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if(addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return NULL;
  }

  addon->m_helperInterfaces = new CAddonCallbacksInterfaces(addon->m_addon);
  return addon->m_helperInterfaces->GetCallbacks();
}

void CAddonCallbacks::InterfacesLib_UnRegisterMe(void *addonData, CB_InterfacesLib *cbTable)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if(addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return;
  }

  delete addon->m_helperInterfaces;
  addon->m_helperInterfaces = NULL;
}

CB_PVRLib* CAddonCallbacks::PVRLib_RegisterMe(void *addonData)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if (addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return NULL;
  }

  addon->m_helperPVR = new CAddonCallbacksPVR(addon->m_addon);
  return addon->m_helperPVR->GetCallbacks();
}

void CAddonCallbacks::PVRLib_UnRegisterMe(void *addonData, CB_PVRLib *cbTable)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if (addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return;
  }

  delete addon->m_helperPVR;
  addon->m_helperPVR = NULL;
}

CB_GameLib* CAddonCallbacks::GameLib_RegisterMe(void *addonData)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if (addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return NULL;
  }

  addon->m_helperGame = new CAddonCallbacksGame(addon->m_addon);
  return addon->m_helperGame->GetCallbacks();
}

void CAddonCallbacks::GameLib_UnRegisterMe(void *addonData, CB_GameLib *cbTable)
{
  CAddonCallbacks* addon = (CAddonCallbacks*) addonData;
  if (addon == NULL)
  {
    CLog::Log(LOGERROR, "CAddonCallbacks - %s - called with a null pointer", __FUNCTION__);
    return;
  }

  delete addon->m_helperGame;
  addon->m_helperGame = NULL;
}

}; /* namespace ADDON */
