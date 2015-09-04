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

#include "GenericDocumentStore.h"
#include "IKeyValueStore.h"
#include "utils/BSONVariantParser.h"
#include "utils/BSONVariantWriter.h"

#include <assert.h>

using namespace dbiplus;

#define DOCUMENT_COLLECTION  "_c"

CGenericDocumentStore::CGenericDocumentStore(IKeyValueStore* datastore) :
  m_datastore(datastore)
{
  assert(m_datastore);
}

CGenericDocumentStore::~CGenericDocumentStore(void)
{
  Disconnect();
  delete m_datastore;
}

bool CGenericDocumentStore::Connect(void)
{
  return m_datastore->Connect();
}

void CGenericDocumentStore::Disconnect(void)
{
  m_datastore->Disconnect();
}

bool CGenericDocumentStore::Put(const std::string& strCollection, const CVariant& document)
{
  // TODO
  CVariant key;
  key[DOCUMENT_COLLECTION] = strCollection;
  key[DOCUMENT_ID] = 0; // TODO
  std::string strKey = CBSONVariantWriter::Write(key);
  std::string strValue = CBSONVariantWriter::Write(document);

  return m_datastore->Put(strKey, strValue);
}

bool CGenericDocumentStore::Get(const std::string& strCollection, const CVariant& query, CVariant& document)
{
  std::string strKey = CBSONVariantWriter::Write(query); // TODO

  std::string strValue;
  if (m_datastore->Get(strKey, strValue))
  {
    document = CBSONVariantParser::Parse(strValue.data(), strValue.size());
    return true;
  }

  return false;
}

IDocumentIterator* CGenericDocumentStore::Find(const std::string& strCollection, const CVariant& query, unsigned int limit, const std::string& orderBy)
{
  return NULL; // TODO
}

bool CGenericDocumentStore::Delete(const std::string& strCollection, const CVariant& query)
{
  return m_datastore->Delete(CBSONVariantWriter::Write(query)); // TODO
}
