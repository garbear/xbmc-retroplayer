/*
 *      Copyright (C) 2015 Team Kodi
 *      http://kodi.tv
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

#include "KyotoCabinetDatastore.h"
#include "utils/log.h"

#include <kcpolydb.h>

using namespace dbiplus;

CKyotoCabinetDatastore::CKyotoCabinetDatastore(const std::string& strPath) :
  m_strPath(strPath),
  m_db(NULL)
{
}

bool CKyotoCabinetDatastore::Connect(void)
{
  Disconnect();

  m_db = new kyotocabinet::PolyDB;
  if (!m_db->open(m_strPath, kyotocabinet::PolyDB::OWRITER | kyotocabinet::PolyDB::OCREATE))
  {
    CLog::Log(LOGERROR, "Error connecting to Kyoto Cabinet db: %s", m_db->error().name());
    delete m_db;
    m_db = NULL;
    return false;
  }

  return true;
}

void CKyotoCabinetDatastore::Disconnect(void)
{
  if (m_db)
  {
    if (!m_db->close())
      CLog::Log(LOGERROR, "Error closing Kyoto Cabinet db: %s", m_db->error().name());
    delete m_db;
    m_db = NULL;
  }
}

bool CKyotoCabinetDatastore::Put(const std::string& key, const std::string& value)
{
  if (!m_db)
    return false;

  if (!m_db->set(key, value))
  {
    CLog::Log(LOGERROR, "Error setting Kyoto Cabinet key: %s", m_db->error().name());
    return false;
  }

  return true;
}

bool CKyotoCabinetDatastore::Get(const std::string& key, std::string& value)
{
  if (!m_db)
    return false;
  
  if (!m_db->get(key, &value))
  {
    CLog::Log(LOGERROR, "Error getting Kyoto Cabinet key: %s", m_db->error().name());
    return false;
  }

  return true;
}

bool CKyotoCabinetDatastore::Delete(const std::string& key)
{
  if (!m_db)
    return false;
  
  if (!m_db->remove(key))
  {
    CLog::Log(LOGERROR, "Error deleting Kyoto Cabinet key: %s", m_db->error().name());
    return false;
  }

  return true;
}
