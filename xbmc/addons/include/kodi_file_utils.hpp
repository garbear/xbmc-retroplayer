/*
 *      Copyright (C) 2013-2015 Team XBMC
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
#pragma once

/*!
 * @file kodi_file_utils.hpp  C++ wrappers for content add-on files
 */

#include "xbmc_content_types.h"

#include <cstddef>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#if defined(TARGET_WINDOWS) && !defined(strdup)
  // Silence warning C4996
  #define strdup _strdup
#endif

#ifndef SAFE_FREE
  #define SAFE_FREE(p)     do { free(p); (p)=NULL; } while (0)
#endif

/*!
 * Wrapper for CONTENT_ADDON_FILE_PROPERTY
 */
class AddonFileItemProperty
{
public:
  /*!
   * Create a new property of type string
   * @param strKey The key of this property
   * @param strValue The value of this property
   */
  AddonFileItemProperty(const std::string& strKey, const std::string& strValue) :
    m_strKey(strKey),
    m_type(CONTENT_ADDON_PROPERTY_TYPE_STRING),
    m_strValue(strValue),
    m_iValue(0),
    m_fValue(0.0)
  {
  }

  /*!
   * Create a new property of type integer
   * @param strKey The key of this property
   * @param iValue The value of this property
   */
  AddonFileItemProperty(const std::string& strKey, int iValue) :
    m_strKey(strKey),
    m_type(CONTENT_ADDON_PROPERTY_TYPE_INT),
    m_iValue(iValue),
    m_fValue(0.0)
  {
  }

  /*!
   * Create a new property of type double
   * @param strKey The key of this property
   * @param fValue The value of this property
   */
  AddonFileItemProperty(const std::string& strKey, double fValue) :
    m_strKey(strKey),
    m_type(CONTENT_ADDON_PROPERTY_TYPE_DOUBLE),
    m_iValue(0),
    m_fValue(fValue)
  {
  }

  /*!
   * Parse prop into a key and value
   * @param prop The property to parse
   */
  AddonFileItemProperty(const CONTENT_ADDON_FILE_PROPERTY& prop) :
    m_strKey(prop.key),
    m_type(prop.type),
    m_strValue(prop.type == CONTENT_ADDON_PROPERTY_TYPE_STRING ? prop.strValue : ""),
    m_iValue(prop.type == CONTENT_ADDON_PROPERTY_TYPE_INT ? prop.iValue : 0),
    m_fValue(prop.type == CONTENT_ADDON_PROPERTY_TYPE_DOUBLE ? prop.fValue : 0)
  {
  }

  /*!
   * @return The type of this property
   */
  CONTENT_ADDON_PROPERTY_TYPE Type(void) const
  {
    return m_type;
  }

  /*!
   * @return The key of this property
   */
  std::string Key(void) const
  {
    std::string val(m_strKey);
    return val;
  }

  /*!
   * @return The string value of this property, or an empty string if this is not a string
   */
  std::string ValueAsString(void) const
  {
    std::string val(m_strValue);
    return val;
  }

  /*!
   * @return The integer value of this property, or 0 if this is not an integer
   */
  int ValueAsInt(void) const
  {
    return m_iValue;
  }

  /*!
   * @return The double value of this property, or 0 if this is not an double
   */
  double ValueAsDouble(void) const
  {
    return m_fValue;
  }

  /*!
   * @return This property as CONTENT_ADDON_FILE_PROPERTY. Must be freed by calling AddonFileItemProperty::Free()
   */
  CONTENT_ADDON_FILE_PROPERTY AsProperty(void) const
  {
    CONTENT_ADDON_FILE_PROPERTY prop = { };

    prop.key = new char[m_strKey.length() + 1];
    std::strcpy(prop.key, m_strKey.c_str());

    if (m_type == CONTENT_ADDON_PROPERTY_TYPE_STRING)
    {
      prop.strValue = new char[m_strValue.length() + 1)];
      std::strcpy(prop.strValue, m_strValue.c_str());
    }
    else if (m_type == CONTENT_ADDON_PROPERTY_TYPE_INT)
      prop.iValue = m_iValue;
    else if (m_type == CONTENT_ADDON_PROPERTY_TYPE_DOUBLE)
      prop.fValue = m_fValue;

    return prop;
  }

  /*!
   * Free all data in the provided property object
   * @param prop The property to free
   */
  static void Free(CONTENT_ADDON_FILE_PROPERTY& prop)
  {
    delete[] prop.key;
    if (prop.type == CONTENT_ADDON_PROPERTY_TYPE_STRING)
      delete[] prop.strValue;
  }

private:
  std::string                 m_strKey;
  CONTENT_ADDON_PROPERTY_TYPE m_type;
  std::string                 m_strValue;
  int                         m_iValue;
  double                      m_fValue;
};

/*!
 * Wrapper for CONTENT_ADDON_FILE
 */
class AddonFileItem
{
protected:
  /*!
   * Create a new empty file item
   * @param type The type of this file
   */
  AddonFileItem(CONTENT_ADDON_TYPE type, const std::string& strPath, const std::string& strName) :
    m_type(type)
  {
    AddPropertyString("path", strPath);
    AddPropertyString("name", strName);
  }

public:
  /*!
   * Create a fileitem, copying the data from the provided file item.
   * @note Strings will _not_ be strdup'ed
   * @param fileItem The item to copy
   */
  AddonFileItem(const CONTENT_ADDON_FILEITEM& fileItem) :
    m_type(fileItem.type)
  {
    for (unsigned int iPtr = 0; iPtr < fileItem.iSize; iPtr++)
      m_properties.insert(std::make_pair(std::string(fileItem.properties[iPtr].key), AddonFileItemProperty(fileItem.properties[iPtr])));
  }

  virtual ~AddonFileItem(void)
  {
  }

  /*!
   * Add a property to this item. The value will be overwritten if the key exists.
   * @param strKey The key of this property
   * @param strValue The value of this property
   */
  void AddPropertyString(const std::string& strKey, const std::string& strValue)
  {
    std::map<std::string, AddonFileItemProperty>::iterator it = m_properties.find(strKey);
    if (it != m_properties.end())
      it->second = AddonFileItemProperty(strKey, strValue);
    else
      m_properties.insert(std::make_pair(strKey, AddonFileItemProperty(strKey, strValue)));
  }

  /*!
   * Add a property to this item. The value will be overwritten if the key exists.
   * @param strKey The key of this property
   * @param iValue The value of this property
   */
  void AddPropertyInt(const std::string& strKey, int iValue)
  {
    std::map<std::string, AddonFileItemProperty>::iterator it = m_properties.find(strKey);
    if (it != m_properties.end())
      it->second = AddonFileItemProperty(strKey, iValue);
    else
      m_properties.insert(std::make_pair(strKey, AddonFileItemProperty(strKey, iValue)));
  }

  /*!
   * Add a property to this item. The value will be overwritten if the key exists.
   * @param strKey The key of this property
   * @param fValue The value of this property
   */
  void AddPropertyDouble(const std::string& strKey, double fValue)
  {
    std::map<std::string, AddonFileItemProperty>::iterator it = m_properties.find(strKey);
    if (it != m_properties.end())
      it->second = AddonFileItemProperty(strKey, fValue);
    else
      m_properties.insert(std::make_pair(strKey, AddonFileItemProperty(strKey, fValue)));
  }

  /*!
   * Assign all properties to the provided file item. Must be freed by calling AddonFileItem::Free()
   * @param fileItem The file item to assign the values to
   */
  void ToStruct(CONTENT_ADDON_FILEITEM& fileItem) const
  {
    fileItem.type       = m_type;
    fileItem.iSize      = 0;
    fileItem.properties = new CONTENT_ADDON_FILE_PROPERTY[m_properties.size()];
    if (fileItem.properties)
    {
      for (std::map<std::string, AddonFileItemProperty>::const_iterator it = m_properties.begin(); it != m_properties.end(); it++)
        fileItem.properties[fileItem.iSize++] = it->second.AsProperty();
    }
  }

  /*!
   * Free all data in the provided file item
   * @param prop The file item to free
   */
  static void Free(CONTENT_ADDON_FILEITEM& fileItem)
  {
    for (unsigned int iPtr = 0; iPtr < fileItem.iSize; iPtr++)
      AddonFileItemProperty::Free(fileItem.properties[iPtr]);
    delete[] fileItem.properties;
  }

  /*!
   * Look up and remove a property of type string
   * @param strKey The key of the property to look up
   * @param strDefault The default value to use, if the key could not be found
   * @return The value
   */
  std::string GetAndRemovePropertyString(const std::string& strKey, const std::string& strDefault = "")
  {
    std::string strReturn(strDefault);
    std::map<std::string, AddonFileItemProperty>::iterator it = m_properties.find(strKey);
    if (it != m_properties.end() && it->second.Type() == CONTENT_ADDON_PROPERTY_TYPE_STRING)
    {
      strReturn = it->second.ValueAsString();
      m_properties.erase(it);
    }
    return strReturn;
  }

  /*!
   * Look up a property of type string
   * @param strKey The key of the property to look up
   * @param strDefault The default value to use, if the key could not be found
   * @return The value
   */
  std::string GetPropertyString(const std::string& strKey, const std::string& strDefault = "") const
  {
    std::string strReturn(strDefault);
    std::map<std::string, AddonFileItemProperty>::const_iterator it = m_properties.find(strKey);
    if (it != m_properties.end() && it->second.Type() == CONTENT_ADDON_PROPERTY_TYPE_STRING)
      strReturn = it->second.ValueAsString();
    return strReturn;
  }

  /*!
   * Look up and remove a property of type integer
   * @param strKey The key of the property to look up
   * @param iDefault The default value to use, if the key could not be found
   * @return The value
   */
  int GetAndRemovePropertyInt(const std::string& strKey, int iDefault = 0)
  {
    int iReturn(iDefault);
    std::map<std::string, AddonFileItemProperty>::iterator it = m_properties.find(strKey);
    if (it != m_properties.end() && it->second.Type() == CONTENT_ADDON_PROPERTY_TYPE_INT)
    {
      iReturn = it->second.ValueAsInt();
      m_properties.erase(it);
    }
    return iReturn;
  }

  /*!
   * Look up a property of type integer
   * @param strKey The key of the property to look up
   * @param iDefault The default value to use, if the key could not be found
   * @return The value
   */
  int GetPropertyInt(const std::string& strKey, int iDefault = 0) const
  {
    int iReturn(iDefault);
    std::map<std::string, AddonFileItemProperty>::const_iterator it = m_properties.find(strKey);
    if (it != m_properties.end() && it->second.Type() == CONTENT_ADDON_PROPERTY_TYPE_INT)
      iReturn = it->second.ValueAsInt();
    return iReturn;
  }

  /*!
   * Look up and remove a property of type integer
   * @param strKey The key of the property to look up
   * @param iDefault The default value to use, if the key could not be found
   * @return The value
   */
  double GetAndRemovePropertyDouble(const std::string& strKey, double fDefault = 0.0)
  {
    double fReturn(fDefault);
    std::map<std::string, AddonFileItemProperty>::iterator it = m_properties.find(strKey);
    if (it != m_properties.end() && it->second.Type() == CONTENT_ADDON_PROPERTY_TYPE_DOUBLE)
    {
      fReturn = it->second.ValueAsDouble();
      m_properties.erase(it);
    }
    return fReturn;
  }

  /*!
   * Look up a property of type integer
   * @param strKey The key of the property to look up
   * @param iDefault The default value to use, if the key could not be found
   * @return The value
   */
  double GetPropertyDouble(const std::string& strKey, double fDefault = 0.0) const
  {
    double fReturn(fDefault);
    std::map<std::string, AddonFileItemProperty>::const_iterator it = m_properties.find(strKey);
    if (it != m_properties.end() && it->second.Type() == CONTENT_ADDON_PROPERTY_TYPE_DOUBLE)
      fReturn = it->second.ValueAsDouble();
    return fReturn;
  }

  std::string Path(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("path") : GetPropertyString("path"); }
  std::string Path() const { return GetPropertyString("path"); }

  std::string Name(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("name") : GetPropertyString("name"); }
  std::string Name() const { return GetPropertyString("name"); }

  std::string Thumb(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("thumb") : GetPropertyString("thumb"); }
  std::string Thumb() const { return GetPropertyString("thumb"); }
  void SetThumb(const std::string& strThumb) { AddPropertyString("thumb", strThumb); }

  std::string Fanart(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("fanart_image") : GetPropertyString("fanart_image"); }
  std::string Fanart() const { return GetPropertyString("fanart_image"); }
  void SetFanart(const std::string& strFanart) { AddPropertyString("fanart_image", strFanart); }

  std::string ProviderIcon(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("provider_icon") : GetPropertyString("provider_icon"); }
  std::string ProviderIcon() const { return GetPropertyString("provider_icon"); }
  void SetProviderIcon(const std::string& strIcon) { AddPropertyString("provider_icon", strIcon); }

  std::map<std::string, AddonFileItemProperty> m_properties;

  CONTENT_ADDON_TYPE Type(void) const { return m_type; }

private:
  CONTENT_ADDON_TYPE m_type;
};

/*!
 * Wrapper for CONTENT_ADDON_FILELIST
 */
class AddonFileItemList
{
public:
  /*!
   * Create a new empty file item list
   */
  AddonFileItemList(void)
  {
  }

  /*!
   * Create a new file item list, copying the data from the provided file item list.
   * @param items
   */
  AddonFileItemList(const CONTENT_ADDON_FILELIST& items)
  {
    m_fileItems.reserve(items.iSize);
    for (unsigned int iPtr = 0; iPtr < items.iSize; iPtr++)
      m_fileItems.push_back(items.items[iPtr]);
  }

  virtual ~AddonFileItemList(void)
  {
  }

  /*!
   * Free all data in the provided file item list
   * @param items The list to free
   */
  static void Free(CONTENT_ADDON_FILELIST* items)
  {
    if (!items)
      return;

    for (unsigned int iPtr = 0; iPtr < items->iSize; iPtr++)
      AddonFileItem::Free(items->items[iPtr]);

    free(items->items);
    free(items);
  }

  /*!
   * This file item list as CONTENT_ADDON_FILELIST*
   * Must be freed by calling Free()
   * @return
   */
  CONTENT_ADDON_FILELIST* AsFileList(void)
  {
    CONTENT_ADDON_FILELIST* items = new CONTENT_ADDON_FILELIST;
    items->items = new CONTENT_ADDON_FILEITEM[m_fileItems.size()];
    for (std::vector<AddonFileItem>::const_iterator it = m_fileItems.begin(); it != m_fileItems.end(); it++)
      it->ToStruct(items->items[items->iSize++]);
    return items;
  }

  /*!
   * Add a file item to this list
   * @param item The item to add
   */
  void AddFileItem(const AddonFileItem& item)
  {
    m_fileItems.push_back(item);
  }

  /*!
   * Add multiple file items to this list
   * @param item The item to add
   */
  void AddFileItems(const AddonFileItemList& list)
  {
    m_fileItems.insert(m_fileItems.end(), list.m_fileItems.begin(), list.m_fileItems.end());
  }

  std::vector<AddonFileItem> m_fileItems;
};

class AddonFileGame : public AddonFileItem
{
public:
  AddonFileGame(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_GAME, strPath, strName) {}
  virtual AddonFileGame(void) {}

  int Year(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("year") : GetPropertyInt("year"); }
  void SetYear(int iYear) { AddPropertyInt("year", iYear); }

  int Rating(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("rating") : GetPropertyInt("rating"); }
  void SetRating(int iRating) { AddPropertyInt("rating", iRating); }
};

class AddonFileDirectory : public AddonFileItem
{
public:
  AddonFileDirectory(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_DIRECTORY, strPath, strName) {}
  virtual ~AddonFileDirectory(void) {}
};

class AddonFileFile : public AddonFileItem
{
public:
  AddonFileFile(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_FILE, strPath, strName) {}
  virtual ~AddonFileFile(void) {}
};

class AddonFileYear : public AddonFileItem
{
public:
  AddonFileYear(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_YEAR, strPath, strName) {}
  virtual ~AddonFileYear(void) {}
};
