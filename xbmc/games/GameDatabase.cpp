/*
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

#include "GameDatabase.h"
#include "GameDbUrl.h"
#include "dbwrappers/dataset.h"
#include "games/tags/GameInfoTag.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "threads/SystemClock.h"
#include "utils/log.h"

//#include <set>
//#include <map>

using namespace std;

bool CGameDatabase::Open()
{
  return CDatabase::Open(g_advancedSettings.m_databaseGames);
}

bool CGameDatabase::CreateTables()
{
  bool bReturn;

  try
  {
    if (!CDatabase::CreateTables())
      return false;

    BeginTransaction();
    CLog::Log(LOGINFO, "GameDatabase: creating tables");
    
    CLog::Log(LOGDEBUG, "GameDatabase: creating table 'game'");
    m_pDS->exec(
      "CREATE TABLE game ( "
        "idGame integer primary key, "
        "strTitle varchar(512), " // index
        "strInternalTitle text, "
        "strOriginalTitle text, "
        "strAlternateTitle text, "
        "idPath integer, " // index, fk
        "strGameCode text, "
        "idPlatform integer, " // index, fk
        "strVersion text, "
        "strRegion text, " // NTSC or PAL
        "strCountry text, "
        "strMedia text, "
        "strPerspective text, "
        "strControllerType text, "
        "iPlayers integer, "
        "strGenres text, " // Only for appearance, searches on genre use genrelinkgame table
        "strDescription text, "
        "idPublisher integer, " // index, fk
        "strDeveloper text, "
        "strReviewer text, "
        "iYear integer, " // index
        "strRating text, "
        //"strURL text, " // ??
        "iFavorite integer, "
        // Collections in their own table
        "iPlayTime integer, " // index
        "strLastPlayed text " // index
      ")");
    m_pDS->exec("CREATE INDEX idxGameZ ON game ( strTitle )");
    m_pDS->exec("CREATE INDEX idxGameY ON game ( idPath )");
    m_pDS->exec("CREATE INDEX idxGameX ON game ( idPlatform )");
    m_pDS->exec("CREATE INDEX idxGameW ON game ( idPublisher )");
    m_pDS->exec("CREATE INDEX idxGameV ON game ( iYear )");
    m_pDS->exec("CREATE INDEX idxGameU ON game ( iPlayTime )");
    m_pDS->exec("CREATE INDEX idxGameT ON game ( strLastPlayed )");
    
    CLog::Log(LOGDEBUG, "GameDatabase: creating table 'path'");
    m_pDS->exec("CREATE TABLE path ( idPath integer primary key, strPath varchar(512), strHash text )\n");
    m_pDS->exec("CREATE INDEX idxPath ON path ( strPath )");

    // Platform is N:1 always. If the game is a ROM, it is the system that plays
    // that exact binary (not any of the other systems the game is available
    // for). Likewise, if the game is an EXE, the platform is PC/Windows, etc.
    // Additional logic (such as checking extensions) may be needed to reduce
    // multiple platforms returned by a scraper into the correct database entry.
    CLog::Log(LOGDEBUG, "GameDatabase: creating table 'platform'");
    m_pDS->exec("CREATE TABLE developer ( idPlatform integer primary key, strPlatform text )");
    
    CLog::Log(LOGDEBUG, "GameDatabase: creating table 'publisher'");
    m_pDS->exec("CREATE TABLE publisher ( idPublisher integer primary key, strPublisher text )");
    
    CLog::Log(LOGDEBUG, "GameDatabase: creating table 'genre'");
    m_pDS->exec("CREATE TABLE genre ( idGenre integer primary key, strPublisher text )");

    CLog::Log(LOGDEBUG, "GameDatabase: creating table 'genrelinkgame'");
    m_pDS->exec("CREATE TABLE genrelinkgame ( idGenre integer, idGame integer )");
    m_pDS->exec("CREATE UNIQUE INDEX ix_genrelinkgame_1 on genrelinkgame ( idGenre, idGame )");
    m_pDS->exec("CREATE UNIQUE INDEX ix_genrelinkgame_2 on genrelinkgame ( idGame, idGenre )");

    CLog::Log(LOGDEBUG, "GameDatabase: creating table 'collection'");
    m_pDS->exec("CREATE TABLE collections ( idCollection integer primary key, strPublisher text )");

    CLog::Log(LOGDEBUG, "GameDatabase: creating table 'collectionlinkgame'");
    m_pDS->exec("CREATE TABLE collectionlinkgame ( idCollection integer, idGame integer )");
    m_pDS->exec("CREATE UNIQUE INDEX ix_genrelinkgame_1 on collectionlinkgame ( idCollection, idGame )");
    m_pDS->exec("CREATE UNIQUE INDEX ix_genrelinkgame_2 on collectionlinkgame ( idGame, idCollection )");

    CommitTransaction();
    bReturn = true;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "GameDatabase: unable to create database tables (error %i)", (int)GetLastError());
    RollbackTransaction();
    bReturn = false;
  }

  return bReturn;
}

bool CGameDatabase::UpdateOldVersion(int iVersion)
{
  return true;
}

CStdString CGameDatabase::GetPlatformById(int id)
{
  return GetSingleValue("platform", "strName", PrepareSQL("idPlatform=%i", id));
}

CStdString CGameDatabase::GetPublisherById(int id)
{
  return GetSingleValue("publisher", "strName", PrepareSQL("idPublisher=%i", id));
}

CStdString CGameDatabase::GetGenreById(int id)
{
  return GetSingleValue("genre", "strName", PrepareSQL("idGenre=%i", id));
}

CStdString CGameDatabase::GetCollectionById(int id)
{
  return GetSingleValue("collection", "strName", PrepareSQL("idCollection=%i", id));
}



bool CGameDatabase::GetGamesNav(const CStdString& strBaseDir, CFileItemList& items, int idGenre /*=-1*/, int idYear /*=-1*/, int idActor /*=-1*/, int idDirector/*=-1*/, int idStudio/*=-1*/, int idCountry/*=-1*/, int idSet/*=-1*/, int idTag/*=-1*/, const SortDescription &sortDescription /*= SortDescription()*/)
{
  return false;
}





bool CGameDatabase::GetGenresNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter /* = Filter() */)
{
  return GetNavCrossLinkedTables(strBaseDir, items, "genre", filter);
}

bool CGameDatabase::GetCollectionsNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter /* = Filter() */)
{
  return GetNavCrossLinkedTables(strBaseDir, items, "collection", filter);
}

bool CGameDatabase::GetNavCrossLinkedTables(const CStdString& strBaseDir, CFileItemList& items, const CStdString &type, const Filter &filter /* = Filter() */)
{
  try
  {
    if (NULL == m_pDB.get()) return false;
    if (NULL == m_pDS.get()) return false;

    bool restricted = g_settings.GetMasterProfile().getLockMode() != LOCK_MODE_EVERYONE && !g_passwordManager.bMasterUser;

    CStdString strSQL;
    Filter extFilter = filter;
    if (!restricted)
    {
      strSQL = PrepareSQL(
        "SELECT %s.id%s, %s.str%s, count(game.playTime) "
        "FROM %s ",
        type.c_str(), type.c_str(), type.c_str(), type.c_str(), type.c_str()
      );
      extFilter.AppendJoin(PrepareSQL(
        "JOIN %slinkgame ON %slinkgame.id%s = %s.id%s "
        "JOIN game ON game.idGame = %slinkgame.idGame ",
        type.c_str(), type.c_str(), type.c_str(), type.c_str(), type.c_str(), type.c_str())
      );
      extFilter.AppendGroup(PrepareSQL("%s.id%s", type.c_str(), type.c_str()));
    }
    else
    {
      strSQL = PrepareSQL(
        "SELECT %s.id%s, %s.str%s, path.strPath, game.playTime "
        "FROM %s ",
        type.c_str(), type.c_str(), type.c_str(), type.c_str(), type.c_str()
      );
      extFilter.AppendJoin(PrepareSQL(
        "JOIN %slinkgame ON %s.id%s = %slinkgame.id%s "
        "JOIN game ON game.idGame = %slinkgame.idGame "
        "JOIN path ON path.idPath = game.idPath ",
        type.c_str(), type.c_str(), type.c_str(), type.c_str(), type.c_str(), type.c_str())
      );
    }
    
    CGameDbUrl gameUrl;
    if (!BuildSQL(strBaseDir, strSQL, extFilter, strSQL, gameUrl))
      return false;
    
    int iRowsFound = RunQuery(strSQL);
    if (iRowsFound <= 0)
      return iRowsFound == 0; // true if no rows, false if error (-1)

    // Comments are for type = "genre" as an example
    if (!restricted)
    {
      while (!m_pDS->eof())
      {
        CFileItemPtr pItem(new CFileItem(m_pDS->fv(1).get_asString()));
        pItem->GetGameInfoTag()->SetDatabaseID(m_pDS->fv(0).get_asInt());
        //pItem->GetGameInfoTag()->SetType(type);
        
        CGameDbUrl itemUrl = gameUrl;
        CStdString path;
        path.Format("%ld/", m_pDS->fv(0).get_asInt());
        itemUrl.AppendPath(path);
        pItem->SetPath(itemUrl.ToString());

        pItem->m_bIsFolder = true;
        pItem->SetLabelPreformated(true);
        // Total play time for all games in the category
        pItem->GetGameInfoTag()->SetPlayTime(m_pDS->fv(2).get_asInt());

        items.Add(pItem);
        m_pDS->next();
      }
      m_pDS->close();
    }
    else
    {
      // idGenre -> (strGenre -> playTime)
      map<int, pair<CStdString, int> > mapItems;
      map<int, pair<CStdString, int> >::iterator it;
      while (!m_pDS->eof())
      {
        int id = m_pDS->fv(0).get_asInt(); // idGenre
        CStdString str = m_pDS->fv(1).get_asString(); // strGenre

        // Is this item new?
        it = mapItems.find(id);
        if (it == mapItems.end())
        {
          // check path (strPath)
          if (g_passwordManager.IsDatabasePathUnlocked(CStdString(m_pDS->fv(2).get_asString()), g_settings.m_gameSources))
          {
            // fv(3) is game.playTime
            mapItems.insert(pair<int, pair<CStdString,int> >(id, pair<CStdString,int>(str, m_pDS->fv(3).get_asInt())));
          }
        }
        else
        {
          // Update the playtime
          it->second.second += m_pDS->fv(3).get_asInt();
        }
        m_pDS->next();
      }
      m_pDS->close();

      for (it = mapItems.begin(); it != mapItems.end(); ++it)
      {
        CFileItemPtr pItem(new CFileItem(it->second.first)); // strGenre
        pItem->GetGameInfoTag()->SetDatabaseID(it->first); // idGenre
        //pItem->GetGameInfoTag()->SetType(type);
        
        CGameDbUrl itemUrl = gameUrl;
        CStdString path;
        path.Format("%ld/", it->first); // idGenre
        itemUrl.AppendPath(path);
        pItem->SetPath(itemUrl.ToString());
        
        pItem->m_bIsFolder = true;
        pItem->GetGameInfoTag()->SetPlayTime(it->second.second);
        if (!items.Contains(pItem->GetPath()))
        {
          pItem->SetLabelPreformated(true);
          items.Add(pItem);
        }
      }
    }
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s failed", __FUNCTION__);
    return false;
  }
  return true;
}



bool CGameDatabase::GetPlatformsNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter /* = Filter() */)
{
  return GetNavJoinedTables(strBaseDir, items, "platform", filter);
}

bool CGameDatabase::GetPublishersNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter /* = Filter() */)
{
  return GetNavJoinedTables(strBaseDir, items, "publisher", filter);
}

bool CGameDatabase::GetNavJoinedTables(const CStdString& strBaseDir, CFileItemList& items, const CStdString &type, const Filter &filter /* = Filter() */)
{
  try
  {
    if (NULL == m_pDB.get()) return false;
    if (NULL == m_pDS.get()) return false;

    bool restricted = g_settings.GetMasterProfile().getLockMode() != LOCK_MODE_EVERYONE && !g_passwordManager.bMasterUser;

    CStdString strSQL;
    Filter extFilter = filter;
    if (!restricted)
    {
      strSQL = PrepareSQL(
        "SELECT %s.id%s, %s.str%s, count(game.playTime) "
        "FROM %s ",
        type.c_str(), type.c_str(), type.c_str(), type.c_str(), type.c_str()
      );
      extFilter.AppendJoin(PrepareSQL(
        "JOIN game ON game.id%s = %s.id%s ",
        type.c_str(), type.c_str(), type.c_str())
      );
      extFilter.AppendGroup(PrepareSQL("%s.id%s", type.c_str(), type.c_str()));
    }
    else
    {
      strSQL = PrepareSQL(
        "SELECT %s.id%s, %s.str%s, path.strPath, game.playTime "
        "FROM %s ",
        type.c_str(), type.c_str(), type.c_str(), type.c_str(), type.c_str()
      );
      extFilter.AppendJoin(PrepareSQL(
        "JOIN game ON game.id%s = %s.id%s "
        "JOIN path ON path.idPath = game.idPath",
        type.c_str(), type.c_str(), type.c_str(), type.c_str(), type.c_str(), type.c_str())
      );
    }
    
    CGameDbUrl gameUrl;
    if (!BuildSQL(strBaseDir, strSQL, extFilter, strSQL, gameUrl))
      return false;
    
    int iRowsFound = RunQuery(strSQL);
    if (iRowsFound <= 0)
      return iRowsFound == 0; // true if no rows, false if error (-1)

    // Comments are for type = "publisher" as an example
    if (!restricted)
    {
      while (!m_pDS->eof())
      {
        CFileItemPtr pItem(new CFileItem(m_pDS->fv(1).get_asString())); // strPublisher
        pItem->GetGameInfoTag()->SetDatabaseID(m_pDS->fv(0).get_asInt());
        //pItem->GetGameInfoTag()->SetType(type);
        
        CGameDbUrl itemUrl = gameUrl;
        CStdString path;
        path.Format("%ld/", m_pDS->fv(0).get_asInt()); // idPublisher
        itemUrl.AppendPath(path);
        pItem->SetPath(itemUrl.ToString());

        pItem->m_bIsFolder = true;
        pItem->SetLabelPreformated(true);
        // Total play time for all games in the category
        pItem->GetGameInfoTag()->SetPlayTime(m_pDS->fv(2).get_asInt());

        items.Add(pItem);
        m_pDS->next();
      }
      m_pDS->close();
    }
    else
    {
      // idPublisher -> (strPublisher -> playTime)
      map<int, pair<CStdString, int> > mapItems;
      map<int, pair<CStdString, int> >::iterator it;
      while (!m_pDS->eof())
      {
        int id = m_pDS->fv(0).get_asInt(); // idPublisher
        CStdString str = m_pDS->fv(1).get_asString(); // strPublisher

        // Is this item new?
        it = mapItems.find(id);
        if (it == mapItems.end())
        {
          // check path (strPath)
          if (g_passwordManager.IsDatabasePathUnlocked(CStdString(m_pDS->fv(2).get_asString()), g_settings.m_gameSources))
          {
            // fv(3) is game.playTime
            mapItems.insert(pair<int, pair<CStdString,int> >(id, pair<CStdString,int>(str, m_pDS->fv(3).get_asInt())));
          }
        }
        else
        {
          // Update the playtime
          it->second.second += m_pDS->fv(3).get_asInt();
        }
        m_pDS->next();
      }
      m_pDS->close();

      for (it = mapItems.begin(); it != mapItems.end(); ++it)
      {
        CFileItemPtr pItem(new CFileItem(it->second.first)); // strPublisher
        pItem->GetGameInfoTag()->SetDatabaseID(it->first); // idPublisher
        //pItem->GetGameInfoTag()->SetType(type);
        
        CGameDbUrl itemUrl = gameUrl;
        CStdString path;
        path.Format("%ld/", it->first); // idPublisher
        itemUrl.AppendPath(path);
        pItem->SetPath(itemUrl.ToString());
        
        pItem->m_bIsFolder = true;
        pItem->GetGameInfoTag()->SetPlayTime(it->second.second);
        if (!items.Contains(pItem->GetPath()))
        {
          pItem->SetLabelPreformated(true);
          items.Add(pItem);
        }
      }
    }
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s failed", __FUNCTION__);
    return false;
  }
  return true;
}

bool CGameDatabase::GetDevelopersNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter /* = Filter() */)
{
  //return GetNavCommon(strBaseDir, items, "developer", filter);
  return false;
}

bool CGameDatabase::GetReviewersNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter /* = Filter() */)
{
  //return GetNavCommon(strBaseDir, items, "reviewer", filter);
  return false;
}

bool CGameDatabase::GetRatingNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter /* = Filter() */)
{
  //return GetNavCommon(strBaseDir, items, "rating", filter);
  return false;
}

bool CGameDatabase::GetYearsNav(const CStdString& strBaseDir, CFileItemList& items, const Filter &filter /* = Filter() */)
{
  //return GetNavCommon(strBaseDir, items, "year", filter);
  return false;
}

//bool CGameDatabase::GetNavSingleTable(const CStdString& strBaseDir, CFileItemList& items, const CStdString &type, const Filter &filter /* = Filter() */)


int CGameDatabase::RunQuery(const CStdString &sql)
{
  unsigned int time = XbmcThreads::SystemClockMillis();
  int rows = -1;
  if (m_pDS->query(sql.c_str()))
  {
    rows = m_pDS->num_rows();
    if (rows == 0)
      m_pDS->close();
  }
  CLog::Log(LOGDEBUG, "GameDatabase: took %d ms for %d items query: %s", XbmcThreads::SystemClockMillis() - time, rows, sql.c_str());
  return rows;
}
