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

#include "ContentAddons.h"
#include "Application.h"
#include "filesystem/AddonsDirectory.h"
#include "filesystem/MusicDatabaseDirectory.h"
#include "MediaSource.h"
#include "utils/StringUtils.h"
#include "URL.h"

using namespace std;
using namespace ADDON;
using namespace XFILE;
using namespace XFILE::MUSICDATABASEDIRECTORY;

//TODO remove musicdb:// hacks

CContentAddons& CContentAddons::Get(void)
{
  static CContentAddons instance;
  return instance;
}

bool CContentAddons::MusicHasAvailableAddons(void) const
{
  CONTENT_CLIENTMAP map;
  {
    CSingleLock lock(m_critSection);
    map = m_addonMap;
  }
  for (CONTENT_CLIENTMAP_CITR it = map.begin(); it != map.end(); it++)
    if (it->second->ProvidesMusicCodec()) //TODO
      return true;
  return false;
}

bool CContentAddons::SupportsConcurrentStreams(const std::string& strPath) const
{
  CONTENT_ADDON addon = GetAddonForPath(strPath);
  if (!addon.get())
    return true;
  return addon->SupportsConcurrentStreams();
}

bool CContentAddons::MusicGetAddons(CFileItemList& items) const
{
  const std::string strBasePath = MUSIC_VIRTUAL_NODE;

  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
  {
    CFileItemPtr pItem;
    if (it->second->ProvidesMusicFiles())
    {
      pItem = CAddonsDirectory::FileItemFromAddon(it->second, strBasePath, true);

      // Mark add-on as needing configuration
      if (pItem && it->second->GetStatus() == ADDON_STATUS_NEED_SETTINGS)
        pItem->SetProperty("need_configuration", "true");
    }
    if (pItem)
      items.Add(pItem);
  }
  return true;
}

bool CContentAddons::MusicGetOverviewItems(CFileItemList& items)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicGetOverviewItems(items);
  return bReturn;
}

bool CContentAddons::MusicGetArtists(CFileItemList& items)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicGetArtists(items);
  return bReturn;
}

bool CContentAddons::MusicGetAlbums(CFileItemList& items, const std::string& strArtistName)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicGetAlbums(items, strArtistName);
  return bReturn;
}

bool CContentAddons::MusicGetSongs(CFileItemList& items, const std::string& strArtistName, const std::string& strAlbumName)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicGetSongs(items, strArtistName, strAlbumName);
  return bReturn;
}

bool CContentAddons::MusicGetPlaylists(CFileItemList& items)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicGetPlaylists(items);
  return bReturn;
}

bool CContentAddons::MusicGetTop100(CFileItemList& items, CONTENT_TOP100_TYPE type)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicGetTop100(items, type);
  return bReturn;
}

bool CContentAddons::MusicSearch(CFileItemList& items, const std::string& strQuery)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->ProvidesMusicFiles())
      bReturn |= it->second->MusicSearch(items, strQuery);
  return bReturn;
}

bool CContentAddons::VideoHasAvailableAddons(void) const
{
  CONTENT_CLIENTMAP map;
  {
    CSingleLock lock(m_critSection);
    map = m_addonMap;
  }
  for (CONTENT_CLIENTMAP_CITR it = map.begin(); it != map.end(); it++)
  {
    if (it->second->Provides(CONTENT_VIDEO_TYPE_MOVIES) ||
        it->second->Provides(CONTENT_VIDEO_TYPE_TVSHOWS) ||
        it->second->Provides(CONTENT_VIDEO_TYPE_MUSICVIDEOS)) // TODO
    {
      return true;
    }
  }
  return false;
}

bool CContentAddons::VideoHasAvailableAddons(CONTENT_VIDEO_TYPE videoType) const
{
  CONTENT_CLIENTMAP map;
  {
    CSingleLock lock(m_critSection);
    map = m_addonMap;
  }
  for (CONTENT_CLIENTMAP_CITR it = map.begin(); it != map.end(); it++)
  {
    if (it->second->Provides(videoType)) // TODO
      return true;
  }
  return false;
}

bool CContentAddons::VideoGetAddons(CFileItemList& items) const
{
  const std::string strBasePath(VIDEO_VIRTUAL_NODE);
  
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
  {
    CFileItemPtr pItem;
    if (it->second->Provides(CONTENT_VIDEO_TYPE_MOVIES) ||
        it->second->Provides(CONTENT_VIDEO_TYPE_TVSHOWS) ||
        it->second->Provides(CONTENT_VIDEO_TYPE_MUSICVIDEOS))
    {
      pItem = CAddonsDirectory::FileItemFromAddon(it->second, strBasePath, true);

      // Mark add-on as needing configuration
      if (pItem && it->second->GetStatus() == ADDON_STATUS_NEED_SETTINGS)
        pItem->SetProperty("need_configuration", "true");
    }
    if (pItem)
      items.Add(pItem);
  }
  return true;
}

bool CContentAddons::VideoGetAddons(CFileItemList& items, CONTENT_VIDEO_TYPE videoType) const
{
  std::string strBasePath = StringUtils::Format("videodb://%d/99/", videoType);

  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
  {
    CFileItemPtr pItem;
    if (it->second->Provides(videoType))
    {
      pItem = CAddonsDirectory::FileItemFromAddon(it->second, strBasePath, true);

      // Mark add-on as needing configuration
      if (pItem && it->second->GetStatus() == ADDON_STATUS_NEED_SETTINGS)
        pItem->SetProperty("need_configuration", "true");
    }
    if (pItem)
      items.Add(pItem);
  }
  return true;
}

bool CContentAddons::VideoGetAllOverviewItems(CFileItemList& items)
{
  bool bReturn(false);
  // Get items by type, not by add-on, so that all items of the same type will
  // be grouped together.
  bReturn |= VideoGetOverviewItems(items, CONTENT_VIDEO_TYPE_MOVIES);
  bReturn |= VideoGetOverviewItems(items, CONTENT_VIDEO_TYPE_TVSHOWS);
  bReturn |= VideoGetOverviewItems(items, CONTENT_VIDEO_TYPE_MUSICVIDEOS);
  return bReturn;
}

bool CContentAddons::VideoGetOverviewItems(CFileItemList& items, CONTENT_VIDEO_TYPE type)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->Provides(type))
      bReturn |= it->second->VideoGetOverviewItems(items, type);
  return bReturn;
}

bool CContentAddons::VideoGetMovies(CFileItemList& items, const std::string& strGenre /* = "" */, int iYear /* = -1 */)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->Provides(CONTENT_VIDEO_TYPE_MOVIES))
      bReturn |= it->second->VideoGetMovies(items, strGenre, iYear);
  return bReturn;
}

bool CContentAddons::VideoGetGenres(CFileItemList& items, CONTENT_VIDEO_TYPE type)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->Provides(type))
      bReturn |= it->second->VideoGetGenres(items, type);
  return bReturn;
}

bool CContentAddons::VideoGetYears(CFileItemList& items, CONTENT_VIDEO_TYPE type)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->Provides(type))
      bReturn |= it->second->VideoGetYears(items, type);
  return bReturn;
}

bool CContentAddons::VideoGetTvShows(CFileItemList& items, const std::string& strGenre /* = "" */, int iYear /* = -1 */)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->Provides(CONTENT_VIDEO_TYPE_TVSHOWS))
      bReturn |= it->second->VideoGetTvShows(items, strGenre, iYear);
  return bReturn;
}

bool CContentAddons::VideoGetSeasons(CFileItemList& items, const std::string& strTitle, const std::string& strGenre /* = "" */, int iYear /* = -1 */)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->Provides(CONTENT_VIDEO_TYPE_TVSHOWS))
      bReturn |= it->second->VideoGetSeasons(items, strTitle, strGenre, iYear);
  return bReturn;
}

bool CContentAddons::VideoGetEpisodes(CFileItemList& items, const std::string& strTitle, const std::string& strGenre /* = "" */, int iYear /* = -1 */, int iSeason /* = -1 */)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->Provides(CONTENT_VIDEO_TYPE_TVSHOWS))
      bReturn |= it->second->VideoGetEpisodes(items, strTitle, strGenre, iYear, iSeason);
  return bReturn;
}

bool CContentAddons::VideoGetMusicVideos(CFileItemList& items, const std::string& strGenre /* = "" */, int iYear /* = -1 */)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->Provides(CONTENT_VIDEO_TYPE_MUSICVIDEOS))
      bReturn |= it->second->VideoGetMusicVideos(items, strGenre, iYear);
  return bReturn;
}

bool CContentAddons::VideoGetRecentlyAdded(CFileItemList& items, CONTENT_VIDEO_TYPE type)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
    if (it->second->Provides(type))
      bReturn |= it->second->VideoGetRecentlyAdded(items, type);
  return bReturn;
}

bool CContentAddons::GetContextButtons(const CFileItemPtr& item, CContextButtons &buttons)
{
  CONTENT_ADDON addon = GetAddonForPath(item->GetPath());
  if (addon.get())
    return addon->MusicGetContextButtons(item, buttons);
  return false;
}

bool CContentAddons::ClickContextButton(const CFileItemPtr& item, CONTEXT_BUTTON button)
{
  CONTENT_ADDON addon = GetAddonForPath(item->GetPath());
  if (addon.get())
    return addon->MusicClickContextButton(item, button);
  return false;
}

bool CContentAddons::IsSupported(const std::string& strPath) const
{
  CONTENT_ADDON addon = GetAddonForPath(strPath);
  return addon.get() != NULL;
}

bool CContentAddons::IsPlugin(const std::string& strPath)
{
  CURL url(strPath);
  if (url.GetProtocol() == "content")
    return true;
  /* TODO
  if (url.GetProtocol() == "musicdb" && CMusicDatabaseDirectory::IsContentAddonDir(strPath))
    return true;
  */
  return false;
}

CONTENT_ADDON CContentAddons::GetAddonForPath(const std::string& strPath) const
{
  CONTENT_ADDON retval;

  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
  {
    CONTENT_ADDON addon = it->second;
    if (addon->SupportsFile(strPath))
    {
      retval = addon;
      break;
    }
  }

  // If add-on wasn't in the map, try the vector of all managed add-ons
  for (VECADDONS::const_iterator it = m_addons.begin(); it != m_addons.end(); it++)
  {
    CONTENT_ADDON addon = std::dynamic_pointer_cast<CContentAddon>(*it);
    if (addon->SupportsFile(strPath))
    {
      retval = addon;
      break;
    }
  }

  return retval;
}

CONTENT_ADDON CContentAddons::GetAddonByID(const std::string& strID) const
{
  CONTENT_ADDON retval;

  CSingleLock lock(m_critSection);
  for (CONTENT_CLIENTMAP_CITR it = m_addonMap.begin(); it != m_addonMap.end(); it++)
  {
    if (!it->second->ID().compare(strID))
    {
      retval = it->second;
      break;
    }
  }

  // If add-on wasn't in the map, try the vector of all managed add-ons
  for (VECADDONS::const_iterator it = m_addons.begin(); it != m_addons.end(); it++)
  {
    if ((*it)->ID() == strID)
    {
      retval = std::dynamic_pointer_cast<CContentAddon>(*it);
      break;
    }
  }

  return retval;
}
