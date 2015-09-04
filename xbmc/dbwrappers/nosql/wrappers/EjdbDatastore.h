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

#include "dbwrappers/nosql/IDocumentStore.h"

#include <string>

struct EJDB;

namespace dbiplus
{
  class CEjdbDatastore : public IDocumentStore
  {
  public:
    CEjdbDatastore(const std::string& strPath);
    virtual ~CEjdbDatastore(void) { Disconnect(); }

    // implementation of IDocumentStore
    virtual bool Connect(void);
    virtual void Disconnect(void);
    virtual bool Put(const std::string& strCollection, const CVariant& document);
    virtual bool Get(const std::string& strCollection, const CVariant& query, CVariant& document);
    virtual IDocumentIterator* Find(const std::string& strCollection, const CVariant& query, unsigned int limit, const std::string& orderBy);
    virtual bool Delete(const std::string& strCollection, const CVariant& query);

  private:
    const std::string m_strPath;
    EJDB*             m_db;
  };
}
