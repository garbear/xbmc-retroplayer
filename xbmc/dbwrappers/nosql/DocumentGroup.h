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
#pragma once

#include "threads/CriticalSection.h"

#include <string>
#include <vector>

class CVariant;

namespace dbiplus
{
  class CDenormalizedDatabase;
  class IDocument;

  enum RelationshipType
  {
    ONE_TO_ONE,
    ONE_TO_MANY,
    MANY_TO_MANY
  };

  class CDocumentGroup
  {
  public:
    CDocumentGroup(CDenormalizedDatabase* db, const std::string& strName, RelationshipType type);
    ~CDocumentGroup(void) { }

    const std::string& Name(void) const { return m_strPropName; }
    RelationshipType Type(void) const { return m_type; }

    /*!
     * \brief Retrieve a single document
     */
    IDocument* GetDocument(const CVariant& value) const;

    /*!
     * \brief Retrieve a set of documents
     */
    void GetDocuments(const CVariant& value, std::vector<IDocument*>& documents) const;

  private:
    CDenormalizedDatabase* const m_db;
    const std::string            m_strPropName;
    const RelationshipType       m_type;
    CCriticalSection             m_mutex;
  };
}
