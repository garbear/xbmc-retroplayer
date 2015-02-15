/*
 *      Copyright (C) 2005-2013 Team XBMC
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
#include "LanguageHook.h"

#include "addons/AddonManager.h"
#include "addons/GUIDialogAddonSettings.h"
#include "guilib/GUIWindowManager.h"
#include "GUIUserMessages.h"
#include "utils/StringUtils.h"

using namespace ADDON;

namespace XBMCAddon
{
  namespace xbmcaddon
  {
    String Addon::getDefaultId() { return languageHook == NULL ? emptyString : languageHook->GetAddonId(); }

    String Addon::getAddonVersion() { return languageHook == NULL ? emptyString : languageHook->GetAddonVersion(); }

    Addon::Addon(const char* cid, bool installed) throw (AddonException)
    {
      String id(cid ? cid : emptyString);

      // if the id wasn't passed then get the id from
      //   the global dictionary
      if (id.empty())
        id = getDefaultId();

      // if we still don't have an id then bail
      if (id.empty())
        throw AddonException("No valid addon id could be obtained. None was passed and the script wasn't executed in a normal xbmc manner.");

      bool success = ADDON::CAddonMgr::Get().GetAddon(id.c_str(), pAddon);
      if (!success && !installed)
      {
        CAddonDatabase addondb;
        addondb.Open();
        success = addondb.GetAddon(id, pAddon);
      }

      // if we still fail we MAY be able to recover.
      if (!success)
      {
        // we need to check the version prior to trying a bw compatibility trick
        ADDON::AddonVersion version(getAddonVersion());
        ADDON::AddonVersion allowable("1.0");

        if (version <= allowable)
        {
          // try the default ...
          id = getDefaultId();

          if (id.empty() || !ADDON::CAddonMgr::Get().GetAddon(id.c_str(), pAddon))
            throw AddonException("Could not get AddonPtr!");
          else
            CLog::Log(LOGERROR,"Use of deprecated functionality. Please to not assume that \"os.getcwd\" will return the script directory.");
        }
        else
        {
          throw AddonException("Could not get AddonPtr given a script id of %s."
                               "If you are trying to use 'os.getcwd' to set the path, you cannot do that in a version %s plugin.", 
                               id.c_str(), version.asString().c_str());
        }
      }

      CAddonMgr::Get().AddToUpdateableAddons(pAddon);
    }

    Addon::~Addon()
    {
      CAddonMgr::Get().RemoveFromUpdateableAddons(pAddon);
    }

    String Addon::getLocalizedString(int id)
    {
      AddonPtr temp;
      if (!ADDON::CAddonMgr::Get().GetAddon(pAddon->ID(), temp))
        return ""; // Can't get a string for an addon that isn't installed
      return pAddon->GetString(id);
    }

    String Addon::getSetting(const char* id)
    {
      AddonPtr temp;
      if (!ADDON::CAddonMgr::Get().GetAddon(pAddon->ID(), temp))
        return "";
      return pAddon->GetSetting(id);
    }

    void Addon::setSetting(const char* id, const String& value)
    {
      AddonPtr temp;
      if (!ADDON::CAddonMgr::Get().GetAddon(pAddon->ID(), temp))
        return;

      DelayedCallGuard dcguard(languageHook);
      ADDON::AddonPtr addon(pAddon);
      bool save=true;
      if (g_windowManager.IsWindowActive(WINDOW_DIALOG_ADDON_SETTINGS))
      {
        CGUIDialogAddonSettings* dialog = (CGUIDialogAddonSettings*)g_windowManager.GetWindow(WINDOW_DIALOG_ADDON_SETTINGS);
        if (dialog->GetCurrentID() == addon->ID())
        {
          CGUIMessage message(GUI_MSG_SETTING_UPDATED,0,0);
          std::vector<std::string> params;
          params.push_back(id);
          params.push_back(value);
          message.SetStringParams(params);
          g_windowManager.SendThreadMessage(message,WINDOW_DIALOG_ADDON_SETTINGS);
          save=false;
        }
      }
      if (save)
      {
        addon->UpdateSetting(id, value);
        addon->SaveSettings();
      }
    }

    void Addon::openSettings()
    {
      AddonPtr temp;
      if (!ADDON::CAddonMgr::Get().GetAddon(pAddon->ID(), temp))
        return;

      DelayedCallGuard dcguard(languageHook);
      // show settings dialog
      ADDON::AddonPtr addon(pAddon);
      CGUIDialogAddonSettings::ShowAndGetInput(addon);
    }

    String Addon::getAddonInfo(const char* id) throw (AddonException)
    {
      if (strcmpi(id, "author") == 0)
        return pAddon->Author();
      else if (strcmpi(id, "changelog") == 0)
        return pAddon->ChangeLog();
      else if (strcmpi(id, "description") == 0)
        return pAddon->Description();
      else if (strcmpi(id, "disclaimer") == 0)
        return pAddon->Disclaimer();
      else if (strcmpi(id, "fanart") == 0)
        return pAddon->FanArt();
      else if (strcmpi(id, "icon") == 0)
        return pAddon->Icon();
      else if (strcmpi(id, "id") == 0)
        return pAddon->ID();
      else if (strcmpi(id, "name") == 0)
        return pAddon->Name();
      else if (strcmpi(id, "path") == 0)
        return pAddon->Path();
      else if (strcmpi(id, "profile") == 0)
        return pAddon->Profile();
      else if (strcmpi(id, "stars") == 0)
      {
        return StringUtils::Format("%d", pAddon->Stars());
      }
      else if (strcmpi(id, "summary") == 0)
        return pAddon->Summary();
      else if (strcmpi(id, "type") == 0)
        return ADDON::TranslateType(pAddon->Type());
      else if (strcmpi(id, "version") == 0)
        return pAddon->Version().asString();
      else if (strcmpi(id, "platforms") == 0)
      {
        InfoMap::const_iterator it;
        if ((it = pAddon->Props().extrainfo.find("platforms")) != pAddon->Props().extrainfo.end())
          return String(it->second.c_str());
        else
          return String();
      }
      else if (strcmpi(id, "extensions") == 0)
      {
        InfoMap::const_iterator it;
        if ((it = pAddon->Props().extrainfo.find("extensions")) != pAddon->Props().extrainfo.end())
          return String(it->second.c_str());
        else
          return String();
      }
      else
        throw AddonException("'%s' is an invalid Id", id);
    }
  }
}
