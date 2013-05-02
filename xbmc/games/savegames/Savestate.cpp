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

#include "Savestate.h"
#include "Application.h"
#include "cores/VideoRenderers/RenderManager.h"
#include "cores/VideoRenderers/RenderCapture.h"
#include "filesystem/File.h"
#include "guilib/LocalizeStrings.h"
#include "pictures/Picture.h"
#include "profiles/ProfilesManager.h"
#include "settings/AdvancedSettings.h"
//#include "settings/Settings.h"
#include "utils/Crc32.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"
#include "XBDateTime.h"

#define SAVESTATE_EXTENSION     ".sav"
#define SAVESTATE_THUMB_WIDTH   (g_advancedSettings.GetThumbSize())

using namespace XFILE;

void CSavestate::Reset()
{
  m_path.clear();
  m_databaseId = -1;
  m_bAutoSave = true;
  m_slot = 0;
  m_label.clear();
  m_size = 0;
  m_gamePath.clear();
  m_gameClient.clear();
  m_gameCRC.clear();
  m_playtimeFrames = 0;
  m_playtimeWallClock = 0.0;
  m_timestamp.clear();
  m_bHasThumbnail = false;
}

CSavestate::CSavestate(const CVariant &object)
{
  Deserialize(object);
}

const CSavestate& CSavestate::operator=(const CSavestate& rhs)
{
  if (this != &rhs)
  {
    m_path              = rhs.m_path;
    m_databaseId        = rhs.m_databaseId;
    m_bAutoSave         = rhs.m_bAutoSave;
    m_slot              = rhs.m_slot;
    m_label             = rhs.m_label;
    m_size              = rhs.m_size;
    m_gamePath          = rhs.m_gamePath;
    m_gameClient        = rhs.m_gameClient;
    m_gameCRC           = rhs.m_gameCRC;
    m_playtimeFrames    = rhs.m_playtimeFrames;
    m_playtimeWallClock = rhs.m_playtimeWallClock;
    m_timestamp         = rhs.m_timestamp;
    m_bHasThumbnail     = rhs.m_bHasThumbnail;
  }
  return *this;
}

bool CSavestate::Read(std::vector<uint8_t> &data)
{
  CStdString path(GetPath());
  CLog::Log(LOGDEBUG, "CSavestate: Reading \"%s\"", path.c_str());
  if (path.empty())
    return false;

  CFile file;
  if (file.Open(path) && file.GetLength() > 0)
  {
    data.resize((size_t)file.GetLength());
    file.Read(data.data(), data.size());
    return true;
  }
  else
  {
    CLog::Log(LOGDEBUG, "CSavestate: Can't read \"%s\"", path.c_str());
  }
  return false;
}

bool CSavestate::Write(const std::vector<uint8_t> &data, bool bThumbnail /* = true */)
{
  CStdString path(GetPath());
  m_bHasThumbnail = false;
  CLog::Log(LOGDEBUG, "CSavestate: Writing \"%s\"", path.c_str());
  if (path.empty())
    return false;

  SetTimestamp();

  CFile file;
  if (!file.OpenForWrite(path, true) || file.Write(data.data(), data.size()) != (int)data.size())
  {
    CLog::Log(LOGERROR, "CSavestate: Error writing \"%s\"", path.c_str());
    return false;
  }

  if (bThumbnail)
  {
    // Create thumbnail image
    CStdString strThumb(GetThumbnail());
    float aspectRatio = g_renderManager.GetAspectRatio();

    unsigned int width = SAVESTATE_THUMB_WIDTH;
    unsigned int height = (unsigned int)(SAVESTATE_THUMB_WIDTH / aspectRatio);
    if (height > SAVESTATE_THUMB_WIDTH)
    {
      height = SAVESTATE_THUMB_WIDTH;
      width = (unsigned int)(SAVESTATE_THUMB_WIDTH * aspectRatio);
    }

    CRenderCapture *thumbnail = g_renderManager.AllocRenderCapture();
    if (thumbnail)
    {
      g_renderManager.Capture(thumbnail, width, height, CAPTUREFLAG_IMMEDIATELY);

      // If we are running off-thread (such as auto-saving from the player thread),
      // Capture() will return immediately and queue the capture job, causing this
      // to fail. If off-thread thumb creation is desired, it will have to be
      // handled differently.
      if (thumbnail->GetUserState() == CAPTURESTATE_DONE)
      {
        if (CPicture::CreateThumbnailFromSurface(thumbnail->GetPixels(), width, height, thumbnail->GetWidth() * 4, strThumb))
          m_bHasThumbnail = true;
      }
      else
      {
        if (g_application.IsCurrentThread())
          CLog::Log(LOGERROR, "CSavestate: failed to capture thumbnail in app thread");
        else
          CLog::Log(LOGDEBUG, "CSavestate: failed to capture thumbnail");
      }

      g_renderManager.ReleaseRenderCapture(thumbnail);
    }
  }

  return true;
}

bool CSavestate::Rename(const CStdString &newLabel)
{
  CStdString oldPath(GetPath());
  SetSaveTypeLabel(newLabel);
  CStdString newPath(GetPath());
  CLog::Log(LOGDEBUG, "CSavestate: Renaming \"%s\"", oldPath.c_str());
  CLog::Log(LOGDEBUG, "CSavestate:    to \"%s\"", newPath.c_str());
  if (oldPath.empty() || newPath.empty())
    return false;

  bool success;
  if (!(success = CFile::Rename(oldPath, newPath)))
    CLog::Log(LOGERROR, "CSavestate: Error renaming save state");
  
  if (success && m_bHasThumbnail)
  {
    CStdString oldThumbnail = URIUtils::ReplaceExtension(oldPath, ".png");
    CStdString newThumbnail = URIUtils::ReplaceExtension(newPath, ".png");
    if (CFile::Rename(oldThumbnail, newThumbnail))
      CLog::Log(LOGDEBUG, "CSavestate: Renamed save state thumbnail");
    else
      CLog::Log(LOGDEBUG, "CSavestate: Error renaming save state thumbnail");
  }

  return success;
}

bool CSavestate::Delete()
{
  CStdString path(GetPath());
  CLog::Log(LOGDEBUG, "CSavestate: Deleting \"%s\"", path.c_str());
  if (path.empty())
    return false;

  bool success; // Success is determined by primary file deletion
  if (!(success = CFile::Delete(path)))
    CLog::Log(LOGERROR, "CSavestate: Can't delete \"%s\"", path.c_str());

  CStdString thumbPath(GetThumbnail());
  CLog::Log(LOGDEBUG, "CSavestate: Deleting thumbnail \"%s\"", thumbPath.c_str());
  if (!CFile::Delete(thumbPath))
    CLog::Log(LOGDEBUG, "CSavestate: Can't delete \"%s\"", thumbPath.c_str());

  return success;
}

const CStdString &CSavestate::GetPath() const
{
  if (m_path.empty() && !m_gameClient.empty() && !m_gameCRC.empty())
  {
    CStdString hash;
    if (m_bAutoSave)
      hash.Format("%s/%s", m_gameClient.c_str(), m_gameCRC.c_str());
    else if (m_slot)
      hash.Format("%s/%s_%u", m_gameClient.c_str(), m_gameCRC.c_str(), m_slot);
    else
    {
      Crc32 crc;
      crc.Compute(m_label);
      hash.Format("%s/%s_%08x", m_gameClient.c_str(), m_gameCRC.c_str(), (unsigned __int32)crc);
    }
    hash += SAVESTATE_EXTENSION;
    URIUtils::AddFileToFolder(CProfilesManager::Get().GetSavegamesFolder(), hash, m_path);
  }
  return m_path;
}

void CSavestate::SetPath(const CStdString &path)
{
  Reset();

  if (!URIUtils::GetExtension(path).Equals(SAVESTATE_EXTENSION))
    return;

  // Analyze the save state name to determine the type
  CStdString saveName = URIUtils::GetFileName(path);
  if (saveName.size() == 8 + strlen(SAVESTATE_EXTENSION))
  {
    // Auto-save (file name is like feba62c2.sav)
    SetSaveTypeAuto();
  }
  else if (saveName.size() == 8 + 2 + strlen(SAVESTATE_EXTENSION) && '1' <= saveName[9] && saveName[9] <= '9')
  {
    // Save type slot (file name is like feba62c2_1.sav)
    unsigned int slot = saveName[9] - '0';
    SetSaveTypeSlot(slot);
  }
  else if (saveName.size() == 8 + 1 + 8 + strlen(SAVESTATE_EXTENSION))
  {
    // Save type label (file name is like feba62c2_8dc22669.sav)
    SetSaveTypeLabel(""); // Unknown label for now
  }
  else
  {
    return; // Invalid
  }

  // Game CRC is first 8 characters of save state file name
  m_gameCRC = saveName.substr(0, 8);

  // Game client ID is parent folder
  CStdString gameclientId = URIUtils::GetParentPath(path);
  URIUtils::RemoveSlashAtEnd(gameclientId);
  m_gameClient = URIUtils::GetFileName(gameclientId);

  m_path = path;
}

CStdString CSavestate::GetThumbnail() const
{
  CStdString path(GetPath());
  if (path.length())
    path = URIUtils::ReplaceExtension(path, ".png");
  return path;
}

void CSavestate::SetSaveTypeAuto()
{
  if (GetSaveType() != SAVETYPE_AUTO)
    m_path.clear();

  m_bAutoSave = true;
  m_slot = 0;
  // Autosave on %s
  m_label.Format(g_localizeStrings.Get(15042).c_str(), CDateTime::GetCurrentDateTime().GetAsLocalizedDateTime().c_str());
}

void CSavestate::SetSaveTypeSlot(unsigned int slot)
{
  if (GetSaveType() != SAVETYPE_SLOT || m_slot != slot)
    m_path.clear();

  m_bAutoSave = false;
  m_slot = slot;
  // Slot %d
  m_label.Format(g_localizeStrings.Get(15043).c_str(), slot);
}

void CSavestate::SetSaveTypeLabel(const CStdString &label)
{
  m_path.clear();

  m_bAutoSave = false;
  m_slot = 0;
  m_label = label;
}

void CSavestate::SetGameCRCFromFile(const CStdString &filename, bool forceFilename /* = false */)
{
  std::vector<char> buffer;
  CFile file;
  int64_t length;
  if (file.Open(filename) && (length = file.GetLength()) > 0)
  {
    if (length > MAX_SAVESTATE_CRC_LENGTH || forceFilename)
    {
      Crc32 crc;
      crc.ComputeFromLowerCase(filename);
      m_gameCRC.Format("%08x", (unsigned __int32)crc);
    }
    else
    {
      buffer.resize((size_t)length);
      file.Read(buffer.data(), length);
      SetGameCRCFromFile(buffer.data(), buffer.size());
    }
  }
}

void CSavestate::SetGameCRCFromFile(const char *data, size_t length)
{
  Crc32 crc;
  crc.Compute(data, length);
  m_gameCRC.Format("%08x", (unsigned __int32)crc);
}

void CSavestate::SetTimestamp(const CStdString &strTimestamp /* = empty */)
{
  if (!strTimestamp.empty())
    m_timestamp = strTimestamp;
  else
    m_timestamp = CDateTime::GetCurrentDateTime().GetAsDBDateTime();
}

void CSavestate::Serialize(CVariant& value) const
{
  value["path"]              = GetPath();
  value["databaseid"]        = m_databaseId;
  value["autosave"]          = m_bAutoSave;
  value["slot"]              = m_slot;
  value["label"]             = m_label;
  value["size"]              = m_size;
  value["gamepath"]          = m_gamePath;
  value["gameclient"]        = m_gameClient;
  value["gamecrc"]           = m_gameCRC;
  value["playtimeframes"]    = m_playtimeFrames;
  value["playtimewallclock"] = m_playtimeWallClock;
  value["timestamp"]         = m_timestamp;
  value["hasthumbnail"]      = m_bHasThumbnail;
}

void CSavestate::Deserialize(const CVariant& value)
{
  m_path              = value["path"].asString();
  m_databaseId        = (int)value["databaseid"].asInteger();
  m_bAutoSave         = value["autosave"].asBoolean();
  m_slot              = (unsigned int)value["slot"].asUnsignedInteger();
  m_label             = value["label"].asString();
  m_size              = (size_t)value["size"].asUnsignedInteger();
  m_gamePath          = value["gamepath"].asString();
  m_gameClient        = value["gameclient"].asString();
  m_gameCRC           = value["gamecrc"].asString();
  m_playtimeFrames    = value["playtimeframes"].asUnsignedInteger();
  m_playtimeWallClock = value["playtimewallclock"].asDouble();
  m_timestamp         = value["timestamp"].asString();
  m_bHasThumbnail     = value["hasthumbnail"].asBoolean();
}
