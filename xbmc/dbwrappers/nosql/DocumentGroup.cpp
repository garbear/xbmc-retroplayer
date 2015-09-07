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
#include "IDocument.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "utils/Variant.h"

#include <assert.h>
#include "DocumentGroup.h"

using namespace dbiplus;

#define DOCUMENT_ID   "id"

CDocumentGroup::CDocumentGroup(CDenormalizedDatabase* db, const std::string& strName, RelationshipType type) :
  m_db(db),
  m_strPropName(strName),
  m_type(type)
{
  assert(m_db);
}

IDocument* CDocumentGroup::GetDocument(const CVariant& value) const
{
  CSingleLock lock(m_mutex);

  IDocument* document = NULL;

  if (m_type != ONE_TO_ONE)
  {
    CLog::Log(LOGERROR, "Can't get property \"%s\": not a one-to-one relationship", m_strPropName.c_str());
  }
  else
  {
    CVariant propertyKey;
    propertyKey[m_strPropName] = value;

    CVariant documentId;
    if (!m_db->Get(propertyKey, documentId))
    {
      CLog::Log(LOGERROR, "Can't find document by property \"%s\"", m_strPropName.c_str());
    }
    else
    {
      CVariant documentKey;
      documentKey[DOCUMENT_ID] = documentId;

      CVariant documentObject;
      if (!m_db->Get(documentKey, documentObject))
      {
        if (documentId.isInteger())
          CLog::Log(LOGERROR, "Can't find document by ID: %d", (int)documentId.asInteger());
      }
      else
      {
        document = m_db->CreateDocument();
        document->Deserialize(documentObject);
      }
    }
  }

  return document;
}

void CDocumentGroup::GetDocuments(const CVariant& value, std::vector<IDocument*>& documents) const
{
  CSingleLock lock(m_mutex);

  if (Type() != ONE_TO_MANY && Type() != MANY_TO_MANY)
  {
    CLog::Log(LOGERROR, "Can't get property \"%s\": not a one-to-many or many-to-many relationship", m_strPropName.c_str());
  }
  else
  {
    CVariant propertyKey;
    propertyKey[m_strPropName] = value;

    CVariant documentIds;
    if (!m_db->Get(propertyKey, documentIds))
    {
      CLog::Log(LOGERROR, "Can't find documents by property \"%s\"", m_strPropName.c_str());
    }
    else
    {
      if (documentIds.empty())
      {
        CLog::Log(LOGERROR, "Can't find documents: property \"%s\" is empty", m_strPropName.c_str());
      }
      else
      {
        documents.reserve(documentIds.size());

        for (CVariant::const_iterator_array itDocumentId = documentIds.begin_array();
            itDocumentId != documentIds.end_array(); ++itDocumentId)
        {
          CVariant documentKey;
          documentKey[DOCUMENT_ID] = *itDocumentId;

          CVariant documentObject;
          if (!m_db->Get(documentKey, documentObject))
          {
            if (itDocumentId->isInteger())
              CLog::Log(LOGERROR, "Can't find document by ID: %d", (int)itDocumentId->asInteger());
          }
          else
          {
            IDocument* document = m_db->CreateDocument();
            document->Deserialize(documentObject);
            documents.push_back(document);
          }
        }
      }
    }
  }
}
