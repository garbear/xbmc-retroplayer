#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
 *      http://www.xbmc.org
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

#include "IFile.h"
#include <memory>

namespace ADDON { class CContentAddon; }

namespace XFILE {

class CContentAddonFile : public IFile
{
public:
  CContentAddonFile(void);
  virtual ~CContentAddonFile(void);

  bool Open(const CURL& url);
  bool Exists(const CURL& url);
  int Stat(const CURL& url, struct __stat64* buffer);
  ssize_t Read(void* bufPtr, size_t bufSize);
  int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
  void Close(void);
  int64_t GetPosition(void);
  int64_t GetLength(void);

  std::string GetContent() { return "application/octet-stream"; }

private:
  std::shared_ptr<ADDON::CContentAddon> m_addon;
  void*                                 m_handle;
  bool                                  m_bOpen;
};

}


