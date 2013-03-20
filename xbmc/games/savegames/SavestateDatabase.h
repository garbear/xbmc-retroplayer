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

  bool GetCRC(const CStdString &gamePath, CStdString &strCrc);

  bool GetAutoSaveByPath(const CStdString &gameClient, const CStdString &gamePath, CSavestate &savestate);
  bool GetAutoSaveByCrc(const CStdString &gameClient, const CStdString &gameCrc, CSavestate &savestate);

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

  virtual CFileItem *CreateFileItem(const CVariant &object, int id) const;

private:
  bool GetAutoSave(const CStdString &gameClient, bool usePath, const CStdString &value, CSavestate &savestate);
};
