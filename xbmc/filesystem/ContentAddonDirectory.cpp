/*
 *      Copyright (C) 2013 Team XBMC
 *      http://www.xbmc.org
 *
 * This Program is free software; you can redistribute it and/or modify
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

#include "ContentAddonDirectory.h"
#include "addons/ContentAddons.h"
#include "URL.h"

using namespace XFILE;
using namespace ADDON;

bool CContentAddonDirectory::Exists(const char* strPath)
{
  CONTENT_ADDON addon = CContentAddons::Get().GetAddonForPath(strPath);
  if (addon.get())
    return addon->FileExists(strPath);
  return false;
}

bool CContentAddonDirectory::GetDirectory(const CURL& url, CFileItemList &items)
{
  std::string strPath(url.Get());
  CONTENT_ADDON addon = CContentAddons::Get().GetAddonForPath(strPath);
  if (addon.get())
    return addon->FileGetDirectory(items, strPath);
  return false;
}
