/*
 *      Copyright (C) 2014 Team XBMC
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

#include "LibretroDLL.h"
#include "libXBMC_game.h"

#ifdef _WIN32
  #include "dlfcn-win32.h"
#else
  #include <dlfcn.h>
#endif

#include <assert.h>

using namespace ADDON;
using namespace LIBRETRO;

CLibretroDLL::CLibretroDLL(CHelper_libXBMC_addon* xbmc)
 : m_xbmc(xbmc),
   m_libretro(NULL)
{
  assert(m_xbmc);
}

void CLibretroDLL::Unload(void)
{
  if (m_libretro)
  {
    dlclose(m_libretro);
    m_libretro = NULL;
  }
}

#define LIBRETRO_REGISTER_SYMBOL(dll, functionPtr)  RegisterSymbol(dll, functionPtr, #functionPtr)

template <typename T>
bool RegisterSymbol(void* dll, T functionPtr, const char* strFunctionPtr)
{
  return (functionPtr = (T)dlsym(dll, strFunctionPtr)) != NULL;
}

bool CLibretroDLL::Load(const char* library_path)
{
  Unload();

  m_libretro = dlopen(library_path, RTLD_LAZY);
  if (m_libretro == NULL)
  {
    m_xbmc->Log(LOG_ERROR, "Unable to load %s", dlerror());
    return false;
  }

  try
  {
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_set_environment)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_set_video_refresh)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_set_audio_sample)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_set_audio_sample_batch)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_set_input_poll)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_set_input_state)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_init)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_deinit)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_api_version)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_get_system_info)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_get_system_av_info)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_set_controller_port_device)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_reset)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_run)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_serialize_size)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_serialize)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_unserialize)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_cheat_reset)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_cheat_set)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_load_game)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_load_game_special)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_unload_game)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_get_region)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_get_memory_data)) throw false;
    if (!LIBRETRO_REGISTER_SYMBOL(m_libretro, retro_get_memory_size)) throw false;
  }
  catch (const bool& bSuccess)
  {
    m_xbmc->Log(LOG_ERROR, "Unable to assign function %s", dlerror());
    return bSuccess;
  }

  return true;
}
