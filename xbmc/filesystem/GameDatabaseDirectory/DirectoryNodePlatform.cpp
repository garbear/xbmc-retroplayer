/*
 *      Copyright (C) 2005-2012 Team XBMC
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

#include "DirectoryNodePlatform.h"
#include "games/GameDatabase.h"

using namespace XFILE::GAMEDATABASEDIRECTORY;

CDirectoryNodePlatform::CDirectoryNodePlatform(const CStdString& strName, CDirectoryNode* pParent)
  : CDirectoryNode(NODE_TYPE_PLATFORM, strName, pParent)
{
}

NODE_TYPE CDirectoryNodePlatform::GetChildType() const
{
  return NODE_TYPE_GAME;
}

CStdString CDirectoryNodePlatform::GetLocalizedName() const
{
  CGameDatabase db;
  if (db.Open())
    return db.GetPlatformById(GetID());
  return "";
}

bool CDirectoryNodePlatform::GetContent(CFileItemList& items) const
{
  CGameDatabase db;
  if (db.Open())
    return db.GetPlatformsNav(BuildPath(), items);
  return false;
}
