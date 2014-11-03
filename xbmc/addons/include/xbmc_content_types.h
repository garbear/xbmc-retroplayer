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

#ifndef XBMC_CONTENT_TYPES_H_
#define XBMC_CONTENT_TYPES_H_

/* current content API version */
#define XBMC_CONTENT_API_VERSION "0.1.0"

/* min. content API version */
#define XBMC_CONTENT_MIN_API_VERSION "0.1.0"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __declspec
#define __declspec(X)
#endif
#endif

#include <stdint.h>

#if !defined(__stat64) && defined(TARGET_POSIX)
  #if defined(TARGET_DARWIN) || defined(TARGET_FREEBSD)
    #define __stat64 stat
  #else
    #define __stat64 stat64
  #endif
#endif

#undef ATTRIBUTE_PACKED
#undef PRAGMA_PACK_BEGIN
#undef PRAGMA_PACK_END

#if defined(__GNUC__)
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define ATTRIBUTE_PACKED __attribute__ ((packed))
#define PRAGMA_PACK 0
#endif
#endif

#if !defined(ATTRIBUTE_PACKED)
#define ATTRIBUTE_PACKED
#define PRAGMA_PACK 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum
  {
    CONTENT_ADDON_PROPERTY_TYPE_STRING,
    CONTENT_ADDON_PROPERTY_TYPE_INT,
    CONTENT_ADDON_PROPERTY_TYPE_DOUBLE
  } CONTENT_ADDON_PROPERTY_TYPE;

  typedef struct CONTENT_ADDON_FILE_PROPERTY
  {
    char*                       key;
    CONTENT_ADDON_PROPERTY_TYPE type;
    union
    {
      int    iValue;
      char*  strValue;
      double fValue;
    };
  } ATTRIBUTE_PACKED CONTENT_ADDON_FILE_PROPERTY;

  typedef enum CONTENT_ADDON_TYPE
  {
    CONTENT_ADDON_TYPE_ARTIST,
    CONTENT_ADDON_TYPE_ALBUM,
    CONTENT_ADDON_TYPE_SONG,
    CONTENT_ADDON_TYPE_PLAYLIST,
    CONTENT_ADDON_TYPE_VIDEO,
    CONTENT_ADDON_TYPE_GENRE,
    CONTENT_ADDON_TYPE_YEAR,

    CONTENT_ADDON_TYPE_DIRECTORY,
    CONTENT_ADDON_TYPE_FILE
  } CONTENT_ADDON_TYPE;

  /*!
   * Representation of a file or directory. Types are described below: (TODO actually describe them)
   *
   * CONTENT_ADDON_TYPE_ARTIST:
   *   required string path
   *   required string name
   *   optional string genres
   *   optional string biography
   *   optional string styles
   *   optional string moods
   *   optional string born
   *   optional string formed
   *   optional string died
   *   optional string disbanded
   *   optional string years_active
   *   optional string instruments
   *   optional string thumb
   *   optional string fanart_image
   *
   * CONTENT_ADDON_TYPE_ALBUM:
   *   required string path
   *   required string name
   *   optional int    year
   *   optional string artists
   *   optional string genres
   *   optional int    rating
   *   optional string review
   *   optional string styles
   *   optional string moods
   *   optional string themes
   *   optional string label
   *   optional string type
   *   optional int    compilation
   *   optional int    times_played
   *   optional string thumb
   *   optional string fanart_image
   *
   * CONTENT_ADDON_TYPE_SONG:
   *   required string path
   *   required string name
   *   optional int    track
   *   optional int    duration
   *   optional int    rating
   *   optional string artists
   *   optional int    year
   *   optional string album
   *   optional string album_artists
   *   optional string thumb
   *   optional string fanart_image
   *   optional string provider_icon    (relative to the add-on's path)
   *
   * CONTENT_ADDON_TYPE_PLAYLIST:
   *   required string path
   *   required string name
   *   optional string thumb
   *   optional string fanart_image
   *
   * CONTENT_ADDON_TYPE_DIRECTORY:
   *   required string path
   *   required string name
   *   optional string thumb
   *   optional string fanart_image
   *
   * CONTENT_ADDON_TYPE_FILE:
   *   required string path
   *   required string name
   *   optional string thumb
   *   optional string fanart_image
   *
   * Properties that are found and that are not listed in this list will be
   * added as standard properties to fileitems in XBMC.
   *
   * The following texts will be replaced by XBMC when used as file name:
   *   [logo]   add-on icon
   */
  typedef struct CONTENT_ADDON_FILEITEM
  {
    CONTENT_ADDON_TYPE           type;
    unsigned int                 iSize;
    CONTENT_ADDON_FILE_PROPERTY* properties;
  } ATTRIBUTE_PACKED CONTENT_ADDON_FILEITEM;

  typedef struct CONTENT_ADDON_FILELIST
  {
    unsigned int            iSize;
    CONTENT_ADDON_FILEITEM* items;
  } ATTRIBUTE_PACKED CONTENT_ADDON_FILELIST;

  // set the fanart_image or thumb to CONTENT_ADDON_FILEITEM_LOGO to use the add-on's logo
  #define CONTENT_ADDON_FILEITEM_LOGO "[logo]"

  typedef enum CONTENT_ADDON_PLAYSTATE
  {
    CONTENT_ADDON_PLAYSTATE_PLAY,
    CONTENT_ADDON_PLAYSTATE_PAUSE,
    CONTENT_ADDON_PLAYSTATE_STOP
  } CONTENT_ADDON_PLAYSTATE;

  typedef enum CONTENT_ADDON_AUDIO_FORMAT // TODO !
  {
    CONTENT_FMT_INVALID = -1,

    CONTENT_FMT_U8,

    CONTENT_FMT_S16BE,
    CONTENT_FMT_S16LE,
    CONTENT_FMT_S16NE,

    CONTENT_FMT_S32BE,
    CONTENT_FMT_S32LE,
    CONTENT_FMT_S32NE,

    CONTENT_FMT_S24BE4,
    CONTENT_FMT_S24LE4,
    CONTENT_FMT_S24NE4,    // 24 bits in lower 3 bytes
    CONTENT_FMT_S24NE4MSB, // S32 with bits_per_sample < 32

    CONTENT_FMT_S24BE3,
    CONTENT_FMT_S24LE3,
    CONTENT_FMT_S24NE3, /* S24 in 3 bytes */

    CONTENT_FMT_DOUBLE,
    CONTENT_FMT_FLOAT,

    /* Bitstream formats */
    CONTENT_FMT_AAC,
    CONTENT_FMT_AC3,
    CONTENT_FMT_DTS,
    CONTENT_FMT_EAC3,
    CONTENT_FMT_TRUEHD,
    CONTENT_FMT_DTSHD,
    CONTENT_FMT_LPCM,

    /* planar formats */
    CONTENT_FMT_U8P,
    CONTENT_FMT_S16NEP,
    CONTENT_FMT_S32NEP,
    CONTENT_FMT_S24NE4P,
    CONTENT_FMT_S24NE4MSBP,
    CONTENT_FMT_S24NE3P,
    CONTENT_FMT_DOUBLEP,
    CONTENT_FMT_FLOATP,

    CONTENT_FMT_MAX
  } CONTENT_ADDON_AUDIO_FORMAT;

  typedef struct CONTENT_ADDON_CODEC_INFO
  {
    char                       strCodecName[32];
    CONTENT_ADDON_AUDIO_FORMAT format;
    int                        iSampleRate;
    int                        iChannels;
    int                        iBitsPerSample;
    int                        iBitrate;
  } CONTENT_ADDON_CODEC_INFO;

  /*!
   * @brief content add-on error codes
   */
  typedef enum CONTENT_ERROR
  {
    CONTENT_ERROR_NO_ERROR           = 0,  /*!< @brief no error occurred */
    CONTENT_ERROR_UNKNOWN            = -1, /*!< @brief an unknown error occurred */
    CONTENT_ERROR_NOT_IMPLEMENTED    = -2, /*!< @brief the method that XBMC called is not implemented by the add-on */
    CONTENT_ERROR_REJECTED           = -3, /*!< @brief the command was rejected by the backend */
    CONTENT_ERROR_INVALID_PARAMETERS = -4, /*!< @brief the parameters of the method that was called are invalid for this operation */
    CONTENT_ERROR_FAILED             = -5, /*!< @brief the command failed */
  } CONTENT_ERROR;

  /*!
   * @brief Properties passed to the Create() method of an add-on.
   */
  typedef struct CONTENT_PROPERTIES
  {
    const char* strUserPath;           /*!< @brief path to the user profile */
    const char* strClientPath;         /*!< @brief path to this add-on */
  } CONTENT_PROPERTIES;

  typedef enum CONTENT_TOP100_TYPE
  {
    CONTENT_TOP100_TYPE_ALBUMS,
    CONTENT_TOP100_TYPE_ARTISTS,
    CONTENT_TOP100_TYPE_SONGS
  } CONTENT_TOP100_TYPE;

  typedef enum CONTENT_ADDON_SEARCH_TYPE
  {
    CONTENT_SEARCH_ALL,
    CONTENT_SEARCH_SONGS,
    CONTENT_SEARCH_ALBUMS,
    CONTENT_SEARCH_ARTISTS
  } CONTENT_ADDON_SEARCH_TYPE;

  typedef enum CONTENT_VIDEO_TYPE
  {
    CONTENT_VIDEO_TYPE_MOVIES = 1,
    CONTENT_VIDEO_TYPE_TVSHOWS = 2,
    CONTENT_VIDEO_TYPE_MUSICVIDEOS = 3
  } CONTENT_VIDEO_TYPE;

  typedef void* CONTENT_HANDLE;

  /*!
   * @brief Structure to transfer the methods from xbmc_content_dll.h to XBMC
   */
  typedef struct ContentAddon
  {
    const char*      (__cdecl* GetContentAPIVersion)(void);
    const char*      (__cdecl* GetMininumContentAPIVersion)(void);
    const char*      (__cdecl* GetServerName)(void);
    void             (__cdecl* FreeFileList)(CONTENT_ADDON_FILELIST*);
    bool             (__cdecl* SupportsFile)(const char* strFilename);

    /// @name VFS operations
    ///{
    CONTENT_ERROR    (__cdecl* FileOpen)(const char*, CONTENT_HANDLE*);
    void             (__cdecl* FileClose)(CONTENT_HANDLE);
    unsigned int     (__cdecl* FileRead)(CONTENT_HANDLE, void*, int64_t);
    int              (__cdecl* FileExists)(const char*);
    int              (__cdecl* FileStat)(const char*, struct __stat64* buffer);
    int64_t          (__cdecl* FileSeek)(CONTENT_HANDLE, int64_t, int);
    int64_t          (__cdecl* FileGetPosition)(CONTENT_HANDLE);
    int64_t          (__cdecl* FileGetLength)(CONTENT_HANDLE);
    CONTENT_ERROR    (__cdecl* FileGetDirectory)(CONTENT_ADDON_FILELIST**, const char*);
    ///}

    /// @name Music files
    ///{
    CONTENT_ERROR    (__cdecl* MusicGetPlaylists)(CONTENT_ADDON_FILELIST**);
    CONTENT_ERROR    (__cdecl* MusicGetPlaylist)(CONTENT_ADDON_FILELIST**, const char*);
    CONTENT_ERROR    (__cdecl* MusicGetArtists)(CONTENT_ADDON_FILELIST**);
    CONTENT_ERROR    (__cdecl* MusicGetAlbums)(CONTENT_ADDON_FILELIST**, const char*);
    CONTENT_ERROR    (__cdecl* MusicGetSongs)(CONTENT_ADDON_FILELIST**, const char*, const char*);
    CONTENT_ERROR    (__cdecl* MusicGetTop100)(CONTENT_ADDON_FILELIST**, CONTENT_TOP100_TYPE);
    CONTENT_ERROR    (__cdecl* MusicGetOverviewItems)(CONTENT_ADDON_FILELIST**);
    CONTENT_ERROR    (__cdecl* MusicSearch)(CONTENT_ADDON_FILELIST**, const char*, CONTENT_ADDON_SEARCH_TYPE);
    ///}

    /// @name Video files
    ///{
    CONTENT_ERROR    (__cdecl* VideoGetOverviewItems)(CONTENT_ADDON_FILELIST**, CONTENT_VIDEO_TYPE);
    CONTENT_ERROR    (__cdecl* VideoGetGenres)(CONTENT_ADDON_FILELIST**, CONTENT_VIDEO_TYPE);
    CONTENT_ERROR    (__cdecl* VideoGetYears)(CONTENT_ADDON_FILELIST**, CONTENT_VIDEO_TYPE);
    CONTENT_ERROR    (__cdecl* VideoGetRecentlyAdded)(CONTENT_ADDON_FILELIST**, CONTENT_VIDEO_TYPE);
    CONTENT_ERROR    (__cdecl* VideoGetMovies)(CONTENT_ADDON_FILELIST**, const char*, int);
    CONTENT_ERROR    (__cdecl* VideoGetTvShows)(CONTENT_ADDON_FILELIST**, const char*, int);
    CONTENT_ERROR    (__cdecl* VideoGetSeasons)(CONTENT_ADDON_FILELIST**, const char*, const char*, int);
    CONTENT_ERROR    (__cdecl* VideoGetEpisodes)(CONTENT_ADDON_FILELIST**, const char*, const char*, int, int);
    CONTENT_ERROR    (__cdecl* VideoGetMusicVideos)(CONTENT_ADDON_FILELIST**, const char*, int);
    ///}

    /// @name Music codec
    ///{
    CONTENT_ERROR    (__cdecl* MusicOpenFile)(const char*);
    CONTENT_ERROR    (__cdecl* MusicPreloadFile)(const char*);
    void             (__cdecl* MusicCloseFile)(void);
    CONTENT_ERROR    (__cdecl* MusicPause)(int);
    CONTENT_ERROR    (__cdecl* MusicGetCodecInfo)(const char*, CONTENT_ADDON_CODEC_INFO*);
    int64_t          (__cdecl* MusicSeek)(int64_t);
    int              (__cdecl* MusicReadPCM)(unsigned char*, int, int*);
    ///}
  } ContentClient;

#ifdef __cplusplus
}
#endif

#endif /* XBMC_CONTENT_TYPES_H_ */
