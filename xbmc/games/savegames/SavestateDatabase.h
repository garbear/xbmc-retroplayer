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
#pragma once

#include "dbwrappers/DynamicDatabase.h"
#include "Savestate.h"
#include "utils/StdString.h"

class CSavestateDatabase : public CDynamicDatabase
{
public:
  CSavestateDatabase();
  virtual ~CSavestateDatabase() { }

  virtual bool Open();

  /**
   * Gets the CRC of the corresponding game path. A savestate record must exist
   * in the database with the game path. If multiple save states have the same
   * game path, it is assumed that they all have the same game CRC as only the
   * first is chosen.
   */
  bool GetCRC(const CStdString &gamePath, CStdString &strCrc);

  /**
   * Precondition: Game path is not in the database. If the CRC is in the
   * database, this will update stale game paths to point to it.
   */
  void UpdateCRC(const CStdString &gamePath, const CStdString &strCrc);

  bool GetAutoSaveByPath(const CStdString &gameClient, const CStdString &gamePath, CSavestate &savestate);
  bool GetAutoSaveByCrc(const CStdString &gameClient, const CStdString &gameCrc, CSavestate &savestate);

  bool Load(CSavestate &savestate, std::vector<uint8_t> &data);
  bool Save(CSavestate &savestate, const std::vector<uint8_t> &data);
  bool RenameSaveState(const CStdString &saveStatePath, const CStdString &newLabel);
  bool DeleteSaveState(const CStdString &saveStatePath, bool deleteOrphans = true);

protected:
  virtual int GetMinVersion() const { return 1; }
  virtual const char *GetBaseDBName() const { return "MySavestates"; }

  virtual bool CreateTables();
  virtual bool UpdateOldVersion(int version);

  /*!
   * Uniqueness is quantified by path
   * @throw dbiplus::DbErrors
   */
  virtual bool Exists(const CVariant &object, int &idObject);
  virtual bool IsValid(const CVariant &object) const;

  virtual CFileItem *CreateFileItem(const CVariant &object) const;

  bool GetAutoSave(const CStdString &gameClient, bool usePath, const CStdString &value, CSavestate &savestate);

  // TODO: Add std::map<string, int> cache for gamepath, gamecrc and gameclient IDs
};
