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

#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "addons/include/xbmc_content_types.h"
#include "../library.xbmc.addon/libXBMC_addon.h"

#ifdef _WIN32
#define CONTENT_HELPER_DLL "\\library.xbmc.content\\libXBMC_content" ADDON_HELPER_EXT
#else
#define CONTENT_HELPER_DLL_NAME "libXBMC_content-" ADDON_HELPER_ARCH ADDON_HELPER_EXT
#define CONTENT_HELPER_DLL "/library.xbmc.content/" CONTENT_HELPER_DLL_NAME
#endif

class CHelper_libXBMC_content
{
public:
  CHelper_libXBMC_content(void)
  {
    m_libXBMC_content = NULL;
    m_Handle          = NULL;
  }

  ~CHelper_libXBMC_content(void)
  {
    if (m_libXBMC_content)
    {
      CONTENT_unregister_me(m_Handle, m_Callbacks);
      dlclose(m_libXBMC_content);
    }
  }

  /*!
   * @brief Resolve all callback methods
   * @param handle Pointer to the add-on
   * @return True when all methods were resolved, false otherwise.
   */
  bool RegisterMe(void* handle)
  {
    m_Handle = handle;

    std::string libBasePath;
    libBasePath  = ((cb_array*)m_Handle)->libPath;
    libBasePath += CONTENT_HELPER_DLL;

#if defined(ANDROID)
      struct stat st;
      if(stat(libBasePath.c_str(),&st) != 0)
      {
        std::string tempbin = getenv("XBMC_ANDROID_LIBS");
        libBasePath = tempbin + "/" + CONTENT_HELPER_DLL_NAME;
      }
#endif

    m_libXBMC_content = dlopen(libBasePath.c_str(), RTLD_LAZY);
    if (m_libXBMC_content == NULL)
    {
      fprintf(stderr, "Unable to load %s\n", dlerror());
      return false;
    }

    CONTENT_register_me = (void* (*)(void *HANDLE))
      dlsym(m_libXBMC_content, "CONTENT_register_me");
    if (CONTENT_register_me == NULL) { fprintf(stderr, "Unable to assign function %s\n", dlerror()); return false; }

    CONTENT_unregister_me = (void (*)(void* HANDLE, void* CB))
      dlsym(m_libXBMC_content, "CONTENT_unregister_me");
    if (CONTENT_unregister_me == NULL) { fprintf(stderr, "Unable to assign function %s\n", dlerror()); return false; }

    CONTENT_set_playstate = (void (*)(void* HANDLE, void* CB, CONTENT_ADDON_PLAYSTATE newState))
      dlsym(m_libXBMC_content, "CONTENT_set_playstate");
    if (CONTENT_set_playstate == NULL) { fprintf(stderr, "Unable to assign function %s\n", dlerror()); return false; }

    m_Callbacks = CONTENT_register_me(m_Handle);
    return m_Callbacks != NULL;
  }

  /*!
   * Change the playstate of this add-on.
   * This will change the playstate of the player in XBMC when this add-on is being used for playback.
   * @param newState The new playstate
   */
  void SetPlaystate(CONTENT_ADDON_PLAYSTATE newState)
  {
    return CONTENT_set_playstate(m_Handle, m_Callbacks, newState);
  }

protected:
  void* (*CONTENT_register_me)(void*);
  void (*CONTENT_unregister_me)(void*, void*);
  void (*CONTENT_set_playstate)(void*, void*, CONTENT_ADDON_PLAYSTATE);

private:
  void* m_libXBMC_content;
  void* m_Handle;
  void* m_Callbacks;
  struct cb_array
  {
    const char* libPath;
  };
};
