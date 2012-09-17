/*
 *      Copyright (C) 2012 Garrett Brown
 *      Copyright (C) 2012 Team XBMC
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

#include "Game.h"
#include "dbwrappers/Database.h"
#include "FileItem.h"

class CGameDatabase : public CDatabase
{
public:
  CGameDatabase(void) { }
  virtual ~CGameDatabase(void) { }

  virtual bool Open();

  virtual int GetMinVersion() const { return 1; }
  const char *GetBaseDBName() const { return "MyGames"; }

  CStdString GetPlatformById(int id);
  CStdString GetPublisherById(int id);
  CStdString GetGenreById(int id);
  CStdString GetCollectionById(int id);

  // General browsing
  bool GetGamesNav(const CStdString& strBaseDir, CFileItemList& items, int idGenre=-1, int idYear=-1, int idActor=-1, int idDirector=-1, int idStudio=-1, int idCountry=-1, int idSet=-1, int idTag=-1, const SortDescription &sortDescription = SortDescription());
  bool GetPlatformsNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter = Filter());
  bool GetGenresNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter = Filter());
  bool GetCollectionsNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter = Filter());
  bool GetPublishersNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter = Filter());
  bool GetDevelopersNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter = Filter());
  bool GetReviewersNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter = Filter());
  bool GetRatingNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter = Filter());
  bool GetYearsNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter = Filter());
  
  bool GetMostPlayedNav(const CStdString& strBaseDir, CFileItemList& items, unsigned int limit=0);
  bool GetRecentlyPlayedNav(const CStdString& strBaseDir, CFileItemList& items, unsigned int limit=0);
  bool GetRecentlyAddedNav(const CStdString& strBaseDir, CFileItemList& items, unsigned int limit=0);

protected:
  // For N:M linked tables, like genre and collection
  bool GetNavCrossLinkedTables(const CStdString& strBaseDir, CFileItemList& items, const CStdString& type, const Filter &filter = Filter());
  // For 1:N
  bool GetNavJoinedTables(const CStdString& strBaseDir, CFileItemList& items, const CStdString& type, const Filter &filter = Filter());
  // For simple table queries (named Single, but might pull in the path table if privacy is enabled)
  bool GetNavSingleTable(const CStdString& strBaseDir, CFileItemList& items, const CStdString& type, const Filter &filter = Filter());

private:
  bool CreateTables();
  /*!
   * @return The number of rows, or -1 if there was an error.
   */
  int RunQuery(const CStdString &sql);

  int AddGame(const CGame& game, bool bCheck = true);
  int AddPath(const CStdString& strPath);
  int AddPlatform(const CStdString& strPlatform);
  int AddPublisher(const CStdString& strPublisher);
  int AddGenre(const CStdString& strGenre);
  bool AddGenreGame(int idGame, int idGenre, int order);
  int AddCollection(const CStdString& strArtist);
  bool AddCollectionGame(int idGame, int idCollection, int order);


  /*!
    * @brief Update an old version of the database.
    * @param version The version to update the database from.
    * @return True if it was updated successfully, false otherwise.
    */
  bool UpdateOldVersion(int version);

  std::map<CStdString, int> m_pathCache; // TODO: static vars in function
  std::map<CStdString, int> m_platformCache;
  std::map<CStdString, int> m_publisherCache;
  std::map<CStdString, int> m_genreCache;
  std::map<CStdString, int> m_collectionCache;
  //std::map<CStdString, int> m_thumbCache;
};
