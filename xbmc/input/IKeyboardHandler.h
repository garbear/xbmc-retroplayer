/*
 *      Copyright (C) 2015 Team XBMC
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
#pragma once

class CKey;

/*!
 * \ingroup input
 * \brief Interface for handling keyboard events
 */
class IKeyboardHandler
{
public:
  virtual ~IKeyboardHandler(void) { }

  /*!
   * \brief A key has been pressed
   *
   * \param key      The pressed key
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnKeyPress(const CKey& key) = 0;

  /*!
   * \brief A key has been released
   *
   * \param key      The released key
   */
  virtual void OnKeyRelease(const CKey& key) = 0;
};
