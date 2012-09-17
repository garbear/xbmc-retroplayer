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
#include "utils/URIUtils.h"

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
      "CREATE TABLE game ("
        "idGame integer primary key,"
        "strTitle varchar(512)," // index
        "strInternalTitle text,"
        "strOriginalTitle text,"
        "strAlternateTitle text,"
        "idPath integer," // index, fk
        "strGameCode text,"
        "idPlatform integer," // index, fk
        "strVersion text,"
        "strFormat text," // NTSC or PAL
        "strRegion text,"
        "strLanguage text,"
        "strMedia text,"
        "strPerspective text,"
        "strControllerType text,"
        "iPlayers integer,"
        "strDescription text,"
        "idPublisher integer," // index, fk
        "strDeveloper text,"
        "strReviewer text,"
        "iYear integer," // index
        "strRating text,"
        "strFileName text,"
        "dwFileNameCRC text,"
        "idThumb integer,"
        "iFavorite integer not null default '0',"
        // Collections in their own table
        "iPlayTime integer," // index
        "strLastPlayed varchar(20) default NULL" // index
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
    m_pDS->exec("CREATE TABLE genrelinkgame ( idGenre integer, idGame integer, iOrder integer )");
    m_pDS->exec("CREATE UNIQUE INDEX ix_genrelinkgame_1 on genrelinkgame ( idGenre, idGame )");
    m_pDS->exec("CREATE UNIQUE INDEX ix_genrelinkgame_2 on genrelinkgame ( idGame, idGenre )");

    CLog::Log(LOGDEBUG, "GameDatabase: creating table 'collection'");
    m_pDS->exec("CREATE TABLE collections ( idCollection integer primary key, strPublisher text )");

    CLog::Log(LOGDEBUG, "GameDatabase: creating table 'collectionlinkgame'");
    m_pDS->exec("CREATE TABLE collectionlinkgame ( idCollection integer, idGame integer, iOrder integer )");
    m_pDS->exec("CREATE UNIQUE INDEX ix_genrelinkgame_1 on collectionlinkgame ( idCollection, idGame )");
    m_pDS->exec("CREATE UNIQUE INDEX ix_genrelinkgame_2 on collectionlinkgame ( idGame, idCollection )");
    
    CLog::Log(LOGDEBUG, "GameDatabase: creating table 'art'");
    m_pDS->exec("CREATE TABLE art ( idArt integer primary key, idMedia integer, strMediaType text, strType text, strUrl text )");
    m_pDS->exec("CREATE INDEX ix_art ON art ( idMedia, strMediaType(20), strType(20) )");
    // TODO: art triggers

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

int CGameDatabase::AddGame(const CGame& game, bool bCheck)
{
  int idGame = -1;
  CStdString strSQL;
  try
  {
    // We need at least the title
    if (game.m_tag.GetTitle().IsEmpty())
      return -1;

    CStdString strPath, strFileName;
    URIUtils::Split(game.m_tag.GetURL(), strPath, strFileName);

    if (NULL == m_pDB.get()) return -1;
    if (NULL == m_pDS.get()) return -1;

    int idPath = AddPath(strPath);
    int idPlatform = AddPlatform(game.m_tag.GetPlatform());
    int idPublisher = AddPublisher(game.m_tag.GetPlatform());
    
    DWORD crc = ComputeCRC(game.m_tag.GetURL());

    bool bInsert = true;
    
    if (bCheck)
    {
      strSQL = PrepareSQL("SELECT * FROM game WHERE " /* idAlbum=%i and */ "dwFileNameCRC='%ul' and strTitle='%s'",
        crc, game.m_tag.GetURL());

      if (!m_pDS->query(strSQL.c_str()))
        return -1;

      if (m_pDS->num_rows() != 0)
      {
        idGame = m_pDS->fv("idGame").get_asInt();
        bInsert = false;
      }
      m_pDS->close();
    }
    if (bInsert)
    {
      CStdString strSQL1;

      CStdString strIdGame;
      if (game.m_tag.GetDatabaseID() < 0)
        strIdGame = "NULL";
      else
        strIdGame.Format("%d", game.m_tag.GetDatabaseID());

      // We use replace because it can handle both inserting a new song and
      // replacing an existing song's record if the given idSong already exists
      strSQL=PrepareSQL(
        "REPLACE INTO game ( "
          "idGame,"       "strTitle,"       "strInternalTitle,"  "strOriginalTitle," "strAlternateTitle," "idPath,"
          "strGameCode,"  "idPlatform,"     "strVersion,"        "strFormat,"        "strRegion,"         "strLanguage,"
          "strMedia,"     "strPerspective," "strControllerType," "iPlayers,"         "strDescription,"    "idPublisher,"
          "strDeveloper," "strReviewer,"    "iYear,"             "strRating,"        "strFileName,"       "dwFileNameCRC,"
          "idThumb,"      "iFavorite,"      "iPlayTime,"         "strLastPlayed"
        ") VALUES ("
          "%i,"           "'%s,'"           "'%s,'"              "'%s,'"             "'%s,'"              "%i,"
          "'%s,'"         "%i,"             "'%s,'"              "'%s,'"             "'%s,'"              "'%s,'"
          "'%s,'"         "'%s,'"           "'%s,'"              "%i,"               "'%s,'"              "%i,"
          "'%s,'"         "'%s,'"           "%i,"                "'%s,'"             "'%s,'"              "'%ul'"
          "%i,"           "%i,"             "%i,"                "%s" /* No quotes on strLastPlayed */
        ")",
        // Row 1
        strIdGame.c_str(),
        game.m_tag.GetTitle().c_str(),
        game.m_tag.GetInternalTitle().c_str(),
        game.m_tag.GetOriginalTitle().c_str(),
        game.m_tag.GetAlternateTitle().c_str(),
        idPath,
        // Row 2
        game.m_tag.GetCode().c_str(),
        idPlatform,
        game.m_tag.GetVersion(),
        game.m_tag.GetFormat(),
        game.m_tag.GetRegion(),
        game.m_tag.GetLanguage(),
        // Row 3
        game.m_tag.GetMedia().c_str(),
        game.m_tag.GetPerspective().c_str(),
        game.m_tag.GetControllerType().c_str(),
        game.m_tag.GetPlayers(),
        game.m_tag.GetDescription().c_str(),
        idPublisher,
        // Row 4
        game.m_tag.GetDeveloper().c_str(),
        game.m_tag.GetReviewer().c_str(),
        game.m_tag.GetYear(),
        game.m_tag.GetRating().c_str(),
        strFileName.c_str(),
        crc,
        // Row 5
        0, // idThumb
        game.m_tag.IsFavorite() ? 1 : 0,
        game.m_tag.GetPlayTime(),
        game.m_tag.GetLastPlayed().IsValid() ? CStdString("'" + game.m_tag.GetLastPlayed().GetAsDBDateTime() + "'").c_str() : "NULL"
      );

      m_pDS->exec(strSQL.c_str());

      if (game.m_tag.GetDatabaseID() < 0)
        idGame = (int)m_pDS->lastinsertid();
      else
        idGame = game.m_tag.GetDatabaseID();
    }

    /*
    if (!song.strThumb.empty())
      SetArtForItem(idSong, "song", "thumb", song.strThumb);
    */

    for (unsigned int i = 0; i < game.m_tag.GetGenres().size(); i++)
    {
      int idGenre = AddGenre(game.m_tag.GetGenres().at(i));
      //bool featured = (i != 0);
      AddGenreGame(idGenre, idGame, i);
    }

    for (unsigned int i = 0; i < game.m_tag.GetCollections().size(); i++)
    {
      int idCollection = AddCollection(game.m_tag.GetCollections().at(i));
      //bool featured = (i != 0);
      AddCollectionGame(idCollection, idGame, i);
    }

    //AnnounceUpdate("game", idGame);
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "GameDatabase: unable to add game (%s)", strSQL.c_str());
  }
  return idGame;
}

int CGameDatabase::AddPath(const CStdString& strPath1)
{
  CStdString strSQL;
  try
  {
    CStdString strPath(strPath1);
    if (!URIUtils::HasSlashAtEnd(strPath))
      URIUtils::AddSlashAtEnd(strPath);

    if (NULL == m_pDB.get()) return -1;
    if (NULL == m_pDS.get()) return -1;

    map <CStdString, int>::const_iterator it;

    it = m_pathCache.find(strPath);
    if (it != m_pathCache.end())
      return it->second;

    strSQL = PrepareSQL("SELECT * FROM path WHERE strPath='%s'", strPath.c_str());
    m_pDS->query(strSQL.c_str());
    if (m_pDS->num_rows() == 0)
    {
      m_pDS->close();
      // Doesn't exists, add it
      strSQL = PrepareSQL("INSERT INTO path (idPath, strPath) VALUES ( NULL, '%s' )", strPath.c_str());
      m_pDS->exec(strSQL.c_str());

      int idPath = (int)m_pDS->lastinsertid();
      m_pathCache.insert(pair<CStdString, int>(strPath, idPath));
      return idPath;
    }
    else
    {
      int idPath = m_pDS->fv("idPath").get_asInt();
      m_pDS->close();
      m_pathCache.insert(pair<CStdString, int>(strPath, idPath));
      return idPath;
    }
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "GameDatabase: Unable to add path (%s)", strSQL.c_str());
  }
  return -1;
}

int CGameDatabase::AddPlatform(const CStdString& strPlatform)
{
  CStdString strSQL;
  try
  {
    if (NULL == m_pDB.get()) return -1;
    if (NULL == m_pDS.get()) return -1;

    map<CStdString, int>::const_iterator it;

    it = m_platformCache.find(strPlatform);
    if (it != m_platformCache.end())
      return it->second;

    strSQL = PrepareSQL("SELECT * FROM platform WHERE strPlatform='%s'", strPlatform.c_str());
    m_pDS->query(strSQL.c_str());
    if (m_pDS->num_rows() == 0)
    {
      m_pDS->close();
      // Doesn't exists, add it
      strSQL = PrepareSQL("INSERT INTO platform (idPlatform, strPlatform) VALUES (NULL, '%s')", strPlatform.c_str());
      m_pDS->exec(strSQL.c_str());

      int idPlatform = (int)m_pDS->lastinsertid();
      m_platformCache.insert(pair<CStdString, int>(strPlatform, idPlatform));
      return idPlatform;
    }
    else
    {
      int idPlatform = m_pDS->fv("idPlatform").get_asInt();
      m_pDS->close();
      m_platformCache.insert(pair<CStdString, int>(strPlatform, idPlatform));
      return idPlatform;
    }
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "GameDatabase: Unable to add platform (%s)", strSQL.c_str());
  }
  return -1;
}

int CGameDatabase::AddPublisher(const CStdString& strPublisher)
{
  CStdString strSQL;
  try
  {
    if (NULL == m_pDB.get()) return -1;
    if (NULL == m_pDS.get()) return -1;

    map<CStdString, int>::const_iterator it;

    it = m_publisherCache.find(strPublisher);
    if (it != m_publisherCache.end())
      return it->second;

    strSQL = PrepareSQL("SELECT * FROM publisher WHERE strPublisher='%s'", strPublisher.c_str());
    m_pDS->query(strSQL.c_str());
    if (m_pDS->num_rows() == 0)
    {
      m_pDS->close();
      // Doesn't exists, add it
      strSQL = PrepareSQL("INSERT INTO publisher (idPublisher, strPublisher) VALUES (NULL, '%s')", strPublisher.c_str());
      m_pDS->exec(strSQL.c_str());

      int idPublisher = (int)m_pDS->lastinsertid();
      m_publisherCache.insert(pair<CStdString, int>(strPublisher, idPublisher));
      return idPublisher;
    }
    else
    {
      int idPublisher = m_pDS->fv("idPublisher").get_asInt();
      m_pDS->close();
      m_publisherCache.insert(pair<CStdString, int>(strPublisher, idPublisher));
      return idPublisher;
    }
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "GameDatabase: Unable to add publisher (%s)", strSQL.c_str());
  }
  return -1;
}

int CGameDatabase::AddGenre(const CStdString& strGenre)
{
  CStdString strSQL;
  try
  {
    if (NULL == m_pDB.get()) return -1;
    if (NULL == m_pDS.get()) return -1;

    map<CStdString, int>::const_iterator it;

    it = m_genreCache.find(strGenre);
    if (it != m_genreCache.end())
      return it->second;

    strSQL = PrepareSQL("SELECT * FROM genre WHERE strGenre='%s'", strGenre.c_str());
    m_pDS->query(strSQL.c_str());
    if (m_pDS->num_rows() == 0)
    {
      m_pDS->close();
      // Doesn't exists, add it
      strSQL = PrepareSQL("INSERT INTO genre (idrGenre, strGenre) VALUES (NULL, '%s')", strGenre.c_str());
      m_pDS->exec(strSQL.c_str());

      int idGenre = (int)m_pDS->lastinsertid();
      m_genreCache.insert(pair<CStdString, int>(strGenre, idGenre));
      return idGenre;
    }
    else
    {
      int idGenre = m_pDS->fv("idGenre").get_asInt();
      m_pDS->close();
      m_genreCache.insert(pair<CStdString, int>(strGenre, idGenre));
      return idGenre;
    }
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "GameDatabase: Unable to add genre (%s)", strSQL.c_str());
  }
  return -1;
}

int CGameDatabase::AddCollection(const CStdString& strCollection)
{
  CStdString strSQL;
  try
  {
    if (NULL == m_pDB.get()) return -1;
    if (NULL == m_pDS.get()) return -1;

    map<CStdString, int>::const_iterator it;

    it = m_collectionCache.find(strCollection);
    if (it != m_collectionCache.end())
      return it->second;

    strSQL = PrepareSQL("SELECT * FROM collection WHERE strCollection='%s'", strCollection.c_str());
    m_pDS->query(strSQL.c_str());
    if (m_pDS->num_rows() == 0)
    {
      m_pDS->close();
      // Doesn't exists, add it
      strSQL = PrepareSQL("INSERT INTO collection (idCollection, strCollection) VALUES (NULL, '%s')", strCollection.c_str());
      m_pDS->exec(strSQL.c_str());

      int idCollection = (int)m_pDS->lastinsertid();
      m_collectionCache.insert(pair<CStdString, int>(strCollection, idCollection));
      return idCollection;
    }
    else
    {
      int idCollection = m_pDS->fv("idCollection").get_asInt();
      m_pDS->close();
      m_collectionCache.insert(pair<CStdString, int>(strCollection, idCollection));
      return idCollection;
    }
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "GameDatabase: Unable to add collection (%s)", strSQL.c_str());
  }
  return -1;
}

bool CGameDatabase::AddGenreGame(int idGame, int idGenre, int order)
{
  CStdString strSQL;
  strSQL=PrepareSQL(
    "REPLACE INTO genrelinkgame (idGenre, idGame, iOrder) "
    "VALUES (%i, %i, %i)",
    idGenre, idGame, order
  );
  return ExecuteQuery(strSQL);
};

bool CGameDatabase::AddCollectionGame(int idGame, int idCollection, int order)
{
  CStdString strSQL;
  strSQL=PrepareSQL(
    "REPLACE INTO collectionlinkgame (idCollection, idGame, iOrder) "
    "VALUES (%i, %i, %i)",
    idCollection, idGame, order
  );
  return ExecuteQuery(strSQL);
};
