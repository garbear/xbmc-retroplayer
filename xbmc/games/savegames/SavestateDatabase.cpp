/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#define USE_BSON

#include "SavestateDatabase.h"
#include "FileItem.h"
#include "settings/AdvancedSettings.h"
#ifdef USE_BSON
#include "utils/BSONVariantParser.h"
#include "utils/BSONVariantWriter.h"
#else
#include "utils/JSONVariantParser.h"
#include "utils/JSONVariantWriter.h"
#endif
#include "utils/log.h"
#include "utils/Variant.h"
#include "XBDateTime.h"

using namespace std;
using namespace XFILE;

CSavestateDatabase::CSavestateDatabase() : CDynamicDatabase("savestate")
{
  BeginDeclarations();
  DeclareIndex("path", "VARCHAR(512)");
  DeclareOneToMany("gamepath", "VARCHAR(512)");
  DeclareOneToMany("gamecrc", "CHAR(8)");
  DeclareOneToMany("gameclient", "VARCHAR(64)");
}

bool CSavestateDatabase::Open()
{
  return CDynamicDatabase::Open(g_advancedSettings.m_databaseSavestates);
}

bool CSavestateDatabase::CreateTables()
{
  try
  {
    BeginTransaction();
    if (!CDynamicDatabase::CreateTables())
      return false;

    CommitTransaction();
    return true;
  }
  catch (dbiplus::DbErrors&)
  {
    CLog::Log(LOGERROR, "SavestateDatabase: unable to create tables (error %i)", (int)GetLastError());
    RollbackTransaction();
  }
  return false;
}

bool CSavestateDatabase::UpdateOldVersion(int version)
{
  if (version < 1)
  {
    BeginDeclarations();
    DeclareIndex("path", "VARCHAR(512)");
    DeclareOneToMany("gamepath", "VARCHAR(512)");
    DeclareOneToMany("gamecrc", "CHAR(8)");
    DeclareOneToMany("gameclient", "CHAR(8)");
  }
  return true;
}

bool CSavestateDatabase::Exists(const CVariant &object, int &idObject)
{
  if (!IsValid(object))
    return false;

  CStdString strSQL = PrepareSQL(
    "SELECT savestate.idsavestate "
    "FROM savestate "
    "WHERE path='%s'",
    object["path"].asString().c_str()
  );

  if (m_pDS->query(strSQL.c_str()))
  {
    bool bFound = false;
    if (m_pDS->num_rows() != 0)
    {
      idObject = m_pDS->fv(0).get_asInt();
      bFound = true;
    }
    m_pDS->close();
    return bFound;
  }

  return false;
}

bool CSavestateDatabase::IsValid(const CVariant &object) const
{
  return !object["path"].asString().empty();
}

CFileItem* CSavestateDatabase::CreateFileItem(const CVariant &object) const
{
  CSavestate p(object);
  CFileItem *item = new CFileItem(p.GetLabel());

  item->SetPath(p.GetPath());
  if (p.HasThumbnail())
    item->SetArt("thumb", p.GetThumbnail());
  else
    item->SetArt("thumb", "DefaultHardDisk.png");

  // Use the slot number as the second label (or blank if a non-slot save type)
  if (p.GetSaveType() == CSavestate::SAVETYPE_SLOT)
  {
    CStdString strSlot;
    strSlot.Format("%u", p.GetSlot());
    item->SetLabel2(strSlot);
  }

  CDateTime timestamp;
  timestamp.SetFromDBDateTime(p.GetTimestamp());
  item->m_dateTime = timestamp;

  // Provide the "duration" property for CGUIInfoManager::GetItemLabel() to
  // determine the duration of the file item.
  item->SetProperty("duration", (uint64_t)p.GetPlaytimeWallClock());
  item->SetProperty("gameclient", p.GetGameClient());

  item->m_dwSize = p.GetSize();

  item->m_bIsFolder = false;
  return item;
}

bool CSavestateDatabase::GetCRC(const CStdString &gamePath, CStdString &strCrc)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  CStdString strSQL;

  try
  {
    int idGamePath;
    if (GetItemID("gamepath", gamePath, idGamePath))
    {
      // The strategy here is outlined in CDynamicDatabase::GetItemNav()
      // This assumes all savestates with CRC X have gamepath Y
      strSQL = PrepareSQL(
        "SELECT gamecrc "
        "FROM gamecrc "
        "WHERE idgamecrc IN ("
          "SELECT idgamecrc "
          "FROM savestate "
          "WHERE idgamepath=%d "
          "LIMIT 1"
        ")",
        idGamePath
      );
      if (m_pDS->query(strSQL.c_str()))
      {
        if (m_pDS->num_rows() != 0)
        {
          strCrc = m_pDS->fv(0).get_asString();
          m_pDS->close();
          return !strCrc.empty();
        }
        m_pDS->close();
      }
    }
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - Unable to get CRC. SQL: %s", __FUNCTION__, strSQL.c_str());
  }
  return false;
}

void CSavestateDatabase::UpdateCRC(const CStdString &gamePath, const CStdString &strCrc)
{
  int idGameCrc;
  if (GetItemID("gamecrc", strCrc, idGameCrc))
  {
    // CRC exists in database. Update existing savestates
    CSavestate savestate;
    CFileItemList items;
    map<string, long> predicates;
    predicates["gamecrc"] = idGameCrc;
    GetObjectsNav(items, predicates);
    for (int i = 0; i < items.Size(); i++)
    {
      if (GetObjectByIndex("path", items[i]->GetPath(), &savestate))
      {
        savestate.SetGamePath(gamePath);
        AddObject(&savestate);
      }
    }
  }
}

bool CSavestateDatabase::GetAutoSaveByPath(const CStdString &gameClient, const CStdString &gamePath, CSavestate &savestate)
{
  return GetAutoSave(gameClient, true, gamePath, savestate);
}

bool CSavestateDatabase::GetAutoSaveByCrc(const CStdString &gameClient, const CStdString &gameCrc, CSavestate &savestate)
{
  return GetAutoSave(gameClient, false, gameCrc, savestate);
}

bool CSavestateDatabase::GetAutoSave(const CStdString &gameClient, bool usePath, const CStdString &value, CSavestate &savestate)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;
  
  CStdString strSQL;

  int idGameClient = -1;
  int idGamePath   = -1;
  int idGameCrc    = -1;

  bool success = GetItemID("gameclient", gameClient, idGameClient);
  if (usePath)
    success &= GetItemID("gamepath", value, idGamePath);
  else
    success &= GetItemID("gamecrc", value, idGameCrc);
  if (!success)
    return false;

  try
  {
    strSQL = PrepareSQL(
      "SELECT idsavestate, strContentBSON64 "
      "FROM savestate "
      "WHERE idgameclient=%d AND %s=%d",
      idGameClient, usePath ? "gamepath" : "gamecrc", usePath ? idGamePath : idGameCrc
    );
    if (m_pDS->query(strSQL.c_str()))
    {
      while (!m_pDS->eof())
      {
#ifdef USE_BSON
        CVariant var = CBSONVariantParser::ParseBase64(m_pDS->fv(1).get_asString());
#else
        const unsigned char *output = reinterpret_cast<const unsigned char*>(m_pDS->fv(1).get_asString().c_str());
        CVariant var = CJSONVariantParser::Parse(output, m_pDS->fv(1).get_asString().size());
#endif
        if (var["autosave"].asBoolean())
        {
          savestate.Deserialize(var);
          savestate.SetDatabaseId(m_pDS->fv(0).get_asInt());
          return true;
        }
        m_pDS->next();
      }
      m_pDS->close();
    }
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - Unable to enumerate objects. SQL: %s", __FUNCTION__, strSQL.c_str());
  }
  return false;
}

bool CSavestateDatabase::Load(CSavestate &savestate, std::vector<uint8_t> &data)
{
  // Try reading the data first, and hit the database only if that succeeds
  return savestate.Read(data) && GetObjectByIndex("path", savestate.GetPath(), &savestate);
}

bool CSavestateDatabase::Save(CSavestate &savestate, const std::vector<uint8_t> &data)
{
  savestate.SetSize(data.size());
  if (savestate.Write(data))
  {
    if (AddObject(&savestate) != -1)
      return true;
    else
      CLog::Log(LOGERROR, "CSavestateDatabase: Failed to update the database with save state information");
  }
  return false;
}

bool CSavestateDatabase::RenameSaveState(const CStdString &saveStatePath, const CStdString &newLabel)
{
  CSavestate savestate;
  if (GetObjectByIndex("path", saveStatePath.c_str(), &savestate) && savestate.GetLabel() != newLabel)
  {
    if (savestate.Rename(newLabel))
      return AddObject(&savestate) != -1;
  }
  return false;
}

bool CSavestateDatabase::DeleteSaveState(const CStdString &saveStatePath, bool deleteOrphans /* = true */)
{
  CSavestate savestate;
  savestate.SetPath(saveStatePath);
  if (!DeleteObjectByIndex("path", saveStatePath.c_str(), deleteOrphans))
  {
    // Failed to delete the object. If it still exists, don't delete the save
    // state from the file system, as that would cause inconsistencies
    if (GetObjectByIndex("path", saveStatePath.c_str(), &savestate))
      return false;
  }
  return savestate.Delete();
}
