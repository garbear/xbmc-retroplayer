/*
 *      Copyright (C) 2012 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include "../../../addons/library.xbmc.content/libXBMC_content.h"
#include "addons/AddonCallbacks.h"

#ifdef _WIN32
#include <windows.h>
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

using namespace std;

extern "C"
{

DLLEXPORT void* CONTENT_register_me(void *hdl)
{
  CB_ContentLib *cb = NULL;
  if (!hdl)
    fprintf(stderr, "ERROR: libXBMC_content::CONTENT_register_me is called with NULL handle\n");
  else
  {
    cb = ((AddonCB*)hdl)->ContentLib_RegisterMe(((AddonCB*)hdl)->addonData);
    if (!cb)
      fprintf(stderr, "ERROR: libXBMC_content::CONTENT_register_me can't get callback table from XBMC\n");
  }
  return cb;
}

DLLEXPORT void CONTENT_unregister_me(void *hdl, void* cb)
{
  if (hdl && cb)
    ((AddonCB*)hdl)->ContentLib_UnRegisterMe(((AddonCB*)hdl)->addonData, (CB_ContentLib*)cb);
}

DLLEXPORT void CONTENT_set_playstate(void *hdl, void* cb, CONTENT_ADDON_PLAYSTATE newState)
{
  if (cb == NULL)
    return;

  ((CB_ContentLib*)cb)->SetPlaystate(((AddonCB*)hdl)->addonData, newState);
}

};
