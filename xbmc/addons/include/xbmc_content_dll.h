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

#ifndef XBMC_CONTENT_DLL_H_
#define XBMC_CONTENT_DLL_H_

#include "xbmc_addon_dll.h"
#include "xbmc_content_types.h"

#ifdef __cplusplus
extern "C" {
#endif

  const char* GetContentAPIVersion(void);

  /*!
   * Get the XBMC_CONTENT_MIN_API_VERSION that was used to compile this add-on.
   * Used to check if this add-on is compatible with XBMC.
   * @return The XBMC_CONTENT_MIN_API_VERSION that was used to compile this add-on.
   * @remarks Valid implementation required.
   */
  const char* GetMininumContentAPIVersion(void);

  /*!
   * @return The name reported by the server (if any) that will be displayed in the UI.
   * @remarks Valid implementation required.
   */
  const char* GetServerName(void);

  /*!
   * Check whether this add-on supports opening and reading from the given file.
   * This method only has to be implemented when this add-on can open files
   * that are not matched by filetypes in addon.xml, that are not provided by
   * methods under CONTENT_ADDON_MUSIC_FILES and when the add-on implements
   * CONTENT_ADDON_MUSIC_CODEC methods. Return false otherwise.
   * @param strFilename
   * @return True when this add-on can open the file, false otherwise.
   */
  bool        SupportsFile(const char* strFilename);

  /*!
   * @note #define CONTENT_ADDON_FILES before including xbmc_content_dll.h in the add-on if the add-on provides files
   *       and add provides_files="true" to the xbmc.content extension point node in addon.xml
   */
  ///{
#ifdef CONTENT_ADDON_FILES
  /*!
   * Open a file to be read by FileRead
   * @param strFileName The file to open
   * @param handle The handle for this file, set by the add-on, that is passed to file operations.
   * @return CONTENT_ERROR_NO_ERROR when the file has been opened successfully.
   */
  CONTENT_ERROR FileOpen(const char* strFileName, CONTENT_HANDLE* handle);

  /*!
   * Close a file opened by FileOpen, and invalidate the handle
   * @param handle The handle to close
   */
  void          FileClose(CONTENT_HANDLE handle);

  /*!
   * Read raw bytes from a file that has been opened with FileOpen
   * @param handle The handle assigned by FileOpen
   * @param pBuffer The buffer to write the data to
   * @param iBufLen The maximum size to read
   * @return The number of bytes that were added to the buffer
   */
  unsigned int  FileRead(CONTENT_HANDLE handle, void* pBuffer, int64_t iBufLen);

  /*!
   * Check whether the add-on can open a file
   * @param strFileName The file to check
   * @return 1 when the file can be opened, 0 otherwise
   */
  int           FileExists(const char* strFileName);

  /*!
   * Perform a stat operation on the specified file
   * @param strFileName The file to stat
   * @param buffer The zero-initialized stat structure
   * @return 0 is returned on success, -1 is returned on error
   */
  int           FileStat(const char* strFileName, struct __stat64* buffer);

  /*!
   * Seek to the given position
   * @param handle The handle of the file to seek
   * @param iFilePosition The position to seek to
   * @param iWhence Seek type. See stdio.h for possible value
   * @return The new file position
   */
  int64_t       FileSeek(CONTENT_HANDLE handle, int64_t iFilePosition, int iWhence);

  /*!
   * Get the current position
   * @param handle The handle of the file to get the position for
   * @return The position. -1 when eof or not found
   */
  int64_t       FileGetPosition(CONTENT_HANDLE handle);

  /*!
   * Total number of bytes in an open file
   * @param handle The handle of the file to get the length for
   * @return The total length or -1 when not found
   */
  int64_t       FileGetLength(CONTENT_HANDLE handle);

  /*!
   * Get the files in a directory
   * @param directory The directory contents. Must be freed by calling FreeFileList when done
   * @param strPath The directory to get
   * @return CONTENT_ERROR_NO_ERROR if the files were assigned successfully
   */
  CONTENT_ERROR FileGetDirectory(CONTENT_ADDON_FILELIST** directory, const char* strPath);
#endif

  /*!
   * @note #define CONTENT_ADDON_MUSIC_FILES before including xbmc_content_dll.h in the add-on if the add-on provides files to be integrated into the music database
   *       and add provides_music_files="true" to the xbmc.content extension point node in addon.xml
   * @todo implement support for add-ons with multiple extension points in xbmc...
   */
  ///{
#if defined(CONTENT_ADDON_MUSIC_FILES) || defined(CONTENT_ADDON_VIDEO_FILES) || defined(CONTENT_ADDON_FILES)
  /*!
   * Free a file list allocated by the add-on
   * @param items The file list to free
   */
  void FreeFileList(CONTENT_ADDON_FILELIST* items);
#endif

#ifdef CONTENT_ADDON_MUSIC_FILES
  /*!
   * Get all playlists
   * @param playlists The requested playlists. Must be freed by calling FreeFileList when done
   * @return CONTENT_ERROR_NO_ERROR if the playlists were assigned successfully
   */
  CONTENT_ERROR MusicGetPlaylists(CONTENT_ADDON_FILELIST** playlists);

  /*!
   * Get the songs in a playlist returned by MusicGetPlaylists
   * @param contents The songs in the playlist. Must be freed by calling FreeFileList when done
   * @param strPlaylist The path or name of the playlist that was returned by MusicGetPlaylists
   * @return CONTENT_ERROR_NO_ERROR if the songs were assigned successfully
   */
  CONTENT_ERROR MusicGetPlaylist(CONTENT_ADDON_FILELIST** contents, const char* strPlaylist);

  /*!
   * Get all artists
   * @param artists The requested artists. Must be freed by calling FreeFileList when done
   * @return CONTENT_ERROR_NO_ERROR if the artists were assigned successfully
   */
  CONTENT_ERROR MusicGetArtists(CONTENT_ADDON_FILELIST** artists);

  /*!
   * Get all albums
   * @param contents The requested albums. Must be freed by calling FreeFileList when done
   * @param strArtist The name or id that the add-on can recognise of the artist to only provide albums of that artist. NULL to provide all albums
   * @return CONTENT_ERROR_NO_ERROR if the albums were assigned successfully
   */
  CONTENT_ERROR MusicGetAlbums(CONTENT_ADDON_FILELIST** contents, const char* strArtist);

  /*!
   * Get all songs
   * @param contents The requested songs. Must be freed by calling FreeFileList when done
   * @param strArtist The name or id that the add-on can recognise of the artist to only provide songs of that artist. NULL to provide all songs
   * @param strAlbum The name or id that the add-on can recognise of the albums to only provide songs of that album. NULL to provide all songs
   * @return CONTENT_ERROR_NO_ERROR if the songs were assigned successfully
   */
  CONTENT_ERROR MusicGetSongs(CONTENT_ADDON_FILELIST** contents, const char* strArtist, const char* strAlbum);

  /*!
   * Get top 100 songs, albums or artist
   * @param contents The requested items. Must be freed by calling FreeFileList when done
   * @param type CONTENT_TOP100_TYPE_ALBUMS to get the top 100 albums, CONTENT_TOP100_TYPE_ARTISTS to get the top 100 artists or CONTENT_TOP100_TYPE_SONGS to get the top 100 songs
   * @return CONTENT_ERROR_NO_ERROR if the items were assigned successfully
   */
  CONTENT_ERROR MusicGetTop100(CONTENT_ADDON_FILELIST** contents, CONTENT_TOP100_TYPE type);

  /*!
   * Get items to be added to the top level music window in XBMC
   * @param items The requested items. Must be freed by calling FreeFileList when done
   * @return CONTENT_ERROR_NO_ERROR if the items were assigned successfully
   */
  CONTENT_ERROR MusicGetOverviewItems(CONTENT_ADDON_FILELIST** items);

  /*!
   * Search for artist, albums or songs
   * @param items The requested items. Must be freed by calling FreeFileList when done
   * @param strQuery The search query
   * @param type The type of search to perform
   * @return CONTENT_ERROR_NO_ERROR if the items were assigned successfully
   */
  CONTENT_ERROR MusicSearch(CONTENT_ADDON_FILELIST** items, const char* strQuery, CONTENT_ADDON_SEARCH_TYPE type);
#endif

#ifdef CONTENT_ADDON_VIDEO_FILES
  /*!
   * Get items to be added to the top level window of the specified video type in XBMC
   * @param items The requested items. Must be freed by calling FreeFileList when done
   * @param type The type of video items (movies, tv shows, music videos)
   * @return CONTENT_ERROR_NO_ERROR if the items were assigned successfully
   */
  CONTENT_ERROR VideoGetOverviewItems(CONTENT_ADDON_FILELIST** items, CONTENT_VIDEO_TYPE type);

  /*!
   * Get all genres of the specified video type
   * @param contents The requested items. Must be freed by calling FreeFileList when done
   * @param type The type of video items (movies, tv shows, music videos)
   * @return CONTENT_ERROR_NO_ERROR if the items were assigned successfully
   */
  CONTENT_ERROR VideoGetGenres(CONTENT_ADDON_FILELIST** contents, CONTENT_VIDEO_TYPE type);

  /*!
   * Get all years of the specified video type
   * @param contents The requested items. Must be freed by calling FreeFileList when done
   * @param type The type of video items (movies, tv shows, music videos)
   * @return CONTENT_ERROR_NO_ERROR if the items were assigned successfully
   */
  CONTENT_ERROR VideoGetYears(CONTENT_ADDON_FILELIST** contents, CONTENT_VIDEO_TYPE type);

  /*!
   * Get "recently added" items of the specified video type. Can be used for new releases, newly discovered items, etc.
   * Recently added tv shows must return a list of episodes, not tv shows.
   * @param contents The requested items. Must be freed by calling FreeFileList when done
   * @param type The type of video items (movies, tv shows, music videos)
   * @return CONTENT_ERROR_NO_ERROR if the items were assigned successfully
   */
  CONTENT_ERROR VideoGetRecentlyAdded(CONTENT_ADDON_FILELIST** contents, CONTENT_VIDEO_TYPE type);

  /*!
   * Get movies
   * @param contents The requested movies. Must be freed by calling FreeFileList when done
   * @param strGenre The genre name or id that the add-on can recognise to only provide movies of that genre. NULL to provide all movies
   * @param strAlbum The year to only provide movies of that year. -1 to provide all movies
   * @return CONTENT_ERROR_NO_ERROR if the movies were assigned successfully
   */
  CONTENT_ERROR VideoGetMovies(CONTENT_ADDON_FILELIST** contents, const char* strGenre, int iYear);

  /*!
   * Get tv shows
   * @param contents The requested tv shows. Must be freed by calling FreeFileList when done
   * @param strGenre The genre name or id that the add-on can recognise to only provide tv shows of that genre. NULL to provide all tv shows
   * @param strAlbum The year to only provide tv shows of that year. -1 to provide all tv shows
   * @return CONTENT_ERROR_NO_ERROR if the tv shows were assigned successfully
   */
  CONTENT_ERROR VideoGetTvShows(CONTENT_ADDON_FILELIST** contents, const char* strGenre, int iYear);

  /*!
   * Get tv show seasons
   * @param contents The requested tv show seasons. Must be freed by calling FreeFileList when done
   * @param strTitle The title name or id that the add-on can recognise to only provide seasons of that title. NULL to provide all seasons
   * @param strGenre The genre name or id that the add-on can recognise to only provide seasons of that genre. NULL to provide all seasons
   * @param strAlbum The year to only provide seasons of that year. -1 to provide all seasons
   * @return CONTENT_ERROR_NO_ERROR if the seasons were assigned successfully
   */
  CONTENT_ERROR VideoGetSeasons(CONTENT_ADDON_FILELIST** contents, const char* strTitle, const char* strGenre, int iYear);

  /*!
   * Get tv show episodes
   * @param contents The requested episodes. Must be freed by calling FreeFileList when done
   * @param strTitle The title name or id that the add-on can recognise to only provide episodes of that title. NULL to provide all episodes
   * @param strGenre The genre name or id that the add-on can recognise to only provide episodes of that genre. NULL to provide all episodes
   * @param strAlbum The year to only provide episodes of that year. -1 to provide all episodes
   * @return CONTENT_ERROR_NO_ERROR if the episodes were assigned successfully
   */
  CONTENT_ERROR VideoGetEpisodes(CONTENT_ADDON_FILELIST** contents, const char* strTitle, const char* strGenre, int iYear, int iSeason);

  /*!
   * Get music videos
   * @param contents The requested music videos. Must be freed by calling FreeFileList when done
   * @param strGenre The genre name or id that the add-on can recognise to only provide music videos of that genre. NULL to provide all music videos
   * @param strAlbum The year to only provide music videos of that year. -1 to provide all music videos
   * @return CONTENT_ERROR_NO_ERROR if the music videos were assigned successfully
   */
  CONTENT_ERROR VideoGetMusicVideos(CONTENT_ADDON_FILELIST** contents, const char* strGenre, int iYear);
#endif

  ///}

  /*!
   * @note #define CONTENT_ADDON_MUSIC_CODEC before including xbmc_content_dll.h in the add-on if the add-on provides a music codec
   *       and add provides_music_codec="true" to the xbmc.content extension point node in addon.xml
   */
  ///{
#ifdef CONTENT_ADDON_MUSIC_CODEC
  /*!
   * Open a file to be read by MusicReadPCM
   * @param strPath The path to the file to open
   * @return CONTENT_ERROR_NO_ERROR when the file has been opened successfully
   * @todo have this method return a handle instead, and use that handle in the methods below, so we can support concurrent streams
   */
  CONTENT_ERROR MusicOpenFile(const char* strPath);

  /*!
   * Preload a file that will be opened by MusicOpenFile, if the add-on supports this
   * @param strPath The path to the file that will be opened
   * @return CONTENT_ERROR_NO_ERROR if the file was preloaded, or when the add-on does not support this
   */
  CONTENT_ERROR MusicPreloadFile(const char* strPath);

  /*!
   * Close a file opened by MusicOpenFile and flush buffers
   */
  void          MusicCloseFile(void);

  /*!
   * Called by XBMC when the player is paused or unpaused
   * @param bPause 1 when paused, 0 when unpaused
   * @return CONTENT_ERROR_NO_ERROR if the add-on handled the (un)pause operation successfully
   */
  CONTENT_ERROR MusicPause(int bPause);

  /*!
   * Provide codec information for the given path
   * @param strPath The path to get codec information for
   * @param info The requested codec information
   * @return CONTENT_ERROR_NO_ERROR if the codec information was assigned successfully
   */
  CONTENT_ERROR MusicGetCodecInfo(const char* strPath, CONTENT_ADDON_CODEC_INFO* info);

  /*!
   * Seek to the given timestamp
   * @param iSeekTime The timestamp to seek to, in milliseconds
   * @return The timestamp in milliseconds after the seek operation completed
   */
  int64_t       MusicSeek(int64_t iSeekTime);

  /*!
   * Read raw PCM data from a file that has been opened with MusicOpenFile
   * @param pBuffer The buffer to write the data to
   * @param iSize The maximum size to read
   * @param iActualSize The number of bytes that were added to the buffer
   * @return 0 when read successfully, -1 to indicate EOF
   */
  int           MusicReadPCM(unsigned char* pBuffer, int iSize, int* iActualSize);
#endif
  ///}

  /*!
   * Called by XBMC to assign the function pointers of this add-on to pClient.
   * @param pClient The struct to assign the function pointers to.
   */
  void __declspec(dllexport) get_addon(ContentClient* pClient)
  {
    pClient->GetContentAPIVersion        = GetContentAPIVersion;
    pClient->GetMininumContentAPIVersion = GetMininumContentAPIVersion;
    pClient->GetServerName               = GetServerName;
    pClient->SupportsFile                = SupportsFile;

#if defined(CONTENT_ADDON_MUSIC_FILES) || defined(CONTENT_ADDON_VIDEO_FILES) || defined(CONTENT_ADDON_FILES)
    pClient->FreeFileList                = FreeFileList;
#endif

#ifdef CONTENT_ADDON_FILES
    pClient->FileOpen         = FileOpen;
    pClient->FileClose        = FileClose;
    pClient->FileRead         = FileRead;
    pClient->FileExists       = FileExists;
    pClient->FileStat         = FileStat;
    pClient->FileSeek         = FileSeek;
    pClient->FileGetPosition  = FileGetPosition;
    pClient->FileGetLength    = FileGetLength;
    pClient->FileGetDirectory = FileGetDirectory;
#endif

#ifdef CONTENT_ADDON_MUSIC_FILES
    pClient->MusicGetPlaylists           = MusicGetPlaylists;
    pClient->MusicGetPlaylist            = MusicGetPlaylist;
    pClient->MusicGetArtists             = MusicGetArtists;
    pClient->MusicGetSongs               = MusicGetSongs;
    pClient->MusicGetAlbums              = MusicGetAlbums;
    pClient->MusicGetTop100              = MusicGetTop100;
    pClient->MusicGetOverviewItems       = MusicGetOverviewItems;
    pClient->MusicSearch                 = MusicSearch;
#endif

#ifdef CONTENT_ADDON_VIDEO_FILES
    pClient->VideoGetOverviewItems       = VideoGetOverviewItems;
    pClient->VideoGetGenres              = VideoGetGenres;
    pClient->VideoGetYears               = VideoGetYears;
    pClient->VideoGetRecentlyAdded       = VideoGetRecentlyAdded;
    pClient->VideoGetMovies              = VideoGetMovies;
    pClient->VideoGetTvShows             = VideoGetTvShows;
    pClient->VideoGetSeasons             = VideoGetSeasons;
    pClient->VideoGetEpisodes            = VideoGetEpisodes;
    pClient->VideoGetMusicVideos         = VideoGetMusicVideos;
#endif

#ifdef CONTENT_ADDON_MUSIC_CODEC
    pClient->MusicOpenFile               = MusicOpenFile;
    pClient->MusicPreloadFile            = MusicPreloadFile;
    pClient->MusicCloseFile              = MusicCloseFile;
    pClient->MusicPause                  = MusicPause;
    pClient->MusicGetCodecInfo           = MusicGetCodecInfo;
    pClient->MusicSeek                   = MusicSeek;
    pClient->MusicReadPCM                = MusicReadPCM;
#endif
  };

#ifdef __cplusplus
};
#endif

#endif /* XBMC_CONTENT_DLL_H_ */
