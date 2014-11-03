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

//TODO implement todo's

#include "ContentAddon.h"
#include "ContentAddons.h"
#include "music/Song.h"
#include "music/Artist.h"
#include "music/Album.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "URL.h"
#include "MediaSource.h"
#include "cores/AudioEngine/Utils/AEChannelInfo.h"
#include "interfaces/AnnouncementManager.h"
#include "ApplicationMessenger.h"
#include "filesystem/MusicDatabaseDirectory/DirectoryNode.h"
#include "filesystem/VideoDatabaseDirectory/DirectoryNode.h"
#include "include/xbmc_file_utils.hpp"
#include "video/VideoInfoTag.h"

using namespace ADDON;
using namespace ANNOUNCEMENT;
using namespace XFILE;
using namespace std;

CContentAddon::CContentAddon(const AddonProps& props) :
    CAddonDll<DllContentAddon, ContentAddon, CONTENT_PROPERTIES>(props),
    m_apiVersion("0.0.0"),
    m_bReadyToUse(false),
    m_playState(CONTENT_ADDON_PLAYSTATE_STOP),
    m_bOnDemand(false),
    m_bProvidesMusicCodec(false),
    m_bProvidesMusicFiles(false),
    m_bProvidesMusicPlaylists(false),
    m_bProvidesMusicTop100(false),
    m_bProvidesMusicSearch(false),
    m_bSupportsConcurrentStreams(false),
    m_bProvidesVideoCodec(false),
    m_bProvidesMovies(false),
    m_bProvidesTvShows(false),
    m_bProvidesMusicVideos(false),
    m_bProvidesRecentlyAddedMovies(false),
    m_bProvidesRecentlyAddedTvShows(false),
    m_bProvidesRecentlyAddedMusicVideos(false),
    m_bProvidesFiles(false),
    m_bFileOpen(false)
{
  m_strUserPath          = CSpecialProtocol::TranslatePath(Profile()).c_str();
  m_strClientPath        = CSpecialProtocol::TranslatePath(Path()).c_str();
  m_pInfo                = new CONTENT_PROPERTIES;
  m_pInfo->strUserPath   = m_strUserPath.c_str();
  m_pInfo->strClientPath = m_strClientPath.c_str();
}

CContentAddon::CContentAddon(const cp_extension_t *ext) :
    CAddonDll<DllContentAddon, ContentAddon, CONTENT_PROPERTIES>(ext),
    m_apiVersion("0.0.0"),
    m_bReadyToUse(false),
    m_playState(CONTENT_ADDON_PLAYSTATE_STOP),
    m_bOnDemand(false),
    m_bFileOpen(false)
{
  std::string strProvidesMusicCodec         = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_music_codec");
  std::string strProvidesMusicFiles         = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_music_files");
  std::string strProvidesFiles              = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_files");
  std::string strProvidesMusicPlaylists     = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_music_playlists");
  std::string strProvidesMusicTop100        = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_music_top100");
  std::string strProvidesMusicSearch        = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_music_search");
  std::string strSupportsConcurrentStreams  = CAddonMgr::Get().GetExtValue(ext->configuration, "@supports_concurrent_streams");

  std::string strProvidesVideoCodec         = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_video_codec");
  std::string strProvidesMovies             = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_movies");
  std::string strProvidesTvShows            = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_tv_shows");
  std::string strProvidesMusicVideos        = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_music_videos");
  std::string strProvidesRecentlyAddedMovies      = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_recently_added_movies");
  std::string strProvidesRecentlyAddedTvShows     = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_recently_added_tv_shows");
  std::string strProvidesRecentlyAddedMusicVideos = CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_recently_added_music_videos");

  m_fileTypes                   = StringUtils::Split(CAddonMgr::Get().GetExtValue(ext->configuration, "@filetypes"), "|");
  m_bProvidesMusicCodec         = !strProvidesMusicCodec.compare("true") || !strProvidesMusicCodec.compare("1");
  m_bProvidesMusicFiles         = !strProvidesMusicFiles.compare("true") || !strProvidesMusicFiles.compare("1");
  m_bProvidesFiles              = !strProvidesFiles.compare("true") || !strProvidesFiles.compare("1");
  m_bProvidesMusicPlaylists     = !strProvidesMusicPlaylists.compare("true") || !strProvidesMusicPlaylists.compare("1");
  m_bProvidesMusicTop100        = !strProvidesMusicTop100.compare("true") || !strProvidesMusicTop100.compare("1");
  m_bProvidesMusicSearch        = !strProvidesMusicSearch.compare("true") || !strProvidesMusicSearch.compare("1");
  m_bSupportsConcurrentStreams  = !strSupportsConcurrentStreams.compare("true") || !strSupportsConcurrentStreams.compare("1");

  m_bProvidesVideoCodec         = !strProvidesVideoCodec.compare("true") || !strProvidesVideoCodec.compare("1");
  m_bProvidesMovies             = !strProvidesMovies.compare("true") || !strProvidesMovies.compare("1");
  m_bProvidesTvShows            = !strProvidesTvShows.compare("true") || !strProvidesTvShows.compare("1");
  m_bProvidesMusicVideos        = !strProvidesMusicVideos.compare("true") || !strProvidesMusicVideos.compare("1");
  m_bProvidesRecentlyAddedMovies      = !strProvidesRecentlyAddedMovies.compare("true") || !strProvidesRecentlyAddedMovies.compare("1");
  m_bProvidesRecentlyAddedTvShows     = !strProvidesRecentlyAddedTvShows.compare("true") || !strProvidesRecentlyAddedTvShows.compare("1");
  m_bProvidesRecentlyAddedMusicVideos = !strProvidesRecentlyAddedMusicVideos.compare("true") || !strProvidesRecentlyAddedMusicVideos.compare("1");

  m_strUserPath                 = CSpecialProtocol::TranslatePath(Profile()).c_str();
  m_strClientPath               = CSpecialProtocol::TranslatePath(Path()).c_str();
  m_pInfo                       = new CONTENT_PROPERTIES;
  m_pInfo->strUserPath          = m_strUserPath.c_str();
  m_pInfo->strClientPath        = m_strClientPath.c_str();

  /* TODO
  //TODO hack
  if (!m_bProvidesMusicFiles && !m_bProvidesMovies && !m_bProvidesTvShows && !m_bProvidesMusicVideos)
    MarkAsDisabled();
  */
}

CContentAddon::~CContentAddon(void)
{
  Destroy();
  SAFE_DELETE(m_pInfo);
}

void CContentAddon::ResetProperties(void)
{
  CSingleLock lock(m_critSection);
  m_apiVersion  = AddonVersion("0.0.0");
  m_bReadyToUse = false;
}

ADDON_STATUS CContentAddon::Create(void)
{
  ADDON_STATUS status(ADDON_STATUS_UNKNOWN);

  /* ensure that a previous instance is destroyed */
  Destroy();

  bool bReadyToUse(false);
  CLog::Log(LOGDEBUG, "%s - creating content add-on instance '%s'", __FUNCTION__, Name().c_str());
  try
  {
    status = CAddonDll<DllContentAddon, ContentAddon, CONTENT_PROPERTIES>::Create();
    bReadyToUse = (status == ADDON_STATUS_OK);
  }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  {
    CSingleLock lock(m_critSection);
    m_bReadyToUse = bReadyToUse;
  }

  if (bReadyToUse)
    CAnnouncementManager::Get().AddAnnouncer(this);

  return status;
}

bool CContentAddon::DllLoaded(void) const
{
  try { return CAddonDll<DllContentAddon, ContentAddon, CONTENT_PROPERTIES>::DllLoaded(); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  return false;
}

void CContentAddon::Destroy(void)
{
  CAnnouncementManager::Get().RemoveAnnouncer(this);

  MusicCloseFile();

  /* reset 'ready to use' to false */
  {
    CSingleLock lock(m_critSection);
    if (!m_bReadyToUse)
      return;
    m_bReadyToUse = false;
  }

  CLog::Log(LOGDEBUG, "%s - destroying add-on '%s'", __FUNCTION__, Name().c_str());

  /* destroy the add-on */
  try { CAddonDll<DllContentAddon, ContentAddon, CONTENT_PROPERTIES>::Destroy(); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  /* reset all properties to defaults */
  ResetProperties();
}

void CContentAddon::ReCreate(void)
{
  Destroy();
  Create();
}

bool CContentAddon::ReadyToUse(void) const
{
  CSingleLock lock(m_critSection);
  return m_bReadyToUse;
}

const char* CContentAddon::GetServerName(void)
{
  try { return m_pStruct->GetServerName(); }
  catch (exception &e) { LogException(e, __FUNCTION__); }
  return NULL;
}

void CContentAddon::FreeFileList(CONTENT_ADDON_FILELIST* items)
{
  try { return m_pStruct->FreeFileList(items); }
  catch (exception &e) { LogException(e, __FUNCTION__); }
}

map<string, CONTENT_ADDON_FILE_PROPERTY> FileListToMap(CONTENT_ADDON_FILE_PROPERTY* properties, int len)
{
  map<string, CONTENT_ADDON_FILE_PROPERTY> retval;
  for (int i = 0; i < len; i++)
    retval.insert(make_pair(string(properties[i].key), properties[i]));
  return retval;
}

void CContentAddon::AddCommonProperties(AddonFileItem* file, CFileItemPtr& fileItem)
{
  std::string strThumb = ContentBuildPath(file->Thumb(true));
  if (!strThumb.empty())
    fileItem->SetArt("thumb", strThumb);
  std::string strArt   = ContentBuildPath(file->Fanart(true));
  if (!strArt.empty())
  {
    fileItem->SetArt("fanart", strArt);
    fileItem->SetProperty("fanart_image", strArt);
  }

  for (std::map<std::string, AddonFileItemProperty>::const_iterator it = file->m_properties.begin(); it != file->m_properties.end(); it++)
  {
    if (it->second.Type() == CONTENT_ADDON_PROPERTY_TYPE_STRING)
      fileItem->SetProperty(it->first, CVariant(it->second.ValueAsString()));
    else if (it->second.Type() == CONTENT_ADDON_PROPERTY_TYPE_INT)
      fileItem->SetProperty(it->first, CVariant(it->second.ValueAsInt()));
  }

  // Make sure the provider logo is set to a valid full path
  std::string strProvider(fileItem->GetProperty("provider_icon").asString());
  if (strProvider.empty())
    fileItem->SetProperty("provider_icon", Icon());
  else if (!CURL::IsFullPath(strProvider))
    fileItem->SetProperty("provider_icon", URIUtils::AddFileToFolder(Path(), strProvider));
}

void CContentAddon::ReadFilePlaylist(AddonFileItem* file, CFileItemList& fileList)
{
  CMediaSource playlist;
  AddonFilePlaylist* playlistFile = reinterpret_cast<AddonFilePlaylist*>(file);
  if (!playlistFile)
    return;

  const std::string strAddonFilePath = playlistFile->Path(true);
  playlist.strPath = MusicBuildPath(CONTENT_ADDON_TYPE_PLAYLIST, strAddonFilePath);
  playlist.strName = playlistFile->Name(true);
  if (playlist.strPath.empty() || playlist.strName.empty()) return;

  CFileItemPtr pItem(new CFileItem(playlist));
  AddCommonProperties(file, pItem);

  {
    CSingleLock lock(m_critSection);
    m_playlistNames.insert(make_pair(strAddonFilePath, playlist.strName));
  }
  //fileList.Add(pItem, CFileItemList::ADD_COMBINE); // TODO
}

void CContentAddon::ReadFileSong(AddonFileItem* file, CFileItemList& fileList, const string& strArtist /* = "" */, const string& strAlbum /* = "" */)
{
  CSong song;
  AddonFileSong* songFile = reinterpret_cast<AddonFileSong*>(file);
  if (!songFile)
    return;

  const std::string path = MusicBuildPath(CONTENT_ADDON_TYPE_SONG, songFile->Path(true), strArtist, strAlbum);
  //song.LoadAddonFileSong(*songFile, path, true); // TODO
  if (song.strFileName.empty() || song.strTitle.empty())
    return;

  CFileItemPtr pItem(new CFileItem(song));
  AddCommonProperties(file, pItem);

  //fileList.Add(pItem, CFileItemList::ADD_IGNORE); // TODO
}

void CContentAddon::ReadFileAlbum(AddonFileItem* file, CFileItemList& fileList, const string& strArtist /* = "" */)
{
  CAlbum album;
  AddonFileAlbum* albumFile = reinterpret_cast<AddonFileAlbum*>(file);
  if (!albumFile)
    return;

  //album.LoadAddonFileAlbum(*albumFile, true); // TODO

  const std::string strAlbumArtist(strArtist.empty() && !album.artist.empty() ? album.artist.at(0) : strArtist);
  const std::string strAddonFilePath = albumFile->Path(true);
  const std::string strPath = MusicBuildPath(CONTENT_ADDON_TYPE_ALBUM, strAddonFilePath, strAlbumArtist);
  CFileItemPtr pItem(new CFileItem(strPath, album));

  AddCommonProperties(file, pItem);

  {
    CSingleLock lock(m_critSection);
    map<std::string, map<std::string, std::string> >::iterator it = m_albumNames.find(strAlbumArtist);
    if (it != m_albumNames.end())
      it->second.insert(make_pair(strAddonFilePath, album.strAlbum));
    else
    {
      map<std::string, std::string> m;
      m.insert(make_pair(strAddonFilePath, album.strAlbum));
      m_albumNames.insert(make_pair(strAlbumArtist, m));
    }
  }
  //fileList.Add(pItem, CFileItemList::ADD_IGNORE); // TODO
}

void CContentAddon::ReadFileArtist(AddonFileItem* file, CFileItemList& fileList)
{
  CArtist artist;
  AddonFileArtist* artistFile = reinterpret_cast<AddonFileArtist*>(file);
  if (!artistFile)
    return;

  const std::string strAddonFilePath = artistFile->Path(true);
  const std::string strPath = MusicBuildPath(CONTENT_ADDON_TYPE_ARTIST, strAddonFilePath);
  //artist.LoadAddonFileArtist(*artistFile, true); // TODO
  if (strPath.empty() || artist.strArtist.empty())
    return;

  CFileItemPtr pItem(new CFileItem(artist));
  pItem->SetPath(strPath);
  pItem->SetIconImage("DefaultArtist.png");
  pItem->SetProperty("artist_description", artist.strBiography);
  AddCommonProperties(file, pItem);

  {
    CSingleLock lock(m_critSection);
    m_artistNames.insert(make_pair(strAddonFilePath, artist.strArtist));
  }
  //fileList.Add(pItem, CFileItemList::ADD_COMBINE); // TODO
}

void CContentAddon::ReadFileVideo(AddonFileItem* file, CFileItemList& fileList, CONTENT_VIDEO_TYPE type)
{
  CVideoInfoTag video;
  AddonFileVideo* videoFile = reinterpret_cast<AddonFileVideo*>(file);
  if (!videoFile)
    return;

  const std::string strPath = VideoBuildPath(CONTENT_ADDON_TYPE_VIDEO, videoFile->Path(true), type);
  //video.LoadAddonFileVideo(*videoFile, strPath, true); // tODO
  if (strPath.empty() || video.m_strTitle.empty())
    return;

  CFileItemPtr pItem(new CFileItem(video));
  AddCommonProperties(file, pItem);

  //fileList.Add(pItem, CFileItemList::ADD_IGNORE); // CFileItemList::ADD_COMBINE // TODO
}

void CContentAddon::ReadFileGenre(AddonFileItem* file, CFileItemList& fileList, CONTENT_VIDEO_TYPE type)
{
  CMediaSource m;
  AddonFileGenre* genreFile = reinterpret_cast<AddonFileGenre*>(file);
  if (!genreFile)
    return;
  
  m.strPath = VideoBuildPath(CONTENT_ADDON_TYPE_GENRE, genreFile->Path(true), type);
  m.strName = genreFile->Name(true);
  
  if (m.strPath.empty() || m.strName.empty())
    return;

  CFileItemPtr pItem(new CFileItem(m));
  AddCommonProperties(file, pItem);

  //fileList.Add(pItem, CFileItemList::ADD_COMBINE); // TODO
}

void CContentAddon::ReadFileYear(AddonFileItem* file, CFileItemList& fileList, CONTENT_VIDEO_TYPE type)
{
  CMediaSource m;
  AddonFileYear* yearFile = reinterpret_cast<AddonFileYear*>(file);
  if (!yearFile)
    return;

  m.strPath = VideoBuildPath(CONTENT_ADDON_TYPE_YEAR, yearFile->Path(true), type);
  m.strName = yearFile->Name(true);

  if (m.strPath.empty() || m.strName.empty())
    return;

  CFileItemPtr pItem(new CFileItem(m));
  AddCommonProperties(file, pItem);

  //fileList.Add(pItem, CFileItemList::ADD_COMBINE); // TODO
}

void CContentAddon::ReadFileDirectory(AddonFileItem* file, CFileItemList& fileList)
{
  CMediaSource m;
  AddonFileDirectory* directoryFile = reinterpret_cast<AddonFileDirectory*>(file);
  if (!directoryFile)
    return;

  m.strPath = ContentBuildPath(directoryFile->Path(true));
  m.strName = directoryFile->Name(true);

  if (m.strPath.empty() || m.strName.empty())
    return;

  CFileItemPtr pItem(new CFileItem(m));
  AddCommonProperties(file, pItem);

  //fileList.Add(pItem, CFileItemList::ADD_COMBINE); // TODO
}

void CContentAddon::ReadFileFile(AddonFileItem* file, CFileItemList& fileList)
{
  CMediaSource m;
  AddonFileFile* fileFile = reinterpret_cast<AddonFileFile*>(file);
  if (!fileFile)
    return;

  m.strPath = ContentBuildPath(fileFile->Path(true));
  m.strName = fileFile->Name(true);

  if (m.strPath.empty() || m.strName.empty())
    return;

  CFileItemPtr pItem(new CFileItem(m));
  AddCommonProperties(file, pItem);

  //fileList.Add(pItem, CFileItemList::ADD_IGNORE); // TODO
}

void CContentAddon::ReadMusicFiles(const CONTENT_ADDON_FILELIST& addonItems, CFileItemList& xbmcItems, const string& strArtist /* = "" */, const string& strAlbum /* = "" */)
{
  AddonFileItemList list(addonItems);
  for (vector<AddonFileItem>::iterator it = list.m_fileItems.begin(); it != list.m_fileItems.end(); it++)
  {
    AddonFileItem& fileItem = *it;
    switch (fileItem.Type())
    {
    case CONTENT_ADDON_TYPE_SONG:
      ReadFileSong(&fileItem, xbmcItems, strArtist, strAlbum);
      break;
    case CONTENT_ADDON_TYPE_ARTIST:
      ReadFileArtist(&fileItem, xbmcItems);
      break;
    case CONTENT_ADDON_TYPE_ALBUM:
      ReadFileAlbum(&fileItem, xbmcItems, strArtist);
      break;
    case CONTENT_ADDON_TYPE_PLAYLIST:
      ReadFilePlaylist(&fileItem, xbmcItems);
      break;
    case CONTENT_ADDON_TYPE_DIRECTORY:
      ReadFileDirectory(&fileItem, xbmcItems);
      break;
    case CONTENT_ADDON_TYPE_FILE:
      ReadFileFile(&fileItem, xbmcItems);
      break;
    default:
      CLog::Log(LOGWARNING, "invalid filetype: %d", fileItem.Type());
      break;
    }
  }
  xbmcItems.SetCacheToDisc(CFileItemList::CACHE_NEVER);
}

void CContentAddon::ReadVideoFiles(const CONTENT_ADDON_FILELIST& addonItems, CFileItemList& xbmcItems, CONTENT_VIDEO_TYPE type)
{
  AddonFileItemList list(addonItems);
  for (vector<AddonFileItem>::iterator it = list.m_fileItems.begin(); it != list.m_fileItems.end(); it++)
  {
    AddonFileItem& fileItem = *it;
    switch (fileItem.Type())
    {
    case CONTENT_ADDON_TYPE_VIDEO:
      ReadFileVideo(&fileItem, xbmcItems, type);
      break;
    case CONTENT_ADDON_TYPE_GENRE:
      ReadFileGenre(&fileItem, xbmcItems, type);
      break;
    case CONTENT_ADDON_TYPE_YEAR:
      ReadFileYear(&fileItem, xbmcItems, type);
      break;
    case CONTENT_ADDON_TYPE_DIRECTORY:
      ReadFileDirectory(&fileItem, xbmcItems);
      break;
    case CONTENT_ADDON_TYPE_FILE:
      ReadFileFile(&fileItem, xbmcItems);
      break;
    default:
      CLog::Log(LOGWARNING, "invalid filetype: %d", fileItem.Type());
      break;
    }
  }
  xbmcItems.SetCacheToDisc(CFileItemList::CACHE_NEVER);
}

bool CContentAddon::MusicGetPlaylists(CFileItemList& items)
{
  if (!ReadyToUse() || !ProvidesMusicFiles() || !ProvidesMusicPlaylists())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "getting playlists from add-on '%s'", Name().c_str());
  try { err = m_pStruct->MusicGetPlaylists(&retVal); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadMusicFiles(*retVal, items);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetArtists(CFileItemList& items)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "getting artists from add-on '%s'", Name().c_str());
  try { err = m_pStruct->MusicGetArtists(&retVal); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadMusicFiles(*retVal, items);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetPlaylistContent(CFileItemList& items, const std::string& strName)
{
  if (!ReadyToUse() || !ProvidesMusicFiles() || !ProvidesMusicPlaylists())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "getting playlist '%s' from add-on '%s'", MusicGetPlaylistName(strName).c_str(), Name().c_str());
  try { err = m_pStruct->MusicGetPlaylist(&retVal, strName.c_str()); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadMusicFiles(*retVal, items);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetAlbums(CFileItemList& items, const std::string& strArtist)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  if (strArtist.empty())
    CLog::Log(LOGDEBUG, "getting all albums from add-on '%s'", Name().c_str());
  else
    CLog::Log(LOGDEBUG, "getting albums for artist '%s' from add-on '%s'", MusicGetArtistName(strArtist).c_str(), Name().c_str());
  try { err = m_pStruct->MusicGetAlbums(&retVal, strArtist.c_str()); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadMusicFiles(*retVal, items, strArtist);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetSongs(CFileItemList& items, const std::string& strArtist, const std::string& strAlbum)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  if (strArtist.empty())
    CLog::Log(LOGDEBUG, "getting all songs from add-on '%s'", Name().c_str());
  else if (strAlbum.empty())
    CLog::Log(LOGDEBUG, "getting songs from artist '%s' from add-on '%s'", MusicGetArtistName(strArtist).c_str(), Name().c_str());
  else
    CLog::Log(LOGDEBUG, "getting songs for album '%s' from artist '%s' from add-on '%s'", MusicGetAlbumName(strArtist, strAlbum).c_str(), MusicGetArtistName(strArtist).c_str(), Name().c_str());

  try { err = m_pStruct->MusicGetSongs(&retVal, strArtist.c_str(), strAlbum.c_str()); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadMusicFiles(*retVal, items, strArtist, strAlbum);

    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetCodecInfo(const string& strPath, CONTENT_ADDON_CODEC_INFO* info)
{
  if (!ReadyToUse() || !ProvidesMusicCodec())
    return false;

  std::string strFilePath = GetFilename(strPath);

  try { return m_pStruct->MusicGetCodecInfo(strFilePath.c_str(), info) == CONTENT_ERROR_NO_ERROR; }
  catch (exception &e) { LogException(e, __FUNCTION__); }
  return false;
}

std::string CContentAddon::GetFilename(const std::string& strPath) const
{
  std::string retval(strPath);

  // check whether the filename starts with content://id/
  std::string strContentNode = StringUtils::Format("%s%s/", CONTENT_NODE, ID().c_str());
  if (StringUtils::StartsWith(strPath, strContentNode))
    return StringUtils::Right(strPath, strPath.length() - strContentNode.length());

  // check whether the file resolves to a directory node that we created
  strContentNode = StringUtils::Format("%s%s/", MUSIC_VIRTUAL_NODE, ID().c_str());
  if (StringUtils::StartsWith(strPath, strContentNode))
  {
    /* TODO
    MUSICDATABASEDIRECTORY::CDirectoryNode* pNode = MUSICDATABASEDIRECTORY::CDirectoryNode::ParseURL(strPath);
    MUSICDATABASEDIRECTORY::CContentAddonDirectoryNode* pAddonNode = dynamic_cast<MUSICDATABASEDIRECTORY::CContentAddonDirectoryNode*>(pNode);
    if (pAddonNode)
    {
      CONTENT_ADDON addon = pAddonNode->GetAddon();
      if (addon && !addon->ID().compare(ID()))
        retval = pAddonNode->Filename();
    }
    */
  }

  strContentNode = StringUtils::Format("%s%s/", VIDEO_VIRTUAL_NODE, ID().c_str());
  if (StringUtils::StartsWith(strPath, strContentNode))
  {
    /* TODO
    VIDEODATABASEDIRECTORY::CDirectoryNode* pNode = VIDEODATABASEDIRECTORY::CDirectoryNode::ParseURL(strPath);
    VIDEODATABASEDIRECTORY::CContentAddonDirectoryNode* pAddonNode = dynamic_cast<VIDEODATABASEDIRECTORY::CContentAddonDirectoryNode*>(pNode); // TODO
    if (pAddonNode)
    {
      CONTENT_ADDON addon = pAddonNode->GetAddon();
      if (addon && !addon->ID().compare(ID()))
        retval = pAddonNode->Filename();
    }
    */
  }

  return retval;
}

bool CContentAddon::MusicOpenFile(const string& strPath)
{
  if (!CreateOnDemand() || !ProvidesMusicCodec() || !SupportsFile(strPath))
    return false;

  std::string strFilePath = GetFilename(strPath);

  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);
  try { err = m_pStruct->MusicOpenFile(strFilePath.c_str()); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err != CONTENT_ERROR_NO_ERROR)
    CLog::Log(LOGERROR, "add-on '%s' returned an error from OpenMusicFile(%s): %d", Name().c_str(), strPath.c_str(), err);
  else
    m_bFileOpen = true;

  return err == CONTENT_ERROR_NO_ERROR;
}

bool CContentAddon::MusicPreloadFile(const string& strPath)
{
  if (!CreateOnDemand() || !ProvidesMusicCodec())
    return false;

  std::string strFilePath = GetFilename(strPath);

  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);
  try { err = m_pStruct->MusicPreloadFile(strFilePath.c_str()); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err != CONTENT_ERROR_NO_ERROR)
    CLog::Log(LOGERROR, "add-on '%s' returned an error from MusicPreloadFile(%s): %d", Name().c_str(), strPath.c_str(), err);

  return err == CONTENT_ERROR_NO_ERROR;
}

void CContentAddon::MusicCloseFile(void)
{
  if (!ReadyToUse() || !ProvidesMusicCodec())
    return;

  if (m_bFileOpen)
  {
    m_bFileOpen = false;
    try { m_pStruct->MusicCloseFile(); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }
}

void CContentAddon::MusicPause(bool bPause)
{
  if (!ReadyToUse() || !ProvidesMusicCodec())
    return;

  {
    CSingleLock lock(m_critSection);
    if ((m_playState == CONTENT_ADDON_PLAYSTATE_PAUSE && bPause) ||
        (m_playState == CONTENT_ADDON_PLAYSTATE_PLAY && !bPause) ||
        m_playState == CONTENT_ADDON_PLAYSTATE_STOP)
      return;
    m_playState = bPause ? CONTENT_ADDON_PLAYSTATE_PAUSE : CONTENT_ADDON_PLAYSTATE_PLAY;
  }

  try { m_pStruct->MusicPause(bPause ? 1 : 0); }
  catch (exception &e) { LogException(e, __FUNCTION__); }
}

int64_t CContentAddon::MusicSeek(int64_t iSeekTime)
{
  if (!ReadyToUse() || !ProvidesMusicCodec())
    return -1;

  try { return m_pStruct->MusicSeek(iSeekTime); }
  catch (exception &e) { LogException(e, __FUNCTION__); }
  return 0;
}

int CContentAddon::MusicReadPCM(BYTE* pBuffer, int size, int* actualsize)
{
  if (!ReadyToUse() || !ProvidesMusicCodec())
    return -1;

  try { return m_pStruct->MusicReadPCM(pBuffer, size, actualsize); }
  catch (exception &e) { LogException(e, __FUNCTION__); }
  return -1;
}

void CContentAddon::LogException(const exception &e, const char *strFunctionName) const
{
  CLog::Log(LOGERROR, "exception '%s' caught while trying to call '%s' on add-on '%s'. Please contact the developer of this add-on: %s", e.what(), strFunctionName, Name().c_str(), Author().c_str());
}

std::string CContentAddon::MusicGetPlaylistName(const std::string& strPlaylist) const
{
  std::string strReturn(strPlaylist);
  if (!ReadyToUse() || !ProvidesMusicFiles() || !ProvidesMusicPlaylists())
    return strReturn;
  CSingleLock lock(m_critSection);
  map<std::string, std::string>::const_iterator it = m_playlistNames.find(strPlaylist);
  if (it != m_playlistNames.end())
    strReturn = it->second;
  return strReturn;
}

std::string CContentAddon::MusicGetArtistName(const std::string& strArtist) const
{
  std::string strReturn(strArtist);
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return strReturn;
  CSingleLock lock(m_critSection);
  map<std::string, std::string>::const_iterator it = m_artistNames.find(strArtist);
  if (it != m_artistNames.end())
    strReturn = it->second;
  return strReturn;
}

std::string CContentAddon::MusicGetAlbumName(const std::string& strArtist, const std::string& strAlbum) const
{
  std::string strReturn(strAlbum);
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return strReturn;
  CSingleLock lock(m_critSection);

  if (!strArtist.empty())
  {
    map<std::string, map<std::string, std::string> >::const_iterator it = m_albumNames.find(strArtist.c_str());
    if (it != m_albumNames.end())
    {
      map<std::string, std::string>::const_iterator it2 = it->second.find(strAlbum.c_str());
      if (it2 != it->second.end())
        return it2->second;
    }
  }

  // return the first match
  for (map<std::string, map<std::string, std::string> >::const_iterator it = m_albumNames.begin(); it != m_albumNames.end(); it++)
  {
    map<std::string, std::string>::const_iterator it2 = it->second.find(strAlbum.c_str());
    if (it2 != it->second.end())
    {
      strReturn = it2->second;
      break;
    }
  }
  return strReturn;
}

bool CContentAddon::VideoGetOverviewItems(CFileItemList& items, CONTENT_VIDEO_TYPE type)
{
  if (!ReadyToUse() || !Provides(type))
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "getting overview items from add-on '%s'", Name().c_str());
  try { err = m_pStruct->VideoGetOverviewItems(&retVal, type); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadVideoFiles(*retVal, items, type);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::VideoGetGeneralOverview(CFileItemList& items)
{
  if (!ReadyToUse())
    return false;
  
  std::string strDir;
  std::string strPrepend = StringUtils::Format("%s%s/", VIDEO_VIRTUAL_NODE, ID().c_str());

  if (Provides(CONTENT_VIDEO_TYPE_MOVIES))
  {
    CFileItemPtr pMoviesItem(new CFileItem(g_localizeStrings.Get(342))); // Movies
    strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), VIDEO_MOVIES);
    pMoviesItem->SetPath(strDir);
    pMoviesItem->m_bIsFolder = true;
    pMoviesItem->SetCanQueue(false);
    items.Add(pMoviesItem);
  }
  if (Provides(CONTENT_VIDEO_TYPE_TVSHOWS))
  {
    CFileItemPtr pTvShowsItem(new CFileItem(g_localizeStrings.Get(20343))); // TV shows
    strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), VIDEO_TVSHOWS);
    pTvShowsItem->SetPath(strDir);
    pTvShowsItem->m_bIsFolder = true;
    pTvShowsItem->SetCanQueue(false);
    items.Add(pTvShowsItem);
  }
  if (Provides(CONTENT_VIDEO_TYPE_MUSICVIDEOS))
  {
    CFileItemPtr pMusicVideosItem(new CFileItem(g_localizeStrings.Get(20389))); // Music videos
    strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), VIDEO_MUSICVIDEOS);
    pMusicVideosItem->SetPath(strDir);
    pMusicVideosItem->m_bIsFolder = true;
    pMusicVideosItem->SetCanQueue(false);
    items.Add(pMusicVideosItem);
  }
  if (ProvidesRecentlyAdded(CONTENT_VIDEO_TYPE_MOVIES))
  {
    CFileItemPtr pRecentlyAddedMoviesItem(new CFileItem(g_localizeStrings.Get(20386))); // Recently added movies
    strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), VIDEO_RECENT_MOVIES);
    pRecentlyAddedMoviesItem->SetPath(strDir);
    pRecentlyAddedMoviesItem->m_bIsFolder = true;
    pRecentlyAddedMoviesItem->SetCanQueue(false);
    items.Add(pRecentlyAddedMoviesItem);
  }
  if (ProvidesRecentlyAdded(CONTENT_VIDEO_TYPE_TVSHOWS))
  {
    CFileItemPtr pRecentlyAddedTvShowsItem(new CFileItem(g_localizeStrings.Get(20387))); // Recently added episodes
    strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), VIDEO_RECENT_TVSHOWS);
    pRecentlyAddedTvShowsItem->SetPath(strDir);
    pRecentlyAddedTvShowsItem->m_bIsFolder = true;
    pRecentlyAddedTvShowsItem->SetCanQueue(false);
    items.Add(pRecentlyAddedTvShowsItem);
  }
  if (ProvidesRecentlyAdded(CONTENT_VIDEO_TYPE_MUSICVIDEOS))
  {
    CFileItemPtr pRecentlyAddedMusicVideosItem(new CFileItem(g_localizeStrings.Get(20390))); // Recently added music videos
    strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), VIDEO_RECENT_MUSICVIDEOS);
    pRecentlyAddedMusicVideosItem->SetPath(strDir);
    pRecentlyAddedMusicVideosItem->m_bIsFolder = true;
    pRecentlyAddedMusicVideosItem->SetCanQueue(false);
    items.Add(pRecentlyAddedMusicVideosItem);
  }

  return items.Size() > 0;
}

bool CContentAddon::VideoGetOverview(CFileItemList& items, CONTENT_VIDEO_TYPE type)
{
  if (!ReadyToUse() || !Provides(type))
    return false;

  std::string strPrepend = StringUtils::Format("%s%s/", VIDEO_VIRTUAL_NODE, ID().c_str());

  if (type == CONTENT_VIDEO_TYPE_MOVIES)
    strPrepend += StringUtils::Format("%s/", VIDEO_MOVIES);
  else if (type == CONTENT_VIDEO_TYPE_TVSHOWS)
    strPrepend += StringUtils::Format("%s/", VIDEO_TVSHOWS);
  else if (type == CONTENT_VIDEO_TYPE_MUSICVIDEOS)
    strPrepend += StringUtils::Format("%s/", VIDEO_MUSICVIDEOS);

  CFileItemPtr pGenreItem(new CFileItem(g_localizeStrings.Get(135))); // Genres
  std::string strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), VIDEO_GENRE);
  pGenreItem->SetPath(strDir);
  pGenreItem->m_bIsFolder = true;
  pGenreItem->SetCanQueue(false);
  items.Add(pGenreItem);

  CFileItemPtr pTitleItem(new CFileItem(g_localizeStrings.Get(369))); // Title
  strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), VIDEO_TITLE);
  pTitleItem->SetPath(strDir);
  pTitleItem->m_bIsFolder = true;
  pTitleItem->SetCanQueue(false);
  items.Add(pTitleItem);

  CFileItemPtr pYearItem(new CFileItem(g_localizeStrings.Get(562))); // Year
  strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), VIDEO_YEAR);
  pYearItem->SetPath(strDir);
  pYearItem->m_bIsFolder = true;
  pYearItem->SetCanQueue(false);
  items.Add(pYearItem);

  return true;
}

bool CContentAddon::VideoGetGenres(CFileItemList& items, CONTENT_VIDEO_TYPE type)
{
  if (!ReadyToUse() || !Provides(type))
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err = CONTENT_ERROR_UNKNOWN;

  CLog::Log(LOGDEBUG, "getting video genres from add-on '%s'", Name().c_str());
  try { err = m_pStruct->VideoGetGenres(&retVal, type); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadVideoFiles(*retVal, items, type);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::VideoGetYears(CFileItemList& items, CONTENT_VIDEO_TYPE type)
{
  if (!ReadyToUse() || !Provides(type))
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err = CONTENT_ERROR_UNKNOWN;

  CLog::Log(LOGDEBUG, "getting video years from add-on '%s'", Name().c_str());
  try { err = m_pStruct->VideoGetYears(&retVal, type); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadVideoFiles(*retVal, items, type);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::VideoGetRecentlyAdded(CFileItemList& items, CONTENT_VIDEO_TYPE type)
{
  if (!ReadyToUse() || !ProvidesRecentlyAdded(type))
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err = CONTENT_ERROR_UNKNOWN;

  CLog::Log(LOGDEBUG, "getting recently added (%d) from add-on '%s'", type, Name().c_str());
  try { err = m_pStruct->VideoGetRecentlyAdded(&retVal, type); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadVideoFiles(*retVal, items, type);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::VideoGetMovies(CFileItemList& items, const std::string& strGenre /* = "" */, int iYear /* = -1 */)
{
  if (!ReadyToUse() || !Provides(CONTENT_VIDEO_TYPE_MOVIES))
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err = CONTENT_ERROR_UNKNOWN;

  CLog::Log(LOGDEBUG, "getting movies from add-on '%s' (genre: '%s', year: %d)", Name().c_str(), strGenre.c_str(), iYear);
  try { err = m_pStruct->VideoGetMovies(&retVal, strGenre.c_str(), iYear); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadVideoFiles(*retVal, items, CONTENT_VIDEO_TYPE_MOVIES);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::VideoGetTvShows(CFileItemList& items, const std::string& strGenre /* = "" */, int iYear /* = -1 */)
{
  if (!ReadyToUse() || !Provides(CONTENT_VIDEO_TYPE_TVSHOWS))
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err = CONTENT_ERROR_UNKNOWN;

  CLog::Log(LOGDEBUG, "getting tv shows from add-on '%s' (genre: '%s', year: %d)", Name().c_str(), strGenre.c_str(), iYear);
  try { err = m_pStruct->VideoGetTvShows(&retVal, strGenre.c_str(), iYear); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadVideoFiles(*retVal, items, CONTENT_VIDEO_TYPE_TVSHOWS);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::VideoGetSeasons(CFileItemList& items, const std::string& strTitle /* = "" */, const std::string& strGenre /* = "" */, int iYear /* = -1 */)
{
  if (!ReadyToUse() || !Provides(CONTENT_VIDEO_TYPE_TVSHOWS))
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err = CONTENT_ERROR_UNKNOWN;

  CLog::Log(LOGDEBUG, "getting seasons from add-on '%s' (tv show: '%s', genre: '%s', year: %d)", Name().c_str(), strTitle.c_str(), strGenre.c_str(), iYear);
  try { err = m_pStruct->VideoGetSeasons(&retVal, strTitle.c_str(), strGenre.c_str(), iYear); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadVideoFiles(*retVal, items, CONTENT_VIDEO_TYPE_TVSHOWS);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::VideoGetEpisodes(CFileItemList& items, const std::string& strTitle /* = "" */, const std::string& strGenre /* = "" */, int iYear /* = -1 */, int iSeason /* = -1 */)
{
  if (!ReadyToUse() || !Provides(CONTENT_VIDEO_TYPE_TVSHOWS))
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err = CONTENT_ERROR_UNKNOWN;

  CLog::Log(LOGDEBUG, "getting episodes from add-on '%s' (tv show: '%s', genre: '%s', year: %d, season: %d)", Name().c_str(), strTitle.c_str(), strGenre.c_str(), iYear, iSeason);
  try { err = m_pStruct->VideoGetEpisodes(&retVal, strTitle.c_str(), strGenre.c_str(), iYear, iSeason); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadVideoFiles(*retVal, items, CONTENT_VIDEO_TYPE_TVSHOWS);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::VideoGetMusicVideos(CFileItemList& items, const std::string& strGenre /* = "" */, int iYear /* = -1 */)
{
  if (!ReadyToUse() || !Provides(CONTENT_VIDEO_TYPE_MUSICVIDEOS))
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err = CONTENT_ERROR_UNKNOWN;

  CLog::Log(LOGDEBUG, "getting music videos from add-on '%s' (genre: '%s', year: %d)", Name().c_str(), strGenre.c_str(), iYear);
  try { err = m_pStruct->VideoGetMusicVideos(&retVal, strGenre.c_str(), iYear); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadVideoFiles(*retVal, items, CONTENT_VIDEO_TYPE_MUSICVIDEOS);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

void CContentAddon::Announce(AnnouncementFlag flag, const char *sender, const char *message, const CVariant &data)
{
  if (!strcmp(sender, "xbmc") && !strcmp(message, "OnStop"))
    MusicCloseFile();
  else if (!strcmp(sender, "xbmc") && !strcmp(message, "OnPause"))
    MusicPause(true);
  else if (!strcmp(sender, "xbmc") && !strcmp(message, "OnPlay"))
    MusicPause(false);
}

bool CContentAddon::MusicGetTop100(CFileItemList& items, CONTENT_TOP100_TYPE type)
{
  if (!ReadyToUse() || !ProvidesMusicFiles() || !ProvidesMusicTop100())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "getting top100 from add-on '%s'", Name().c_str());
  try { err = m_pStruct->MusicGetTop100(&retVal, type); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadMusicFiles(*retVal, items);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetOverviewItems(CFileItemList& items)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "getting overview items from add-on '%s'", Name().c_str());
  try { err = m_pStruct->MusicGetOverviewItems(&retVal); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadMusicFiles(*retVal, items);
    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetOverview(CFileItemList& items)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  std::string strPrepend = StringUtils::Format("%s%s/", MUSIC_VIRTUAL_NODE, ID().c_str());

  CFileItemPtr pArtistItem(new CFileItem(g_localizeStrings.Get(133)));
  std::string strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), MUSIC_ARTIST);
  pArtistItem->SetPath(strDir);
  pArtistItem->m_bIsFolder = true;
  pArtistItem->SetCanQueue(false);
  items.Add(pArtistItem);

  CFileItemPtr pAlbumItem(new CFileItem(g_localizeStrings.Get(132)));
  strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), MUSIC_ALBUM);
  pAlbumItem->SetPath(strDir);
  pAlbumItem->m_bIsFolder = true;
  pAlbumItem->SetCanQueue(false);
  items.Add(pAlbumItem);

  CFileItemPtr pSongItem(new CFileItem(g_localizeStrings.Get(134)));
  strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), MUSIC_SONG);
  pSongItem->SetPath(strDir);
  pSongItem->m_bIsFolder = true;
  pSongItem->SetCanQueue(false);
  items.Add(pSongItem);

  if (ProvidesMusicTop100())
  {
    CFileItemPtr pTop100Item(new CFileItem(g_localizeStrings.Get(271)));
    strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), MUSIC_TOP100);
    pTop100Item->SetPath(strDir);
    pTop100Item->m_bIsFolder = true;
    pTop100Item->SetCanQueue(false);
    items.Add(pTop100Item);
  }

  if (ProvidesMusicPlaylists())
  {
    CFileItemPtr pPlaylistItem(new CFileItem(g_localizeStrings.Get(136)));
    strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), MUSIC_PLAYLIST);
    pPlaylistItem->SetPath(strDir);
    pPlaylistItem->m_bIsFolder = true;
    pPlaylistItem->SetCanQueue(false);
    items.Add(pPlaylistItem);
  }

  if (ProvidesMusicSearch())
  {
    CFileItemPtr pSearchItem(new CFileItem(g_localizeStrings.Get(137)));
    //strDir = URIUtils::MakeMusicSearchPath(ID()); // TODO
    pSearchItem->SetPath(strDir);
    pSearchItem->m_bIsFolder = true;
    pSearchItem->SetCanQueue(false);
    items.Add(pSearchItem);
  }

  return true;
}

bool CContentAddon::MusicGetTop100Overview(CFileItemList& items)
{
  if (!ReadyToUse() || !ProvidesMusicFiles() || !ProvidesMusicTop100())
    return false;

  std::string strPrepend = StringUtils::Format("%s%s/%s/", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_TOP100);

  // TODO only add items that this add-on supports

  CFileItemPtr pArtistItem(new CFileItem(g_localizeStrings.Get(133)));
  std::string strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), MUSIC_ARTIST);
  pArtistItem->SetPath(strDir);
  pArtistItem->m_bIsFolder = true;
  pArtistItem->SetCanQueue(false);
  items.Add(pArtistItem);

  CFileItemPtr pAlbumItem(new CFileItem(g_localizeStrings.Get(132)));
  strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), MUSIC_ALBUM);
  pAlbumItem->SetPath(strDir);
  pAlbumItem->m_bIsFolder = true;
  pAlbumItem->SetCanQueue(false);
  items.Add(pAlbumItem);

  CFileItemPtr pSongItem(new CFileItem(g_localizeStrings.Get(134)));
  strDir = StringUtils::Format("%s%s/", strPrepend.c_str(), MUSIC_SONG);
  pSongItem->SetPath(strDir);
  pSongItem->m_bIsFolder = true;
  pSongItem->SetCanQueue(false);
  items.Add(pSongItem);

  return true;
}

bool CContentAddon::MusicSearch(CFileItemList& items, const std::string& strQuery, CONTENT_ADDON_SEARCH_TYPE type /* = CONTENT_SEARCH_ALL */)
{
  if (!ReadyToUse() || !ProvidesMusicFiles() || !ProvidesMusicSearch())
    return false;

  CONTENT_ADDON_FILELIST* retVal = NULL;
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  CLog::Log(LOGDEBUG, "searching for '%s' in add-on '%s'", strQuery.c_str(), Name().c_str());
  try { err = m_pStruct->MusicSearch(&retVal, strQuery.c_str(), type); }
  catch (exception &e) { LogException(e, __FUNCTION__); return false; }

  if (err == CONTENT_ERROR_NO_ERROR && retVal)
  {
    ReadMusicFiles(*retVal, items);

    FreeFileList(retVal);
    return true;
  }

  return false;
}

bool CContentAddon::MusicGetContextButtons(const CFileItemPtr& item, CContextButtons &buttons)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  // TODO
  return false;
}

bool CContentAddon::MusicClickContextButton(const CFileItemPtr& item, CONTEXT_BUTTON button)
{
  if (!ReadyToUse() || !ProvidesMusicFiles())
    return false;

  // TODO
  return false;
}

void CContentAddon::SetPlaystate(CONTENT_ADDON_PLAYSTATE newState)
{
  CLog::Log(LOGDEBUG, "%s(%d)", __FUNCTION__, (int)newState);
  CSingleLock lock(m_critSection);
  switch (newState)
  {
  case CONTENT_ADDON_PLAYSTATE_PAUSE:
    if (m_playState == CONTENT_ADDON_PLAYSTATE_PLAY)
    {
      m_playState = CONTENT_ADDON_PLAYSTATE_PAUSE;
      CApplicationMessenger::Get().MediaPause();
    }
    break;
  case CONTENT_ADDON_PLAYSTATE_PLAY:
    if (m_playState == CONTENT_ADDON_PLAYSTATE_PAUSE)
    {
      m_playState = CONTENT_ADDON_PLAYSTATE_PLAY;
      CApplicationMessenger::Get().MediaPause();
    }
    break;
  case CONTENT_ADDON_PLAYSTATE_STOP:
    if (m_playState != CONTENT_ADDON_PLAYSTATE_STOP)
    {
      m_playState = CONTENT_ADDON_PLAYSTATE_STOP;
      CApplicationMessenger::Get().MediaStop();
    }
    break;
  default:
    CLog::Log(LOGWARNING, "add-on '%s' tried to set an invalid playstate (%d)", Name().c_str(), (int)newState);
    break;
  }
}

bool CContentAddon::SupportsFile(const std::string& strPath) const
{
  // check whether the file resolves to a directory node that we created
  std::string strContentNode = StringUtils::Format("%s%s/", CONTENT_NODE, ID().c_str());
  if (StringUtils::StartsWith(strPath, strContentNode))
    return true;
  
  strContentNode = StringUtils::Format("%s%s/", MUSIC_VIRTUAL_NODE, ID().c_str());
  if (StringUtils::StartsWith(strPath, strContentNode))
  {
    auto_ptr<MUSICDATABASEDIRECTORY::CDirectoryNode> pNode(MUSICDATABASEDIRECTORY::CDirectoryNode::ParseURL(strPath));
    if (pNode.get())
    {
      /* TODO
      MUSICDATABASEDIRECTORY::CContentAddonDirectoryNode* pAddonNode = dynamic_cast<MUSICDATABASEDIRECTORY::CContentAddonDirectoryNode*>(pNode.get());
      if (pAddonNode)
      {
        CONTENT_ADDON addon = pAddonNode->GetAddon();
        return (addon.get() && !addon->ID().compare(ID()));
      }
      */
    }
  }

  strContentNode = StringUtils::Format("%s%s/", VIDEO_VIRTUAL_NODE, ID().c_str());
  if (StringUtils::StartsWith(strPath, strContentNode))
  {
    auto_ptr<VIDEODATABASEDIRECTORY::CDirectoryNode> pNode(VIDEODATABASEDIRECTORY::CDirectoryNode::ParseURL(strPath));
    if (pNode.get())
    {
      /* TODO
      VIDEODATABASEDIRECTORY::CContentAddonDirectoryNode* pAddonNode = dynamic_cast<VIDEODATABASEDIRECTORY::CContentAddonDirectoryNode*>(pNode.get());
      if (pAddonNode)
      {
        CONTENT_ADDON addon = pAddonNode->GetAddon();
        return (addon.get() && !addon->ID().compare(ID()));
      }
      */
    }
  }

  // check whether the filetype matches any of the filetypes in addon.xml
  CURL url(strPath);
  if (SupportsFileType(url.GetFileType()))
    return true;

  // if this add-on provides a music codec, ask the add-on whether it supports this filename
  if (ProvidesMusicCodec())
  {
    try { return m_pStruct->SupportsFile(strPath.c_str()); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  // no match, the add-on doesn't support this file
  return false;
}

bool CContentAddon::SupportsFileType(const string& strType) const
{
  return find(m_fileTypes.begin(), m_fileTypes.end(), strType) != m_fileTypes.end();
}

bool CContentAddon::Provides(CONTENT_VIDEO_TYPE type) const
{
  if (type == CONTENT_VIDEO_TYPE_MOVIES)
    return m_bReadyToUse & m_bProvidesMovies;
  if (type == CONTENT_VIDEO_TYPE_TVSHOWS)
    return m_bReadyToUse & m_bProvidesTvShows;
  if (type == CONTENT_VIDEO_TYPE_MUSICVIDEOS)
    return m_bReadyToUse & m_bProvidesMusicVideos;
  return false;
}

bool CContentAddon::ProvidesRecentlyAdded(CONTENT_VIDEO_TYPE type) const
{
  if (type == CONTENT_VIDEO_TYPE_MOVIES)
    return m_bReadyToUse & m_bProvidesRecentlyAddedMovies;
  if (type == CONTENT_VIDEO_TYPE_TVSHOWS)
    return m_bReadyToUse & m_bProvidesRecentlyAddedTvShows;
  if (type == CONTENT_VIDEO_TYPE_MUSICVIDEOS)
    return m_bReadyToUse & m_bProvidesRecentlyAddedMusicVideos;
  return false;
}

bool CContentAddon::CreateOnDemand(void)
{
  if (ReadyToUse())
    return true;
  if (OnDemand())
  {
    CLog::Log(LOGDEBUG, "Creating on-demand add-on '%s'", Name().c_str());
    return Create() == ADDON_STATUS_OK;
  }
  return false;
}

std::string CContentAddon::MusicBuildPath(CONTENT_ADDON_TYPE type, const std::string& strFilename, const std::string& strArtist /* = "" */, const std::string& strAlbum /* = "" */) const
{
  if (strFilename.empty())
    return strFilename;

  CURL fileName(strFilename);
  if (!URIUtils::IsInternetStream(fileName))
  {
    std::string strReturn;
    switch (type)
    {
    case CONTENT_ADDON_TYPE_SONG:
      {
        if (!strArtist.empty())
        {
          if (!strAlbum.empty())
            strReturn = StringUtils::Format("%s%s/%s/%s/%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_ARTIST, strArtist.c_str(), strAlbum.c_str(), strFilename.c_str());
          else
            strReturn = StringUtils::Format("%s%s/%s/%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_ARTIST, strArtist.c_str(), strFilename.c_str());
        }
        else
          strReturn = StringUtils::Format("%s%s/%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_SONG, strFilename.c_str());
      }
      break;
    case CONTENT_ADDON_TYPE_ARTIST:
      strReturn = StringUtils::Format("%s%s/%s/%s/", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_ARTIST, strFilename.c_str());
      break;
    case CONTENT_ADDON_TYPE_PLAYLIST:
      strReturn = StringUtils::Format("%s%s/%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_PLAYLIST, strFilename.c_str());
      break;
    case CONTENT_ADDON_TYPE_DIRECTORY:
      strReturn = StringUtils::Format("%s%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), strFilename.c_str());
      break;
    case CONTENT_ADDON_TYPE_ALBUM:
      {
        if (strArtist.empty())
          strReturn = StringUtils::Format("%s%s/%s/%s/", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_ALBUM, strFilename.c_str());
        else
          strReturn = StringUtils::Format("%s%s/%s/%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), MUSIC_ARTIST, strArtist.c_str(), strFilename.c_str());
      }
      break;
    default:
      break;
    }

    return strReturn;
  }

  return strFilename;
}

std::string CContentAddon::VideoBuildPath(CONTENT_ADDON_TYPE type, const std::string& strFilename, CONTENT_VIDEO_TYPE videoType) const
{
  if (strFilename.empty())
    return strFilename;

  std::string strVideoType;
  switch (videoType)
  {
  case CONTENT_VIDEO_TYPE_MOVIES:
    strVideoType = VIDEO_MOVIES;
    break;
  case CONTENT_VIDEO_TYPE_TVSHOWS:
    strVideoType = VIDEO_TVSHOWS;
    break;
  case CONTENT_VIDEO_TYPE_MUSICVIDEOS:
    strVideoType = VIDEO_MUSICVIDEOS;
    break;
  default:
    return strFilename;
  }

  CURL fileName(strFilename);
  if (!URIUtils::IsInternetStream(fileName))
  {
    std::string strReturn;
    switch (type)
    {
    case CONTENT_ADDON_TYPE_VIDEO:
      {
        
        if (ProvidesVideoCodec())
          strReturn = StringUtils::Format("%s%s/%s/%s/%s", MUSIC_VIRTUAL_NODE, ID().c_str(), strVideoType.c_str(), VIDEO_TITLE, strFilename.c_str());
        else
          strReturn = strFilename;
      }
      break;
    case CONTENT_ADDON_TYPE_GENRE:
      strReturn = StringUtils::Format("%s%s/%s/%s/%s", VIDEO_VIRTUAL_NODE, ID().c_str(), strVideoType.c_str(), VIDEO_GENRE, strFilename.c_str());
      break;
    case CONTENT_ADDON_TYPE_YEAR:
      strReturn = StringUtils::Format("%s%s/%s/%s/%s", VIDEO_VIRTUAL_NODE, ID().c_str(), strVideoType.c_str(), VIDEO_YEAR, strFilename.c_str());
      break;
    default:
      break;
    }

    return strReturn;
  }

  return strFilename;
}

std::string CContentAddon::ContentBuildPath(const std::string& strPath)
{
  if (strPath.empty() || URIUtils::IsInternetStream(CURL(strPath)))
    return strPath;

  std::string retVal;
  if (!strPath.compare("[logo]"))
    retVal = Icon();
  else
    retVal = StringUtils::Format("%s%s/%s", CONTENT_NODE, ID().c_str(), strPath.c_str());
  return retVal;
}

bool CContentAddon::FileOpen(const std::string& strFileName, CONTENT_HANDLE* handle)
{
  CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);

  if (ProvidesFiles())
  {
    std::string strFilePath = GetFilename(strFileName);
    try { err = m_pStruct->FileOpen(strFilePath.c_str(), handle); }
    catch (exception &e) { LogException(e, __FUNCTION__); return false; }

    if (err != CONTENT_ERROR_NO_ERROR)
      CLog::Log(LOGERROR, "add-on '%s' returned an error from FileOpen(%s): %d", Name().c_str(), strFileName.c_str(), err);
  }

  return err == CONTENT_ERROR_NO_ERROR;
}

void CContentAddon::FileClose(CONTENT_HANDLE handle)
{
  if (ProvidesFiles())
  {
    try { m_pStruct->FileClose(handle); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }
}

unsigned int CContentAddon::FileRead(CONTENT_HANDLE handle, void* pBuffer, int64_t iBufLen)
{
  if (ProvidesFiles())
  {
    try { return m_pStruct->FileRead(handle, pBuffer, iBufLen); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  return 0;
}

bool CContentAddon::FileExists(const std::string& strFileName)
{
  if (ProvidesFiles())
  {
    std::string strFilePath = GetFilename(strFileName);
    try { return m_pStruct->FileExists(strFilePath.c_str()) == 0; }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  return false;
}

int CContentAddon::FileStat(const std::string& strFileName, struct __stat64* buffer)
{
  if (ProvidesFiles())
  {
    std::string strFilePath = GetFilename(strFileName);
    try { return m_pStruct->FileStat(strFilePath.c_str(), buffer); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  return -1;
}

int64_t CContentAddon::FileSeek(CONTENT_HANDLE handle, int64_t iFilePosition, int iWhence)
{
  if (ProvidesFiles())
  {
    try { return m_pStruct->FileSeek(handle, iFilePosition, iWhence); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  return 0;
}

int64_t CContentAddon::FileGetPosition(CONTENT_HANDLE handle)
{
  if (ProvidesFiles())
  {
    try { return m_pStruct->FileGetPosition(handle); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  return 0;
}

int64_t CContentAddon::FileGetLength(CONTENT_HANDLE handle)
{
  if (ProvidesFiles())
  {
    try { return m_pStruct->FileGetLength(handle); }
    catch (exception &e) { LogException(e, __FUNCTION__); }
  }

  return 0;
}

bool CContentAddon::FileGetDirectory(CFileItemList& items, const std::string& strPath)
{
  if (ProvidesFiles())
  {
    CONTENT_ADDON_FILELIST* retVal = NULL;
    CONTENT_ERROR err(CONTENT_ERROR_UNKNOWN);
    CLog::Log(LOGDEBUG, "getting directory '%s' from add-on '%s'", strPath.c_str(), Name().c_str());

    try
    {
      std::string strDirPath = GetFilename(strPath);
      err = m_pStruct->FileGetDirectory(&retVal, strDirPath.c_str());
    }
    catch (exception &e) { LogException(e, __FUNCTION__); }

    if (err == CONTENT_ERROR_NO_ERROR && retVal)
    {
      ReadMusicFiles(*retVal, items);
      FreeFileList(retVal);
      return true;
    }
  }

  return false;
}
