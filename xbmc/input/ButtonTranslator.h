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

#ifndef BUTTON_TRANSLATOR_H
#define BUTTON_TRANSLATOR_H

#pragma once

#include <map>
#include <string>
#include <vector>
#include "system.h" // for HAS_EVENT_SERVER

#ifdef HAS_EVENT_SERVER
#include "network/EventClient.h"
#endif

class CKey;
class CAction;
class TiXmlNode;
struct AxisConfig;
class CRegExp;
typedef std::vector<AxisConfig> AxesConfig; // [<axis, isTrigger, rest state value>]

struct CButtonAction
{
  int id;
  std::string strID; // needed for "ActivateWindow()" type actions
};
///
/// singleton class to map from buttons to actions
/// Warning: _not_ threadsafe!
class CButtonTranslator
{
#ifdef HAS_EVENT_SERVER
  friend class EVENTCLIENT::CEventButtonState;
#endif

private:
  //private construction, and no assignements; use the provided singleton methods
  CButtonTranslator();
  CButtonTranslator(const CButtonTranslator&);
  CButtonTranslator const& operator=(CButtonTranslator const&);
  virtual ~CButtonTranslator();
  bool HasDeviceType(TiXmlNode *pWindow, std::string type);
public:
  ///access to singleton
  static CButtonTranslator& GetInstance();

  // Add/remove a HID device with custom mappings
  void AddDevice(std::string& strDevice);
  void RemoveDevice(std::string& strDevice);

  /// loads Lircmap.xml/IRSSmap.xml (if enabled) and Keymap.xml
  bool Load(bool AlwaysLoad = false);
  /// clears the maps
  void Clear();

  static void GetActions(std::vector<std::string> &actionList);
  static void GetWindows(std::vector<std::string> &windowList);

  CAction GetAction(int window, const CKey &key, bool fallback = true);

  static bool IsAnalog(int actionID);

  /*! \brief Translate between a window name and it's id
   \param window name of the window
   \return id of the window, or WINDOW_INVALID if not found
   */
  static int TranslateWindow(const std::string &window);

  /*! \brief Translate between a window id and it's name
   \param window id of the window
   \return name of the window, or an empty string if not found
   */
  static std::string TranslateWindow(int window);

  static bool TranslateActionString(const char *szAction, int &action);

  int TranslateLircRemoteString(const char* szDevice, const char *szButton);

  bool TranslateTouchAction(int window, int touchAction, int touchPointers, int &action);

private:
  typedef std::multimap<uint32_t, CButtonAction> buttonMap; // our button map to fill in

  // m_translatorMap contains all mappings i.e. m_BaseMap + HID device mappings
  std::map<int, buttonMap> m_translatorMap;
  // m_deviceList contains the list of connected HID devices
  std::list<std::string> m_deviceList;

  int GetActionCode(int window, int action);
  int GetActionCode(int window, const CKey &key, std::string &strAction) const;
  int GetFallbackWindow(int windowID);

  static uint32_t TranslateGamepadString(const char *szButton);
  static uint32_t TranslateRemoteString(const char *szButton);
  static uint32_t TranslateUniversalRemoteString(const char *szButton);

  static uint32_t TranslateKeyboardString(const char *szButton);
  static uint32_t TranslateKeyboardButton(TiXmlElement *pButton);

  static uint32_t TranslateMouseCommand(TiXmlElement *pButton);

  static uint32_t TranslateAppCommand(const char *szButton);

  void MapWindowActions(TiXmlNode *pWindow, int wWindowID);
  void MapAction(uint32_t buttonCode, const char *szAction, buttonMap &map);

  bool LoadKeymap(const std::string &keymapPath);
  bool LoadLircMap(const std::string &lircmapPath);
  void ClearLircButtonMapEntries();

  void MapRemote(TiXmlNode *pRemote, const char* szDevice);

  typedef std::map<std::string, std::string> lircButtonMap;
  std::map<std::string, lircButtonMap*> lircRemotesMap;

  void MapTouchActions(int windowID, TiXmlNode *pTouch);
  static uint32_t TranslateTouchCommand(TiXmlElement *pButton, CButtonAction &action);
  int GetTouchActionCode(int window, int action);

  std::map<int, buttonMap> m_touchMap;

  bool m_Loaded;
};

#endif

