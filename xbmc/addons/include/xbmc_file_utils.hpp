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
 * @file xbmc_file_utils.hpp  C++ wrappers for content add-on files
 */

#include "xbmc_content_types.h"
#include <cstddef>
#include <string>
#include <string.h>
//#include <malloc.h> // TODO
#include <vector>
#include <map>

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
    CONTENT_ADDON_FILE_PROPERTY prop;
    prop.key = strdup(m_strKey.c_str());
    prop.type = m_type;
    if (m_type == CONTENT_ADDON_PROPERTY_TYPE_STRING)
      prop.strValue = strdup(m_strValue.c_str());
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
    SAFE_FREE(prop.key);
    if (prop.type == CONTENT_ADDON_PROPERTY_TYPE_STRING)
      SAFE_FREE(prop.strValue);
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
  void FileItem(CONTENT_ADDON_FILEITEM& fileItem) const
  {
    fileItem.type       = m_type;
    fileItem.iSize      = 0;
    fileItem.properties = (CONTENT_ADDON_FILE_PROPERTY*) malloc(sizeof(CONTENT_ADDON_FILE_PROPERTY) * m_properties.size());
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
    SAFE_FREE(fileItem.properties);
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
    CONTENT_ADDON_FILELIST* items = (CONTENT_ADDON_FILELIST*)malloc(sizeof(CONTENT_ADDON_FILELIST));
    if (items)
    {
      memset(items, 0, sizeof(CONTENT_ADDON_FILELIST));

      items->items = (CONTENT_ADDON_FILEITEM*)malloc(sizeof(CONTENT_ADDON_FILEITEM) * m_fileItems.size());
      if (items->items && !m_fileItems.empty())
      {
        memset(items->items, 0, sizeof(CONTENT_ADDON_FILEITEM) * m_fileItems.size());

        for (std::vector<AddonFileItem>::const_iterator it = m_fileItems.begin(); it != m_fileItems.end(); it++)
          it->FileItem(items->items[items->iSize++]);
      }
    }
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

class AddonFileSong : public AddonFileItem
{
public:
  AddonFileSong(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_SONG, strPath, strName) {}
  virtual ~AddonFileSong(void) {}

  int Track(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("track") : GetPropertyInt("track"); }
  void SetTrack(int iTrack) { AddPropertyInt("track", iTrack); }

  int Duration(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("duration") : GetPropertyInt("duration"); }
  void SetDuration(int iDuration) { AddPropertyInt("duration", iDuration); }

  int Rating(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("rating") : GetPropertyInt("rating"); }
  void SetRating(int iRating) { AddPropertyInt("rating", iRating); }

  std::string Artists(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("artists") : GetPropertyString("artists"); }
  void SetArtists(const std::string& strArtists) { AddPropertyString("artists", strArtists); }

  int Year(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("year") : GetPropertyInt("year"); }
  void SetYear(int iYear) { AddPropertyInt("year", iYear); }

  std::string Album(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("album") : GetPropertyString("album"); }
  void SetAlbum(const std::string& strAlbum) { AddPropertyString("album", strAlbum); }

  std::string AlbumArtists(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("album_artists") : GetPropertyString("album_artists"); }
  void SetAlbumArtists(const std::string& strAlbumArtists) { AddPropertyString("album_artists", strAlbumArtists); }
};

class AddonFileArtist : public AddonFileItem
{
public:
  AddonFileArtist(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_ARTIST, strPath, strName) {}
  virtual ~AddonFileArtist(void) {}

  std::string Genres(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("genres") : GetPropertyString("genres"); }
  void SetGenres(const std::string& strGenres) { AddPropertyString("genres", strGenres); }

  std::string Biography(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("biography") : GetPropertyString("biography"); }
  void SetBiography(const std::string& strBiography) { AddPropertyString("biography", strBiography); }

  std::string Styles(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("styles") : GetPropertyString("styles"); }
  void SetStyles(const std::string& strStyles) { AddPropertyString("styles", strStyles); }

  std::string Moods(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("moods") : GetPropertyString("moods"); }
  void SetMoods(const std::string& strMoods) { AddPropertyString("moods", strMoods); }

  std::string Born(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("born") : GetPropertyString("born"); }
  void SetBorn(const std::string& strBorn) { AddPropertyString("born", strBorn); }

  std::string Formed(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("formed") : GetPropertyString("formed"); }
  void SetFormed(const std::string& strFormed) { AddPropertyString("formed", strFormed); }

  std::string Died(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("died") : GetPropertyString("died"); }
  void SetDied(const std::string& strDied) { AddPropertyString("died", strDied); }

  std::string Disbanded(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("disbanded") : GetPropertyString("disbanded"); }
  void SetDisbanded(const std::string& strDisbanded) { AddPropertyString("disbanded", strDisbanded); }

  std::string YearsActive(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("years_active") : GetPropertyString("years_active"); }
  void SetYearsActive(const std::string& strYearsActive) { AddPropertyString("years_active", strYearsActive); }

  std::string Instruments(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("instruments") : GetPropertyString("instruments"); }
  void SetInstruments(const std::string& strInstruments) { AddPropertyString("instruments", strInstruments); }
};

class AddonFileAlbum : public AddonFileItem
{
public:
  AddonFileAlbum(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_ALBUM, strPath, strName) {}
  virtual ~AddonFileAlbum(void) {}

  int Year(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("year") : GetPropertyInt("year"); }
  void SetYear(int iYear) { AddPropertyInt("year", iYear); }

  std::string Artists(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("artists") : GetPropertyString("artists"); }
  void SetArtists(const std::string& strArtists) { AddPropertyString("artists", strArtists); }

  std::string Genres(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("genres") : GetPropertyString("genres"); }
  void SetGenres(const std::string& strGenres) { AddPropertyString("genres", strGenres); }

  int Rating(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("rating") : GetPropertyInt("rating"); }
  void SetRating(int iRating) { AddPropertyInt("rating", iRating); }

  std::string Review(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("review") : GetPropertyString("review"); }
  void SetReview(const std::string& strReview) { AddPropertyString("review", strReview); }

  std::string Styles(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("styles") : GetPropertyString("styles"); }
  void SetStyles(const std::string& strStyles) { AddPropertyString("styles", strStyles); }

  std::string Moods(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("moods") : GetPropertyString("moods"); }
  void SetMoods(const std::string& strMoods) { AddPropertyString("moods", strMoods); }

  std::string Themes(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("themes") : GetPropertyString("themes"); }
  void SetThemes(const std::string& strThemes) { AddPropertyString("themes", strThemes); }

  std::string Label(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("label") : GetPropertyString("label"); }
  void SetLabel(const std::string& strLabel) { AddPropertyString("label", strLabel); }

  std::string Type(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("type") : GetPropertyString("type"); }
  void SetType(const std::string& strType) { AddPropertyString("type", strType); }

  bool Compilation(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("compilation") == 1 : GetPropertyInt("compilation") == 1; }
  void SetCompilation(bool bCompilation) { AddPropertyInt("compilation", bCompilation?1:0); }

  int TimesPlayed(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("times_played") : GetPropertyInt("times_played"); }
  void SetTimesPlayed(int iTimesPlayed) { AddPropertyInt("times_played", iTimesPlayed); }
};

// TODO: Decide if we want to keep the commented items
// See VideoInfoTag.cpp
class AddonFileVideo : public AddonFileItem
{
public:
  AddonFileVideo(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_VIDEO, strPath, strName) {}
  virtual ~AddonFileVideo(void) {}

  std::string Directors() const { return GetPropertyString("directors"); }
  std::string Directors(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("directors") : GetPropertyString("directors"); }
  void SetDirectors(const std::string& strDirectors) { AddPropertyString("directors", strDirectors); }

  std::string WritingCredits() const { return GetPropertyString("writing_credits"); }
  std::string WritingCredits(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("writing_credits") : GetPropertyString("writing_credits"); }
  void SetWritingCredits(const std::string& strWritingCredits) { AddPropertyString("writing_credits", strWritingCredits); }

  std::string Genres() const { return GetPropertyString("genres"); }
  std::string Genres(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("genres") : GetPropertyString("genres"); }
  void SetGenres(const std::string& strGenres) { AddPropertyString("genres", strGenres); }

  std::string Countries() const { return GetPropertyString("countries"); }
  std::string Countries(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("countries") : GetPropertyString("countries"); }
  void SetCountries(const std::string& strCountries) { AddPropertyString("countries", strCountries); }

  std::string TagLine() const { return GetPropertyString("tagline"); }
  std::string TagLine(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("tagline") : GetPropertyString("tagline"); }
  void SetTagLine(const std::string& strTagLine) { AddPropertyString("tagline", strTagLine); }

  std::string PlotOutline() const { return GetPropertyString("plot_outline"); }
  std::string PlotOutline(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("plot_outline") : GetPropertyString("plot_outline"); }
  void SetPlotOutline(const std::string& strPlotOutline) { AddPropertyString("plot_outline", strPlotOutline); }

  std::string Trailer() const { return GetPropertyString("trailer"); }
  std::string Trailer(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("trailer") : GetPropertyString("trailer"); }
  void SetTrailer(const std::string& strTrailer) { AddPropertyString("trailer", strTrailer); }

  std::string Plot() const { return GetPropertyString("plot"); }
  std::string Plot(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("plot") : GetPropertyString("plot"); }
  void SetPlot(const std::string& strPlot) { AddPropertyString("plot", strPlot); }

  //CScraperUrl m_strPictureURL?

  std::string SortTitle() const { return GetPropertyString("sort_title"); }
  std::string SortTitle(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("sort_title") : GetPropertyString("sort_title"); }
  void SetSortTitle(const std::string& strSortTitle) { AddPropertyString("sort_title", strSortTitle); }

  /*
  std::string Votes() const { return GetPropertyString("votes"); }
  std::string Votes(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("votes") : GetPropertyString("votes"); }
  void SetVotes(const std::string& strVotes) { AddPropertyString("votes", strVotes); }
  */

  std::string Artists() const { return GetPropertyString("artists"); }
  std::string Artists(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("artists") : GetPropertyString("artists"); }
  void SetArtists(const std::string& strArtists) { AddPropertyString("artists", strArtists); }

  //std::vector<SActorInfo> m_cast?

  std::string Set() const { return GetPropertyString("set"); }
  std::string Set(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("set") : GetPropertyString("set"); }
  void SetSet(const std::string& strSet) { AddPropertyString("set", strSet); }

  std::string Tags() const { return GetPropertyString("tags"); }
  std::string Tags(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("tags") : GetPropertyString("tags"); }
  void SetTags(const std::string& strTags) { AddPropertyString("tags", strTags); }

  std::string IMDBNumber() const { return GetPropertyString("imdb_number"); }
  std::string IMDBNumber(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("imdb_number") : GetPropertyString("imdb_number"); }
  void SetIMDBNumber(const std::string& strIMDBNumber) { AddPropertyString("imdb_number", strIMDBNumber); }

  std::string MPAARating() const { return GetPropertyString("mpaa_rating"); }
  std::string MPAARating(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("mpaa_rating") : GetPropertyString("mpaa_rating"); }
  void SetMPAARating(const std::string& strMPAARating) { AddPropertyString("mpaa_rating", strMPAARating); }

  std::string OriginalTitle() const { return GetPropertyString("original_title"); }
  std::string OriginalTitle(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("original_title") : GetPropertyString("original_title"); }
  void SetOriginalTitle(const std::string& strOriginalTitle) { AddPropertyString("original_title", strOriginalTitle); }

  /*
  std::string EpisodeGuide() const { return GetPropertyString("episode_guide"); }
  std::string EpisodeGuide(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("episode_guide") : GetPropertyString("episode_guide"); }
  void SetEpisodeGuide(const std::string& strEpisodeGuide) { AddPropertyString("episode_guide", strEpisodeGuide); }
  */

  //CDateTime m_premiered

  /*
  std::string Status() const { return GetPropertyString("status"); }
  std::string Status(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("status") : GetPropertyString("status"); }
  void SetStatus(const std::string& strStatus) { AddPropertyString("status", strStatus); }

  std::string ProductionCode() const { return GetPropertyString("production_code"); }
  std::string ProductionCode(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("production_code") : GetPropertyString("production_code"); }
  void SetProductionCode(const std::string& strProductionCode) { AddPropertyString("production_code", strProductionCode); }
  */

  //CDateTime m_firstAired
  
  std::string ShowTitle() const { return GetPropertyString("show_title"); }
  std::string ShowTitle(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("show_title") : GetPropertyString("show_title"); }
  void SetShowTitle(const std::string& strShowTitle) { AddPropertyString("show_title", strShowTitle); }

  std::string Studios() const { return GetPropertyString("studios"); }
  std::string Studios(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("studios") : GetPropertyString("studios"); }
  void SetStudios(const std::string& strStudios) { AddPropertyString("studios", strStudios); }

  std::string Album() const { return GetPropertyString("album"); }
  std::string Album(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("album") : GetPropertyString("album"); }
  void SetAlbum(const std::string& strAlbum) { AddPropertyString("album", strAlbum); }

  //CDateTime m_lastPlayed

  /*
  std::string ShowLinks() const { return GetPropertyString("show_links"); }
  std::string ShowLinks(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("show_links") : GetPropertyString("show_links"); }
  void SetShowLinks(const std::string& strShowLinks) { AddPropertyString("show_links", strShowLinks); }

  std::string ShowPath() const { return GetPropertyString("show_path"); }
  std::string ShowPath(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("show_path") : GetPropertyString("show_path"); }
  void SetShowPath(const std::string& strShowPath) { AddPropertyString("show_path", strShowPath); }
  */

  int TimesPlayed() const { return GetPropertyInt("times_played"); }
  int TimesPlayed(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("times_played") : GetPropertyInt("times_played"); }
  void SetTimesPlayed(int iTimesPlayed) { AddPropertyInt("times_played", iTimesPlayed); }

  /*
  int Top250() const { return GetPropertyInt("top250"); }
  int Top250(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("top250") : GetPropertyInt("top250"); }
  void SetTop250(int iTop250) { AddPropertyInt("top250", iTop250); }
  */

  int Year() const { return GetPropertyInt("year"); }
  int Year(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("year") : GetPropertyInt("year"); }
  void SetYear(int iYear) { AddPropertyInt("year", iYear); }

  int Season() const { return GetPropertyInt("season"); }
  int Season(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("season") : GetPropertyInt("season"); }
  void SetSeason(int iSeason) { AddPropertyInt("season", iSeason); }

  int Episode() const { return GetPropertyInt("episode"); }
  int Episode(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("episode") : GetPropertyInt("episode"); }
  void SetEpisode(int iEpisode) { AddPropertyInt("episode", iEpisode); }

  /*
  std::string UniqueID() const { return GetPropertyString("unique_id"); }
  std::string UniqueID(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("unique_id") : GetPropertyString("unique_id"); }
  void SetUniqueID(const std::string& strUniqueID) { AddPropertyString("unique_id", strUniqueID); }

  int SpecialSortSeason() const { return GetPropertyInt("special_sort_season"); }
  int SpecialSortSeason(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("special_sort_season") : GetPropertyInt("special_sort_season"); }
  void SetSpecialSortSeason(int iSpecialSortSeason) { AddPropertyInt("special_sort_season", iSpecialSortSeason); }

  int SpecialSortEpisode() const { return GetPropertyInt("special_sort_episode"); }
  int SpecialSortEpisode(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("special_sort_episode") : GetPropertyInt("special_sort_episode"); }
  void SetSpecialSortEpisode(int iSpecialSortEpisode) { AddPropertyInt("special_sort_episode", iSpecialSortEpisode); }
  */

  int Track() const { return GetPropertyInt("track"); }
  int Track(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("track") : GetPropertyInt("track"); }
  void SetTrack(int iTrack) { AddPropertyInt("track", iTrack); }

  double Rating() const { return GetPropertyDouble("rating"); }
  double Rating(bool bRemove = false) { return bRemove ? GetAndRemovePropertyDouble("rating") : GetPropertyDouble("rating"); }
  void SetRating(double fRating) { AddPropertyDouble("rating", fRating); }

  /*
  double EpisodeBookmark() const { return GetPropertyDouble("episode_bookmark"); }
  double EpisodeBookmark(bool bRemove = false) { return bRemove ? GetAndRemovePropertyDouble("episode_bookmark") : GetPropertyDouble("episode_bookmark"); }
  void SetEpisodeBookmark(double fEpisodeBookmark) { AddPropertyDouble("episode_bookmark", fEpisodeBookmark); }

  int BookmarkID() const { return GetPropertyInt("bookmark_id"); }
  int BookmarkID(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("bookmark_id") : GetPropertyInt("bookmark_id"); }
  void SetBookmarkID(int iBookmarkID) { AddPropertyInt("bookmark_id", iBookmarkID); }
  */

  //CBookmark m_resumePoint

  //CDateTime m_dateAdded

  /*
  std::string Type() const { return GetPropertyString("type"); }
  std::string Type(bool bRemove = false) { return bRemove ? GetAndRemovePropertyString("type") : GetPropertyString("type"); }
  void SetType(const std::string& strType) { AddPropertyString("type", strType); }
  */

  int Duration() const { return GetPropertyInt("duration"); }
  int Duration(bool bRemove = false) { return bRemove ? GetAndRemovePropertyInt("duration") : GetPropertyInt("duration"); }
  void SetDuration(int iDuration) { AddPropertyInt("duration", iDuration); }
};

class AddonFilePlaylist : public AddonFileItem
{
public:
  AddonFilePlaylist(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_PLAYLIST, strPath, strName) {}
  virtual ~AddonFilePlaylist(void) {}
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

class AddonFileGenre : public AddonFileItem
{
public:
  AddonFileGenre(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_GENRE, strPath, strName) {}
  virtual ~AddonFileGenre(void) {}
};

class AddonFileYear : public AddonFileItem
{
public:
  AddonFileYear(const std::string& strPath, const std::string& strName) :
    AddonFileItem(CONTENT_ADDON_TYPE_YEAR, strPath, strName) {}
  virtual ~AddonFileYear(void) {}
};
