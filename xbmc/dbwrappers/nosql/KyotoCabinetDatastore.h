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

#include "IKeyValueStore.h"

#include <string>

namespace kyotocabinet
{
  class PolyDB;
}

namespace dbiplus
{
  class CKyotoCabinetDatastore : public IKeyValueStore
  {
  public:
    CKyotoCabinetDatastore(const std::string& strPath);
    virtual ~CKyotoCabinetDatastore(void) { Disconnect(); }

    // implementation of IKeyValueStore
    virtual bool Connect(void);
    virtual void Disconnect(void);
    virtual bool Put(const std::string& key, const std::string& value);
    virtual bool Get(const std::string& key, std::string& value);
    virtual bool Delete(const std::string& key);

  private:
    const std::string     m_strPath;
    kyotocabinet::PolyDB* m_db;
  };
}
