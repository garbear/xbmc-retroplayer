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

#include "MongoDbDatastore.h"
#include "utils/log.h"

#include <mongo/client/dbclient.h>

using namespace dbiplus;

#define MONGODB_ADDRESS  "localhost" // TODO

CMongoDbDatastore::CMongoDbDatastore(const std::string& strPath) :
  m_strPath(strPath),
  m_db(NULL)
{
}

bool CMongoDbDatastore::Connect(void)
{
  Disconnect();
  mongo::Status status = mongo::client::initialize();
  if (!status.isOK())
  {
    CLog::Log(LOGERROR, "MongoDB: Failed to initailize client");
    return false;
  }

  m_db = new mongo::DBClientConnection;
  try
  {
    m_db->connect(MONGODB_ADDRESS);
  }
  catch (const mongo::DBException& e)
  {
    CLog::Log(LOGERROR, "MongoDB: %s", e.what());
    delete m_db;
    m_db = NULL;
    return false;
  }
  
  CLog::Log(LOGDEBUG, "MongoDB: Connected to %s", MONGODB_ADDRESS);
  return true;
}

void CMongoDbDatastore::Disconnect(void)
{
  delete m_db;
  m_db = NULL;
}

bool CMongoDbDatastore::Put(const std::string& key, const std::string& value)
{
  if (!m_db)
    return false;

  return false; // TODO
}

bool CMongoDbDatastore::Get(const std::string& key, std::string& value)
{
  if (!m_db)
    return false;
  
  return false; // TODO
}

bool CMongoDbDatastore::Delete(const std::string& key)
{
  if (!m_db)
    return false;
  
  return false; // TODO
}
