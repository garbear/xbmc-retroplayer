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

#include "utils/IDeserializable.h"
#include "utils/ISerializable.h"
#include "utils/StdString.h"

#include <stdint.h>
#include <vector>

#define MAX_SAVESTATE_CRC_LENGTH   40 * 1024 * 1024 // MiB

class CDateTime;

class CSavestate : public ISerializable, public IDeserializable
{
public:
  enum SAVETYPE
  {
    SAVETYPE_AUTO,
    SAVETYPE_SLOT,
    SAVETYPE_LABEL,
  };

  CSavestate() { Reset(); }
  CSavestate(const CVariant &object);
  void Reset();
  CSavestate(const CSavestate &other) { *this = other; }
  const CSavestate& operator=(const CSavestate &rhs);
  
  bool Read(std::vector<uint8_t> &data);
  bool Write(const std::vector<uint8_t> &data, bool bThumbnail = true);
  bool Rename(const CStdString &newLabel);
  bool Delete();

  virtual void Serialize(CVariant& value) const;
  virtual void Deserialize(const CVariant& value);

  // Serializable information

  /**
   * Path to savestate is derived from game client and game CRC. Returns empty
   * if either of these is unknown. Format is
   *
   * Autosave (hex is game CRC):
   *     special://savegames/gameclient.id/feba62c2.sav
   *
   * Save type slot (digit after the underscore is slot 1-9):
   *     special://savegames/gameclient.id/feba62c2_1.sav
   *
   * Save type label (hex after the underscore is CRC of the label):
   *     special://savegames/gameclient.id/feba62c2_8dc22669.sav
   */
  const CStdString &GetPath() const;
  void SetPath(const CStdString &path);

  CStdString GetThumbnail() const;

  int GetDatabaseId() const { return m_databaseId; }
  void SetDatabaseId(int databaseId) { m_databaseId = databaseId; }
  bool IsDatabaseObject() const { return m_databaseId != -1; }

  SAVETYPE GetSaveType() const { return m_bAutoSave ? SAVETYPE_AUTO : m_slot ? SAVETYPE_SLOT : SAVETYPE_LABEL; }
  void SetSaveTypeAuto();

  unsigned int GetSlot() const { return m_slot; }
  void SetSaveTypeSlot(unsigned int slot);

  const CStdString &GetLabel() const { return m_label; }
  void SetSaveTypeLabel(const CStdString &label);

  // Excluding header (in case XBMC savestate header is written with the file in the future)
  size_t GetSize() const { return m_size; }
  void SetSize(size_t size) { m_size = size; }

  const CStdString &GetGamePath() const { return m_gamePath; }
  void SetGamePath(const CStdString &gamePath) { m_gamePath = gamePath; }

  const CStdString &GetGameClient() const { return m_gameClient; }
  void SetGameClient(const CStdString &gameClient) { m_gameClient = gameClient; }

  /**
   * The game file CRC is used to achieve path-independence. However, to avoid
   * taking CRCs of really large files, we fall back to CRCing the path name at
   * the expense of path-independence. Outside of CSavestate, the file size
   * limit should be tested using MAX_SAVESTATE_CRC_LENGTH.
   */
  const CStdString &GetGameCRC() const { return m_gameCRC; }
  void SetGameCRC(const CStdString &gameCRC) { m_gameCRC = gameCRC; }
  void SetGameCRCFromFile(const CStdString &filename, bool forceFilename = false);
  void SetGameCRCFromFile(const char *data, size_t length);

  uint64_t GetPlaytimeFrames() const { return m_playtimeFrames; }
  void SetPlaytimeFrames(uint64_t playtimeFrames) { m_playtimeFrames = playtimeFrames; }

  double GetPlaytimeWallClock() const { return m_playtimeWallClock; } // seconds
  void SetPlaytimeWallClock(double playtimeWallClock) { m_playtimeWallClock = playtimeWallClock; } // seconds

  const CStdString &GetTimestamp() const { return m_timestamp; } // DB datetime
  void SetTimestamp(const CStdString &strTimestamp = ""); // Sets current time if empty

  double HasThumbnail() const { return m_bHasThumbnail; }
  void SetHasThumbnail(bool hasThumbnail) { m_bHasThumbnail = hasThumbnail; }

private:
  // If we consider writing header information to ROM files (using BSON most favorably)
  static const unsigned int SAVESTATE_VERSION = 1;

  mutable CStdString m_path;

  int          m_databaseId;
  bool         m_bAutoSave;
  unsigned int m_slot;
  CStdString   m_label;
  size_t       m_size; // excluding XBMC header
  CStdString   m_gamePath;
  CStdString   m_gameClient;
  CStdString   m_gameCRC;
  uint64_t     m_playtimeFrames;
  double       m_playtimeWallClock; // seconds
  CStdString   m_timestamp;
  bool         m_bHasThumbnail;
};
