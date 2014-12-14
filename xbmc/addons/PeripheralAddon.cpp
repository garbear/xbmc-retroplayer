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

#include "PeripheralAddon.h"
#include "filesystem/SpecialProtocol.h"
#include "peripherals/Peripherals.h"
#include "peripherals/bus/PeripheralBusAddon.h"
#include "utils/log.h"

#include <string.h>

using namespace ADDON;
using namespace PERIPHERALS;

#ifndef SAFE_DELETE
  #define SAFE_DELETE(p)  do { delete (p); (p) = NULL; } while (0)
#endif

CPeripheralAddon::CPeripheralAddon(const AddonProps& props)
 : CAddonDll<DllPeripheral, PeripheralAddon, PERIPHERAL_PROPERTIES>(props),
   m_apiVersion("0.0.0")
{
  ResetProperties();
}

CPeripheralAddon::CPeripheralAddon(const cp_extension_t *ext)
 : CAddonDll<DllPeripheral, PeripheralAddon, PERIPHERAL_PROPERTIES>(ext),
   m_apiVersion("0.0.0")
{
  ResetProperties();
}

CPeripheralAddon::~CPeripheralAddon(void)
{
  Destroy();
  SAFE_DELETE(m_pInfo);
}

void CPeripheralAddon::ResetProperties(void)
{
  /* initialise members */
  SAFE_DELETE(m_pInfo);
  m_pInfo = new PERIPHERAL_PROPERTIES;
  m_strUserPath        = CSpecialProtocol::TranslatePath(Profile());
  m_pInfo->user_path   = m_strUserPath.c_str();
  m_strClientPath      = CSpecialProtocol::TranslatePath(Path());
  m_pInfo->addon_path  = m_strClientPath.c_str();
  memset(&m_addonCapabilities, 0, sizeof(m_addonCapabilities));
  m_apiVersion = AddonVersion("0.0.0");
}

AddonPtr CPeripheralAddon::GetRunningInstance(void) const
{
  CPeripheralBusAddon* addonBus = static_cast<CPeripheralBusAddon*>(g_peripherals.GetBusByType(PERIPHERAL_BUS_ADDON));
  if (addonBus)
  {
    AddonPtr peripheralAddon;
    if (addonBus->GetAddon(ID(), peripheralAddon))
      return peripheralAddon;
  }
  return CAddon::GetRunningInstance();
}

ADDON_STATUS CPeripheralAddon::Create(void)
{
  ADDON_STATUS status(ADDON_STATUS_UNKNOWN);

  /* ensure that a previous instance is destroyed */
  Destroy();

  /* reset all properties to defaults */
  ResetProperties();

  /* initialise the add-on */
  CLog::Log(LOGDEBUG, "PERIPHERAL - %s - creating peripheral add-on instance '%s'", __FUNCTION__, Name().c_str());
  try { status = CAddonDll<DllPeripheral, PeripheralAddon, PERIPHERAL_PROPERTIES>::Create(); }
  catch (const std::exception &e) { LogException(e, __FUNCTION__); }

  GetAddonProperties();

  return status;
}

void CPeripheralAddon::Destroy(void)
{
  /* reset 'ready to use' to false */
  CLog::Log(LOGDEBUG, "PERIPHERAL - %s - destroying peripheral add-on '%s'", __FUNCTION__, Name().c_str());

  /* destroy the add-on */
  try { CAddonDll<DllPeripheral, PeripheralAddon, PERIPHERAL_PROPERTIES>::Destroy(); }
  catch (const std::exception &e) { LogException(e, __FUNCTION__); }

  /* reset all properties to defaults */
  ResetProperties();
}

bool CPeripheralAddon::GetAddonProperties(void)
{
  PERIPHERAL_CAPABILITIES addonCapabilities = { };

  /* get the capabilities */
  try
  {
    PERIPHERAL_ERROR retVal = m_pStruct->GetAddonCapabilities(&addonCapabilities);
    if (retVal != PERIPHERAL_NO_ERROR)
    {
      CLog::Log(LOGERROR, "PERIPHERAL - couldn't get the capabilities for add-on '%s'. Please contact the developer of this add-on: %s",
                Name().c_str(), Author().c_str());
      return false;
    }
  }
  catch (std::exception &e) { LogException(e, "GetAddonCapabilities()"); return false; }

  /* update the members */
  m_addonCapabilities   = addonCapabilities;

  return true;
}

PERIPHERAL_CAPABILITIES CPeripheralAddon::GetAddonCapabilities(void) const
{
  PERIPHERAL_CAPABILITIES addonCapabilities(m_addonCapabilities);
  return addonCapabilities;
}

bool CPeripheralAddon::CheckAPIVersion(void)
{
  /* check the API version */
  AddonVersion minVersion = AddonVersion(XBMC_PERIPHERAL_MIN_API_VERSION);
  try { m_apiVersion = AddonVersion(m_pStruct->GetPeripheralAPIVersion()); }
  catch (std::exception &e) { LogException(e, "GetPeripheralAPIVersion()"); return false;  }

  if (!IsCompatibleAPIVersion(minVersion, m_apiVersion))
  {
    CLog::Log(LOGERROR, "PERIPHERAL - Add-on '%s' is using an incompatible API version. XBMC minimum API version = '%s', add-on API version '%s'", Name().c_str(), minVersion.asString().c_str(), m_apiVersion.asString().c_str());
    return false;
  }

  return true;
}

bool CPeripheralAddon::IsCompatibleAPIVersion(const AddonVersion &minVersion, const AddonVersion &version)
{
  AddonVersion myMinVersion = AddonVersion(XBMC_PERIPHERAL_MIN_API_VERSION);
  AddonVersion myVersion = AddonVersion(XBMC_PERIPHERAL_API_VERSION);
  return (version >= myMinVersion && minVersion <= myVersion);
}

bool CPeripheralAddon::PerformJoystickScan(std::vector<AddonJoystick>& joysticks)
{
  unsigned int joystickCount;
  JOYSTICK* pJoysticks;

  PERIPHERAL_ERROR retVal;

  try { retVal = m_pStruct->PerformJoystickScan(&joystickCount, &pJoysticks); }
  catch (std::exception &e) { LogException(e, "PerformJoystickScan()"); return false;  }

  if (retVal == PERIPHERAL_NO_ERROR)
  {
    for (unsigned int i = 0; i < joystickCount; i++)
      joysticks.push_back(pJoysticks[i]);

    try { m_pStruct->FreeJoysticks(joystickCount, pJoysticks); }
    catch (std::exception &e) { LogException(e, "FreeJoysticks()"); }

    return true;
  }

  LogError(retVal, "PerformJoystickScan()");
  return false;
}

bool CPeripheralAddon::SetButton(unsigned int buttonIndex, JOYSTICK_ID_BUTTON newId, const std::string& strNewLabel)
{
  PERIPHERAL_ERROR retVal;

  try { LogError(retVal = m_pStruct->SetButton(buttonIndex, newId, strNewLabel.c_str()), "SetButton()"); }
  catch (std::exception &e) { LogException(e, "SetButton()"); return false; }

  return retVal == PERIPHERAL_NO_ERROR;
}

bool CPeripheralAddon::AddTrigger(const AddonTrigger& trigger)
{
  PERIPHERAL_ERROR retVal(PERIPHERAL_ERROR_FAILED);

  JOYSTICK_TRIGGER triggerStruct;
  trigger.ToTrigger(triggerStruct);

  try { LogError(retVal = m_pStruct->AddTrigger(&triggerStruct), "AddTrigger()"); }
  catch (std::exception &e) { LogException(e, "AddTrigger()"); }

  AddonTrigger::Free(triggerStruct);

  return retVal == PERIPHERAL_NO_ERROR;
}

bool CPeripheralAddon::RemoveTrigger(unsigned int triggerIndex)
{
  PERIPHERAL_ERROR retVal(PERIPHERAL_ERROR_FAILED);

  try { LogError(retVal = m_pStruct->RemoveTrigger(triggerIndex), "RemoveTrigger()"); }
  catch (std::exception &e) { LogException(e, "RemoveTrigger()"); }

  return retVal == PERIPHERAL_NO_ERROR;
}

bool CPeripheralAddon::AddAnalogStick(const AddonAnalogStick& analogStick)
{
  PERIPHERAL_ERROR retVal(PERIPHERAL_ERROR_FAILED);

  JOYSTICK_ANALOG_STICK analogStickStruct;
  analogStick.ToAnalogStick(analogStickStruct);

  try { LogError(retVal = m_pStruct->AddAnalogStick(&analogStickStruct), "AddTrigger()"); }
  catch (std::exception &e) { LogException(e, "AddTrigger()"); }

  AddonAnalogStick::Free(analogStickStruct);

  return retVal == PERIPHERAL_NO_ERROR;
}

bool CPeripheralAddon::RemoveAnalogStick(unsigned int analogStickIndex)
{
  PERIPHERAL_ERROR retVal(PERIPHERAL_ERROR_FAILED);

  try { LogError(retVal = m_pStruct->RemoveAnalogStick(analogStickIndex), "RemoveAnalogStick()"); }
  catch (std::exception &e) { LogException(e, "RemoveAnalogStick()"); }

  return retVal == PERIPHERAL_NO_ERROR;
}

const char *CPeripheralAddon::ToString(const PERIPHERAL_ERROR error)
{
  switch (error)
  {
  case PERIPHERAL_NO_ERROR:
    return "no error";
  case PERIPHERAL_ERROR_FAILED:
    return "command failed";
  case PERIPHERAL_ERROR_INVALID_PARAMETERS:
    return "invalid parameters";
  case PERIPHERAL_ERROR_NOT_IMPLEMENTED:
    return "not implemented";
  case PERIPHERAL_ERROR_NOT_CONNECTED:
    return "not connected";
  case PERIPHERAL_ERROR_CONNECTION_FAILED:
    return "connection failed";
  case PERIPHERAL_ERROR_UNKNOWN:
  default:
    return "unknown error";
  }
}

bool CPeripheralAddon::LogError(const PERIPHERAL_ERROR error, const char *strMethod) const
{
  if (error != PERIPHERAL_NO_ERROR)
  {
    CLog::Log(LOGERROR, "PERIPHERAL - %s - addon '%s' returned an error: %s",
        strMethod, Name().c_str(), ToString(error));
    return false;
  }
  return true;
}

void CPeripheralAddon::LogException(const std::exception &e, const char *strFunctionName) const
{
  CLog::Log(LOGERROR, "PERIPHERAL - exception '%s' caught while trying to call '%s' on add-on '%s'. Please contact the developer of this add-on: %s",
            e.what(), strFunctionName, Name().c_str(), Author().c_str());
}
