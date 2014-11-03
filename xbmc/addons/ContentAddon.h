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

#include "addons/Addon.h"
#include "addons/AddonDll.h"
#include "addons/DllContentAddon.h"
#include "interfaces/IAnnouncer.h"
#include "dialogs/GUIDialogContextMenu.h"

class CAEChannelInfo;
class AddonFileItem;

namespace ADDON
{
  #define CONTENT_NODE             "content://"
  #define MUSIC_VIRTUAL_NODE       "musicdb://content/"
  #define MUSIC_PLAYLIST           "playlist"
  #define MUSIC_ARTIST             "artist"
  #define MUSIC_ALBUM              "album"
  #define MUSIC_SONG               "song"
  #define MUSIC_TOP100             "top100"
  #define VIDEO_VIRTUAL_NODE       "videodb://content/"
  #define VIDEO_MOVIES             "movies"
  #define VIDEO_RECENT_MOVIES      "recentmovies"
  #define VIDEO_TVSHOWS            "tvshows"
  #define VIDEO_RECENT_TVSHOWS     "recenttvshows"
  #define VIDEO_MUSICVIDEOS        "musicvideos"
  #define VIDEO_RECENT_MUSICVIDEOS "recentmusicvideos"
  #define VIDEO_GENRE              "genre"
  #define VIDEO_TITLE              "title"
  #define VIDEO_YEAR               "year"

  class CAddonCallbacksContent;

  class CContentAddon : public ADDON::CAddonDll<DllContentAddon, ContentAddon, CONTENT_PROPERTIES>
  {
    friend class CAddonCallbacksContent;

  public:
    CContentAddon(const ADDON::AddonProps& props);
    CContentAddon(const cp_extension_t *ext);
    ~CContentAddon(void);

    ADDON_STATUS Create(void);
    bool         DllLoaded(void) const;
    void         Destroy(void);
    void         ReCreate(void);
    bool         ReadyToUse(void) const;
    void         Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char* sender, const char* message, const CVariant& data);

    bool         SupportsFile(const std::string& strPath) const;
    bool         SupportsFileType(const std::string& strType) const;

    bool         OnDemand(void) const { return !m_bProvidesMusicFiles && ProvidesMusicCodec(); }
    bool         ProvidesMusicCodec(void) const { return m_bReadyToUse && m_bProvidesMusicCodec; }
    bool         ProvidesMusicFiles(void) const { return m_bReadyToUse && m_bProvidesMusicFiles; }
    bool         ProvidesMusicPlaylists(void) const { return m_bReadyToUse && m_bProvidesMusicPlaylists; }
    bool         ProvidesMusicTop100(void) const { return m_bReadyToUse && m_bProvidesMusicTop100; }
    bool         ProvidesMusicSearch(void) const { return m_bReadyToUse && m_bProvidesMusicSearch; }
    bool         SupportsConcurrentStreams(void) const { return m_bReadyToUse && m_bSupportsConcurrentStreams; }

    bool         ProvidesVideoCodec(void) const { return m_bReadyToUse && m_bProvidesVideoCodec; }
    /*!
     * Returns true if the video type is provided
     *  - Music:         Provides(CONTENT_VIDEO_TYPE_MOVIES);
     *  - TV Shows:      Provides(CONTENT_VIDEO_TYPE_TVSHOWS);
     *  - Music Videos:  Provides(CONTENT_VIDEO_TYPE_MUSICVIDEOS);
     * @param type the content type
     */
    bool         Provides(CONTENT_VIDEO_TYPE type) const;
    bool         ProvidesRecentlyAdded(CONTENT_VIDEO_TYPE type) const;
    bool         ProvidesFiles(void) const { return m_bReadyToUse && m_bProvidesFiles; }

    bool         FileOpen(const std::string& strFileName, CONTENT_HANDLE* handle);
    void         FileClose(CONTENT_HANDLE handle);
    unsigned int FileRead(CONTENT_HANDLE handle, void* pBuffer, int64_t iBufLen);
    bool         FileExists(const std::string& strFileName);
    int          FileStat(const std::string& strFileName, struct __stat64* buffer);
    int64_t      FileSeek(CONTENT_HANDLE handle, int64_t iFilePosition, int iWhence);
    int64_t      FileGetPosition(CONTENT_HANDLE handle);
    int64_t      FileGetLength(CONTENT_HANDLE handle);
    bool         FileGetDirectory(CFileItemList& items, const std::string& strPath);

    const char*  GetServerName(void);
    bool         MusicGetOverview(CFileItemList& items);
    bool         MusicGetTop100Overview(CFileItemList& items);
    bool         MusicGetPlaylists(CFileItemList& items);
    bool         MusicGetPlaylistContent(CFileItemList& items, const std::string& strName);
    bool         MusicGetArtists(CFileItemList& items);
    bool         MusicGetAlbums(CFileItemList& items, const std::string& strArtist);
    bool         MusicGetSongs(CFileItemList& items, const std::string& strArtist, const std::string& strAlbum);
    bool         MusicGetTop100(CFileItemList& items, CONTENT_TOP100_TYPE type);
    bool         MusicGetOverviewItems(CFileItemList& items);
    bool         MusicSearch(CFileItemList& items, const std::string& query, CONTENT_ADDON_SEARCH_TYPE type = CONTENT_SEARCH_ALL);
    bool         MusicGetContextButtons(const CFileItemPtr& item, CContextButtons &buttons);
    bool         MusicClickContextButton(const CFileItemPtr& item, CONTEXT_BUTTON button);
    bool         MusicOpenFile(const std::string& strPath);
    bool         MusicPreloadFile(const std::string& strPath);
    void         MusicCloseFile(void);
    bool         MusicGetCodecInfo(const std::string& strPath, CONTENT_ADDON_CODEC_INFO* info);
    void         MusicPause(bool bPause);
    int64_t      MusicSeek(int64_t iSeekTime);
    int          MusicReadPCM(BYTE* pBuffer, int size, int* actualsize);
    std::string   MusicGetPlaylistName(const std::string& strPlaylist) const;
    std::string   MusicGetAlbumName(const std::string& strArtist, const std::string& strAlbum) const;
    std::string   MusicGetArtistName(const std::string& strArtist) const;
    
    // TODO: API calls for video codecs and video files
    // TODO: Final GUI integration: context menu fixes and OnInfo() hooks, etc.
    bool         VideoGetGeneralOverview(CFileItemList& items);
    bool         VideoGetOverview(CFileItemList& items, CONTENT_VIDEO_TYPE type);
    bool         VideoGetOverviewItems(CFileItemList& items, CONTENT_VIDEO_TYPE type);
    bool         VideoGetGenres(CFileItemList& items, CONTENT_VIDEO_TYPE type);
    bool         VideoGetYears(CFileItemList& items, CONTENT_VIDEO_TYPE type);
    bool         VideoGetRecentlyAdded(CFileItemList& items, CONTENT_VIDEO_TYPE type);
    bool         VideoGetMovies(CFileItemList& items, const std::string& strGenre = "", int iYear = -1);
    bool         VideoGetTvShows(CFileItemList& items, const std::string& strGenre = "", int iYear = -1);
    bool         VideoGetSeasons(CFileItemList& items, const std::string& strTitle = "", const std::string& strGenre = "", int iYear = -1);
    bool         VideoGetEpisodes(CFileItemList& items, const std::string& strTitle = "", const std::string& strGenre = "", int iYear = -1, int iSeason = -1);
    bool         VideoGetMusicVideos(CFileItemList& items, const std::string& strGenre = "", int iYear = -1);

  private:
    std::string   ContentBuildPath(const std::string& strPath);
    std::string   MusicBuildPath(CONTENT_ADDON_TYPE type, const std::string& strFilename, const std::string& strArtist = "", const std::string& strAlbum = "") const;
    std::string   VideoBuildPath(CONTENT_ADDON_TYPE type, const std::string& strFilename, CONTENT_VIDEO_TYPE videoType) const;
    std::string   GetFilename(const std::string& strPath) const;
    bool CreateOnDemand(void);
    void SetPlaystate(CONTENT_ADDON_PLAYSTATE newState);
    void FreeFileList(CONTENT_ADDON_FILELIST* items);
    void LogException(const std::exception& e, const char* strFunctionName) const;
    void ResetProperties(void);
    bool GetAddonCapabilities(void);

    /*!
     * Add common properties and properties that have not been used to a fileitem
     * @param properties The properties
     * @param fileItem The item to add the properties to
     */
    void AddCommonProperties(AddonFileItem* file, CFileItemPtr& fileItem);

    void ReadFilePlaylist(AddonFileItem* file, CFileItemList& fileList);
    void ReadFileArtist(AddonFileItem* file, CFileItemList& fileList);
    void ReadFileAlbum(AddonFileItem* file, CFileItemList& fileList, const std::string& strArtist = "");
    void ReadFileSong(AddonFileItem* file, CFileItemList& fileList, const std::string& strArtist = "", const std::string& strAlbum = "");
    void ReadFileVideo(AddonFileItem* file, CFileItemList& fileList, CONTENT_VIDEO_TYPE type);
    void ReadFileGenre(AddonFileItem* file, CFileItemList& fileList, CONTENT_VIDEO_TYPE type);
    void ReadFileYear(AddonFileItem* file, CFileItemList& fileList, CONTENT_VIDEO_TYPE type);

    void ReadFileDirectory(AddonFileItem* file, CFileItemList& fileList);
    void ReadFileFile(AddonFileItem* file, CFileItemList& fileList);

    void ReadMusicFiles(const CONTENT_ADDON_FILELIST& addonItems, CFileItemList& fileList, const std::string& strArtist = "", const std::string& strAlbum = "");
    void ReadVideoFiles(const CONTENT_ADDON_FILELIST& addonItems, CFileItemList& fileList, CONTENT_VIDEO_TYPE type);

    CCriticalSection                                        m_critSection;
    AddonVersion                                            m_apiVersion;
    bool                                                    m_bReadyToUse;
    CONTENT_ADDON_PLAYSTATE                                 m_playState;
    std::map<std::string, std::string>                        m_playlistNames;
    std::map<std::string, std::string>                        m_artistNames;
    std::map<std::string, std::map<std::string, std::string> > m_albumNames;
    std::string                                             m_strUserPath;
    std::string                                             m_strClientPath;
    std::vector<std::string>                                m_fileTypes;
    bool                                                    m_bOnDemand;
    bool                                                    m_bProvidesMusicCodec;
    bool                                                    m_bProvidesMusicFiles;
    bool                                                    m_bProvidesMusicPlaylists;
    bool                                                    m_bProvidesMusicTop100;
    bool                                                    m_bProvidesMusicSearch;
    bool                                                    m_bSupportsConcurrentStreams;
    bool                                                    m_bProvidesVideoCodec;
    bool                                                    m_bProvidesMovies;
    bool                                                    m_bProvidesTvShows;
    bool                                                    m_bProvidesMusicVideos;
    bool                                                    m_bProvidesRecentlyAddedMovies;
    bool                                                    m_bProvidesRecentlyAddedTvShows;
    bool                                                    m_bProvidesRecentlyAddedMusicVideos;
    bool                                                    m_bProvidesFiles;
    bool                                                    m_bFileOpen;
  };
}
