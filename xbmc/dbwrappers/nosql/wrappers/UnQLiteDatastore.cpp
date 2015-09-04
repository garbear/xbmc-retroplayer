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

#include "UnQLiteDatastore.h"

#include <unqlite.h>

using namespace dbiplus;

CUnQLiteDatastore::CUnQLiteDatastore(const std::string& strPath) :
  m_strPath(strPath),
  m_db(NULL)
{
}

bool CUnQLiteDatastore::Connect(void)
{
  Disconnect();

  return false; // TODO
}

void CUnQLiteDatastore::Disconnect(void)
{
  //delete m_db;
  //m_db = NULL;
}

bool CUnQLiteDatastore::Put(const std::string& strCollection, const CVariant& document)
{
  if (!m_db)
    return false;

  return false; // TODO
}

bool CUnQLiteDatastore::Get(const std::string& strCollection, const CVariant& query, CVariant& document)
{
  if (!m_db)
    return false;

  return false; // TODO
}

IDocumentIterator* CUnQLiteDatastore::Find(const std::string& strCollection, const CVariant& query, unsigned int limit, const std::string& orderBy)
{
  if (!m_db)
    return NULL;

  return NULL; // TODO
}

bool CUnQLiteDatastore::Delete(const std::string& strCollection, const CVariant& query)
{
  if (!m_db)
    return false;

  return false; // TODO
}
