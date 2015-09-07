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

#include "IDocument.h"
#include "threads/CriticalSection.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class CVariant;

namespace dbiplus
{
  //typedef uint64_t dbkey_t; // TODO: Move to types?

  class CDocumentGroup;
  class IDocumentStore;

  class CDenormalizedDatabase
  {
  public:
    CDenormalizedDatabase(void);
    virtual ~CDenormalizedDatabase(void);

    virtual const char* GetBaseDBName(void) const = 0;

    virtual IDocument* CreateDocument(void) const = 0;

    bool Connect(void);
    bool IsConnected(void) const;
    void Disconnect(void);

    //bool CreateSnapshot(void);
    //bool WriteBatch(void);
    //bool RangeIteration(void);

    // CustomComparators
    // Sync/Async

  protected:
    void AddOneToOne(const std::string& strProperty);
    void AddOneToMany(const std::string& strProperty);
    void AddManyToMany(const std::string& strProperty);

    IDocument* GetDocument(const std::string& strProperty, const CVariant& value) const;
    std::vector<IDocument*> GetDocuments(void) const;
    std::vector<IDocument*> GetDocuments(const std::string& strProperty, const CVariant& value) const;

    //void Rebuild(void); // TODO

  private:
    friend class CDocumentGroup;

    bool Put(const CVariant& key, const CVariant& value);
    bool Get(const CVariant& key, CVariant& value) const;
    bool Delete(const CVariant& key);

    std::string GetDatabasePath(void) const;

    std::unique_ptr<dbiplus::IDocumentStore> m_pDS;
    std::map<std::string, CDocumentGroup*>   m_relationships;
    unsigned int                             m_openCount;
    CCriticalSection                         m_openMutex;
  };
}
