/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
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
#include "addons/ContentAddons.h"
#include "threads/SystemClock.h"
#include "MusicSearchDirectory.h"
#include "music/MusicDatabase.h"
#include "URL.h"
#include "FileItem.h"
#include "utils/log.h"
#include "utils/URIUtils.h" // for MUSICSEARCH_TARGET_* defines
#include "guilib/LocalizeStrings.h"

using namespace ADDON;
using namespace XFILE;

CMusicSearchDirectory::CMusicSearchDirectory(void)
{
}

CMusicSearchDirectory::~CMusicSearchDirectory(void)
{
}

bool CMusicSearchDirectory::GetDirectory(const CURL& url, CFileItemList &items)
{
  // break up our path
  // format is: musicsearch://<target>/<search string>
  // target is: MUSICSEARCH_TARGET_ALL, MUSICSEARCH_TARGET_LIBRARY or content add-on ID
  std::string target(url.GetHostName());
  std::string search(url.GetHostName());

  if (search.empty())
    return false;

  /* TODO
   CONTENT_ADDON addon;
  if (target != MUSICSEARCH_TARGET_ALL && target != MUSICSEARCH_TARGET_LIBRARY &&
      !(addon = CContentAddons::Get().GetAddonByID(target)))
    return false;
  */

  // and retrieve the search details
  items.SetURL(url);
  unsigned int time = XbmcThreads::SystemClockMillis();
  /* TODO
  if (target == MUSICSEARCH_TARGET_ALL)
    CContentAddons::Get().MusicSearch(items, search);
  else if (target != MUSICSEARCH_TARGET_LIBRARY)
  {
    // target is a content add-on ID
    if (addon->ProvidesMusicFiles())
      addon->MusicSearch(items, search);
  }
  if (target == MUSICSEARCH_TARGET_ALL || target == MUSICSEARCH_TARGET_LIBRARY)
  {
    CMusicDatabase db;
    if (db.Open())
    {
      db.Search(search, items);
      db.Close();
    }
   }
  */
  CLog::Log(LOGDEBUG, "%s (%s) took %u ms",
            __FUNCTION__, url.GetRedacted().c_str(), XbmcThreads::SystemClockMillis() - time);
  items.SetLabel(g_localizeStrings.Get(137)); // Search
  return true;
}

bool CMusicSearchDirectory::Exists(const CURL& url)
{
  return true;
}
