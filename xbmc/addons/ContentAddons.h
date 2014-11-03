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

#include "ContentAddon.h"
#include "dialogs/GUIDialogContextMenu.h"
#include "cores/paplayer/ICodec.h"
#include "filesystem/MusicDatabaseDirectory/DirectoryNode.h"
#include "include/xbmc_content_types.h"
#include "BinaryAddonManager.h"

namespace ADDON
{
  typedef std::map<std::string, std::shared_ptr<CContentAddon> >                 CONTENT_CLIENTMAP;
  typedef std::map<std::string, std::shared_ptr<CContentAddon> >::iterator       CONTENT_CLIENTMAP_ITR;
  typedef std::map<std::string, std::shared_ptr<CContentAddon> >::const_iterator CONTENT_CLIENTMAP_CITR;
  typedef std::shared_ptr<CContentAddon>                                         CONTENT_ADDON;

  class CContentAddons :
    public CBinaryAddonManager<CContentAddon>
  {
  public:
    static CContentAddons& Get(void);
    virtual ~CContentAddons(void) {};

    bool MusicHasAvailableAddons(void) const;
    bool SupportsConcurrentStreams(const std::string& strPath) const;
    bool MusicGetAddons(CFileItemList& items) const;
    bool MusicGetOverviewItems(CFileItemList& items);
    bool MusicGetArtists(CFileItemList& items);
    bool MusicGetAlbums(CFileItemList& items, const std::string& strArtistName);
    bool MusicGetSongs(CFileItemList& items, const std::string& strArtistName, const std::string& strAlbumName);
    bool MusicGetPlaylists(CFileItemList& items);
    bool MusicGetTop100(CFileItemList& items, CONTENT_TOP100_TYPE type);
    bool MusicSearch(CFileItemList& items, const std::string& query);

    bool VideoHasAvailableAddons(void) const;
    bool VideoHasAvailableAddons(CONTENT_VIDEO_TYPE videoType) const;
    bool VideoGetAddons(CFileItemList& items) const;
    bool VideoGetAddons(CFileItemList& items, CONTENT_VIDEO_TYPE videoType) const;
    bool VideoGetAllOverviewItems(CFileItemList& items);
    bool VideoGetOverviewItems(CFileItemList& items, CONTENT_VIDEO_TYPE type);
    bool VideoGetMovies(CFileItemList& items, const std::string& strGenre = "", int iYear = -1);
    bool VideoGetGenres(CFileItemList& items, CONTENT_VIDEO_TYPE type);
    bool VideoGetYears(CFileItemList& items, CONTENT_VIDEO_TYPE type);
    bool VideoGetTvShows(CFileItemList& items, const std::string& strGenre = "", int iYear = -1);
    bool VideoGetSeasons(CFileItemList& items, const std::string& strTitle, const std::string& strGenre = "", int iYear = -1);
    bool VideoGetEpisodes(CFileItemList& items, const std::string& strTitle, const std::string& strGenre = "", int iYear = -1, int iSeason = -1);
    bool VideoGetMusicVideos(CFileItemList& items, const std::string& strGenre = "", int iYear = -1);
    bool VideoGetRecentlyAdded(CFileItemList& items, CONTENT_VIDEO_TYPE type);

    bool GetContextButtons(const CFileItemPtr& item, CContextButtons &buttons);
    bool ClickContextButton(const CFileItemPtr& item, CONTEXT_BUTTON button);

    CONTENT_ADDON GetAddonForPath(const std::string& strPath) const;
    CONTENT_ADDON GetAddonByID(const std::string& strID) const;
    bool IsSupported(const std::string& strPath) const;
    /*!
     * \brief Returns true if the path is to a virtual content add-on item.
     */
    static bool IsPlugin(const std::string& strFile);
  private:
    CContentAddons(void) :
      CBinaryAddonManager<CContentAddon>(ADDON_CONTENTDLL) {}

    bool GetAddons(CFileItemList& items, bool bMusic, const std::string& basePath) const;
  };
}
