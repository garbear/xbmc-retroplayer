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

#include "DenormalizedDatabase.h"
#include "GenericDocumentStore.h"
#include "KyotoCabinetDatastore.h"
#include "LevelDbDatastore.h"
#include "DatabaseManager.h"
#include "DocumentGroup.h"
#include "filesystem/SpecialProtocol.h"
#include "profiles/ProfilesManager.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"

using namespace dbiplus;

#define ALL_DOCUMENTS  "all"

CDenormalizedDatabase::CDenormalizedDatabase(void) :
  m_openCount(0)
{
  // Allows us to query all documents
  AddOneToMany(ALL_DOCUMENTS);
}

CDenormalizedDatabase::~CDenormalizedDatabase(void)
{
  for (auto it = m_relationships.begin(); it != m_relationships.end(); ++it)
    delete it->second;
}

bool CDenormalizedDatabase::Connect(void)
{
  // check our database manager to see if this database can be opened
  if (!CDatabaseManager::Get().CanOpen(GetBaseDBName()))
    return false;

  CSingleLock lock(m_openMutex);
  if (m_openCount == 0)
  {
#if defined(TARGET_WINDOWS)
    //m_pDS.reset(new CGenericDocumentStore(new CKyotoCabinetDatastore(GetDatabasePath()))); // TODO
#else
    m_pDS.reset(new CGenericDocumentStore(new CLevelDbDatastore(GetDatabasePath())));
#endif

    if (!m_pDS)
      return false;

    if (!m_pDS->Connect())
    {
      m_pDS.reset();
      return false;
    }
  }

  m_openCount++;

  return true;
}

bool CDenormalizedDatabase::IsConnected(void) const
{
  CSingleLock lock(m_openMutex);
  return m_openCount > 0;
}

void CDenormalizedDatabase::Disconnect(void)
{
  CSingleLock lock(m_openMutex);
  if (m_openCount > 0 && --m_openCount == 0)
  {
    m_pDS->Disconnect();
    m_pDS.reset();
  }
}

IDocument* CDenormalizedDatabase::GetDocument(const std::string& strProperty, const CVariant& value) const
{
  IDocument* document = NULL;

  auto itRelationship = m_relationships.find(strProperty);

  const bool bHasOneToOne = (itRelationship != m_relationships.end());

  if (!bHasOneToOne)
    CLog::Log(LOGERROR, "No relationship for property \"%s\"", strProperty.c_str());
  else
    document = itRelationship->second->GetDocument(value);

  return document;
}

std::vector<IDocument*> CDenormalizedDatabase::GetDocuments(void) const
{
  return GetDocuments(ALL_DOCUMENTS, CVariant(true));
}

std::vector<IDocument*> CDenormalizedDatabase::GetDocuments(const std::string& strProperty, const CVariant& value) const
{
  std::vector<IDocument*> documents;

  auto itRelationship = m_relationships.find(strProperty);

  if (itRelationship == m_relationships.end())
    CLog::Log(LOGERROR, "No relationship for property \"%s\"", strProperty.c_str());
  else
    itRelationship->second->GetDocuments(value, documents);

  return documents;
}

void CDenormalizedDatabase::AddOneToOne(const std::string& strProperty)
{
  m_relationships[strProperty] = new CDocumentGroup(this, strProperty, ONE_TO_ONE);
}

void CDenormalizedDatabase::AddOneToMany(const std::string& strProperty)
{
  m_relationships[strProperty] = new CDocumentGroup(this, strProperty, ONE_TO_MANY);
}

void CDenormalizedDatabase::AddManyToMany(const std::string& strProperty)
{
  m_relationships[strProperty] = new CDocumentGroup(this, strProperty, MANY_TO_MANY);
}

bool CDenormalizedDatabase::Put(const CVariant& key, const CVariant& value)
{
  if (!m_pDS)
    return false;

  return m_pDS->Put(key, value);
}

bool CDenormalizedDatabase::Get(const CVariant& key, CVariant& value) const
{
  if (!m_pDS)
    return false;

  return m_pDS->Get(key, value);
}

bool CDenormalizedDatabase::Delete(const CVariant& key)
{
  if (!m_pDS)
    return false;

  return m_pDS->Delete(key);
}

std::string CDenormalizedDatabase::GetDatabasePath(void) const
{
  std::string strDir = CSpecialProtocol::TranslatePath(CProfilesManager::Get().GetDatabaseFolder());
  return URIUtils::AddFileToFolder(strDir, GetBaseDBName()) + ".db";
}
