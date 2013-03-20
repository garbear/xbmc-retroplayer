/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include "LibretroEnvironment.h"
#include "Application.h"
#include "utils/log.h"


CLibretroEnvironment::SetPixelFormat_t      CLibretroEnvironment::fn_SetPixelFormat      = NULL;
CLibretroEnvironment::SetKeyboardCallback_t CLibretroEnvironment::fn_SetKeyboardCallback = NULL;

void CLibretroEnvironment::SetCallbacks(SetPixelFormat_t spf, SetKeyboardCallback_t skc)
{
  fn_SetPixelFormat = spf;
  fn_SetKeyboardCallback = skc;
}

void CLibretroEnvironment::ResetCallbacks()
{
  fn_SetPixelFormat = NULL;
  fn_SetKeyboardCallback = NULL;
}

bool CLibretroEnvironment::EnvironmentCallback(unsigned int cmd, void *data)
{
  // Note: SHUTDOWN doesn't use data and GET_SYSTEM_DIRECTORY uses data as a return path
  if (!(cmd == RETRO_ENVIRONMENT_SHUTDOWN || cmd == RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY) && !data)
  {
    CLog::Log(LOGERROR, "CLibretroEnvironment query ID=%d: no data! naughty core?", cmd);
    return false;
  }

  switch (cmd)
  {
  case RETRO_ENVIRONMENT_GET_OVERSCAN:
    {
      // Whether or not the game client should use overscan (true) or crop away overscan (false)
      *reinterpret_cast<bool*>(data) = false;
      CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: %s", RETRO_ENVIRONMENT_GET_OVERSCAN,
          *reinterpret_cast<bool*>(data) ? "use overscan" : "crop away overscan");
      break;
    }
  case RETRO_ENVIRONMENT_GET_CAN_DUPE:
    {
      // Boolean value whether or not we support frame duping, passing NULL to video frame callback
      *reinterpret_cast<bool*>(data) = true;
      CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: frame duping is %s",
          RETRO_ENVIRONMENT_GET_CAN_DUPE, *reinterpret_cast<bool*>(data) ? "enabled" : "disabled");
      break;
    }
  case RETRO_ENVIRONMENT_GET_VARIABLE:
    {
      // Interface to acquire user-defined information from environment that cannot feasibly be
      // supported in a multi-system way. Mostly used for obscure, specific features that the
      // user can tap into when necessary.
      retro_variable *var = reinterpret_cast<retro_variable*>(data);
      if (var->key && var->value)
      {
        // For example...
        if (strncmp("too_sexy_for", var->key, 12) == 0)
        {
          var->value = "my_shirt";
          CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: variable %s set to %s",
              RETRO_ENVIRONMENT_GET_VARIABLE, var->key, var->value);
        }
        else
        {
          var->value = NULL;
          CLog::Log(LOGERROR, "CLibretroEnvironment query ID=%d: undefined variable %s",
              RETRO_ENVIRONMENT_GET_VARIABLE, var->key);
        }
      }
      else
      {
        if (var->value)
          var->value = NULL;
        CLog::Log(LOGERROR, "CLibretroEnvironment query ID=%d: no variable given",
            RETRO_ENVIRONMENT_GET_VARIABLE);
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_VARIABLES:
    {
      // Allows an implementation to signal the environment which variables it might want to check
      // for later using GET_VARIABLE. 'data' points to an array of retro_variable structs terminated
      // by a { NULL, NULL } element. retro_variable::value should contain a human readable description
      // of the key.
      const retro_variable *vars = reinterpret_cast<const retro_variable*>(data);
      if (!vars->key)
      {
        CLog::Log(LOGERROR, "CLibretroEnvironment query ID=%d: no variables given",
            RETRO_ENVIRONMENT_SET_VARIABLES);
      }
      else
      {
        while (vars && vars->key)
        {
          if (vars->value)
            CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: notified of var %s (%s)",
              RETRO_ENVIRONMENT_SET_VARIABLES, vars->key, vars->value);
          else
            CLog::Log(LOGWARNING, "CLibretroEnvironment query ID=%d: var %s has no description",
              RETRO_ENVIRONMENT_SET_VARIABLES, vars->key);
          vars++;
        }
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_MESSAGE:
    {
      // Sets a message to be displayed. Generally not for trivial messages.
      const retro_message *msg = reinterpret_cast<const retro_message*>(data);
      if (msg->msg && msg->frames)
        CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: display msg \"%s\" for %d frames",
            RETRO_ENVIRONMENT_SET_MESSAGE, msg->msg, msg->frames);
      break;
    }
  case RETRO_ENVIRONMENT_SET_ROTATION:
    {
      // Sets screen rotation of graphics. Valid values are 0, 1, 2, 3, which rotates screen
      // by 0, 90, 180, 270 degrees counter-clockwise respectively.
      unsigned int rotation = *reinterpret_cast<const unsigned int*>(data);
      if (0 <= rotation && rotation <= 3)
        CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: set screen rotation to %d degrees",
            RETRO_ENVIRONMENT_SET_ROTATION, rotation * 90);
      else
        CLog::Log(LOGERROR, "CLibretroEnvironment query ID=%d: invalid rotation %d",
            RETRO_ENVIRONMENT_SET_ROTATION, rotation);
      break;
    }
  case RETRO_ENVIRONMENT_SHUTDOWN:
    // Game has been shut down. Should only be used if game has a specific way to shutdown
    // the game from a menu item or similar.
    CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: game signaled shutdown event", RETRO_ENVIRONMENT_SHUTDOWN);

    if (g_application.IsPlayingGame())
      g_application.StopPlaying();

    break;
  case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
    {
      // Generally how computationally intense this core is, to gauge how capable XBMC's host system
      // will be for running the core. It can also be called on a game-specific basis. The levels
      // are "floating", but roughly defined as:
      // 0: Low-powered embedded devices such as Raspberry Pi
      // 1: Phones, tablets, 6th generation consoles, such as Wii/Xbox 1, etc
      // 2: 7th generation consoles, such as PS3/360, with sub-par CPUs.
      // 3: Modern desktop/laptops with reasonably powerful CPUs.
      // 4: High-end desktops with very powerful CPUs.
      unsigned int performanceLevel = *reinterpret_cast<const unsigned int*>(data);
      if (0 <= performanceLevel && performanceLevel <= 3)
        CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: performance hint: %d",
            RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, performanceLevel);
      else if (performanceLevel == 4)
        CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: performance hint: I hope you have a badass computer...",
            RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL);
      else
        CLog::Log(LOGERROR, "GameClient environment query ID=%d: invalid performance hint: %d",
            RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, performanceLevel);
      break;
    }
  case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    {
      // Returns a directory for storing system specific ROMs such as BIOSes, configuration data,
      // etc. The returned value can be NULL, in which case it's up to the implementation to find
      // a suitable directory.
      *reinterpret_cast<const char**>(data) = NULL;
      CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: no system directory given to core",
          RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY);
      break;
    }
  case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
    {
      // Get the internal pixel format used by the core. The default pixel format is
      // RETRO_PIXEL_FORMAT_0RGB1555 (see libretro.h). Returning false lets the core
      // know that XBMC does not support the pixel format.
      retro_pixel_format pix_fmt = *reinterpret_cast<const retro_pixel_format*>(data);
      // Validate the format
      switch (pix_fmt)
      {
      case RETRO_PIXEL_FORMAT_0RGB1555: // 5 bit color, high bit must be zero
      case RETRO_PIXEL_FORMAT_XRGB8888: // 8 bit color, high byte is ignored
      case RETRO_PIXEL_FORMAT_RGB565:   // 5/6/5 bit color
        CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: set pixel format: %d",
            RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, pix_fmt);
        if (fn_SetPixelFormat)
          fn_SetPixelFormat(pix_fmt);
        break;
      default:
        CLog::Log(LOGERROR, "CLibretroEnvironment query ID=%d: invalid pixel format: %d",
            RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, pix_fmt);
        return false;
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
    {
      // Describes the internal input bind through a human-readable string.
      // This string can be used to better let a user configure input. The array
      // is terminated by retro_input_descriptor::description being set to NULL.
      const retro_input_descriptor *descriptor = reinterpret_cast<const retro_input_descriptor*>(data);

      if (!descriptor->description)
      {
        CLog::Log(LOGERROR, "CLibretroEnvironment query ID=%d: no descriptors given",
            RETRO_ENVIRONMENT_SET_VARIABLES);
      }
      else
      {
        while (descriptor && descriptor->description)
        {
          CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: notified of input %s (port=%d, device=%d, index=%d, id=%d)",
            RETRO_ENVIRONMENT_SET_VARIABLES, descriptor->description,
              descriptor->port, descriptor->device, descriptor->index, descriptor->id);
          descriptor++;
        }
      }
      break;
    }
  case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK:
    {
      // Sets a callback function, called by XBMC, used to notify core about
      // keyboard events.
      const retro_keyboard_callback *callback_struct = reinterpret_cast<const retro_keyboard_callback*>(data);
      if (callback_struct->callback)
      {
        CLog::Log(LOGINFO, "CLibretroEnvironment query ID=%d: set keyboard callback", RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK);
        if (fn_SetKeyboardCallback)
          fn_SetKeyboardCallback(callback_struct->callback);
      }
      break;
    }
  default:
    CLog::Log(LOGERROR, "CLibretroEnvironment query: invalid query: %u", cmd);
    return false;
  }
  return true;
}
