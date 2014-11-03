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

#include "ContentAddonFile.h"
#include "addons/ContentAddons.h"
#include "utils/log.h"
#include "URL.h"

using namespace std;
using namespace XFILE;
using namespace ADDON;

CContentAddonFile::CContentAddonFile(void) :
    m_handle(NULL),
    m_bOpen(false)
{
}

CContentAddonFile::~CContentAddonFile(void)
{
  Close();
}

bool CContentAddonFile::Open(const CURL& url)
{
  //m_addon = CContentAddons::Get().GetAddonForPath(url.Get()); // TODO
  if (m_addon.get())
  {
    m_bOpen = m_addon->FileOpen(url.Get(), &m_handle);
    return m_bOpen;
  }
  CLog::Log(LOGERROR, "%s(%s) failed to find add-on", __FUNCTION__, url.Get().c_str());
  return false;
}

bool CContentAddonFile::Exists(const CURL& url)
{
  CONTENT_ADDON addon = CContentAddons::Get().GetAddonForPath(url.Get());
  if (addon.get())
    return addon->FileExists(url.Get());
  return false;
}

int CContentAddonFile::Stat(const CURL& url, struct __stat64* buffer)
{
  if (buffer)
  {
    memset(buffer, 0, sizeof(struct __stat64));
    CONTENT_ADDON addon = CContentAddons::Get().GetAddonForPath(url.Get());
    if (addon && addon->FileStat(url.Get(), buffer) == 0)
      return 0;
  }

  errno = ENOENT;
  return -1;
}

void CContentAddonFile::Close(void)
{
  if (m_bOpen && m_addon.get())
  {
    m_addon->FileClose(m_handle);
    m_bOpen  = false;
    m_handle = NULL;
  }
}

ssize_t CContentAddonFile::Read(void* buffer, size_t size)
{
  if (m_bOpen && m_addon.get())
    return m_addon->FileRead(m_handle, buffer, size);
  return 0;
}

int64_t CContentAddonFile::GetLength(void)
{
  if (m_bOpen && m_addon.get())
    return m_addon->FileGetLength(m_handle);
  return 0;
}

int64_t CContentAddonFile::Seek(int64_t pos, int whence)
{
  if (m_bOpen && m_addon.get())
    return m_addon->FileSeek(m_handle, pos, whence);
  return 0;
}

int64_t CContentAddonFile::GetPosition(void)
{
  if (m_bOpen && m_addon.get())
    return m_addon->FileGetPosition(m_handle);
  return 0;
}
