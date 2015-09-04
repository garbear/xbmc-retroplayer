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

#include <string>

class CVariant;
class IDocumentIterator;

#define DOCUMENT_ID  "_id"

namespace dbiplus
{
  class IDocumentStore
  {
  public:
    virtual ~IDocumentStore(void) { }

    virtual bool Connect(void) = 0;
    virtual void Disconnect(void) = 0;

    /*!
     * \brief Insert a document
     *
     * If the document does not contain the _id field, the field is
     * automatically set to a generated ObjectId.
     */
    virtual bool Put(const std::string& strCollection, const CVariant& document) = 0;

    /*!
     * \brief Select a document
     */
    virtual bool Get(const std::string& strCollection, const CVariant& query, CVariant& document) = 0;

    /*!
     * \brief Select multiple documents
     *
     * TODO: Document query syntax ($contains, $is, $isnot, $lt, $gt, etc.)
     */
    virtual IDocumentIterator* Find(const std::string& strCollection, const CVariant& query, unsigned int limit, const std::string& orderBy) = 0;

    /*!
     * \brief Remove a document
     */
    virtual bool Delete(const std::string& strCollection, const CVariant& query) = 0;
  };
}
