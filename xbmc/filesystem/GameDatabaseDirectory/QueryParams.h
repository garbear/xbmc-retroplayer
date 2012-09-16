#pragma once
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

#include "DirectoryNode.h"

namespace XFILE
{
  namespace GAMEDATABASEDIRECTORY
  {
    class CQueryParams
    {
    public:
      CQueryParams();
      long GetGameId() const { return m_idGame; }
      long GetGenreId() const { return m_idGenre; }
      long GetCollectionId() const { return m_idCollection; }
      long GetPlatformId() const { return m_idPlatform; }
      long GetPublisherId() const { return m_idPublisher; }
      long GetYear() const { return m_year; }

    protected:
      void SetQueryParam(NODE_TYPE NodeType, const CStdString& strNodeName);

      friend class CDirectoryNode;

    private:
      long m_idGame;
      long m_idGenre;
      long m_idCollection;
      long m_idPlatform;
      long m_idPublisher;
      long m_year;
    };
  }
}
