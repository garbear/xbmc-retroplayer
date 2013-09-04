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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#pragma once

#include "settings/ISettingCallback.h"

#include <boost/noncopyable.hpp>

namespace GAMES
{
  class CGameSettings : public ISettingCallback, boost::noncopyable
  {
  public:
    static CGameSettings &Get();
    
    // Inherited from ISettingCallback
    virtual bool OnSettingChanging(const CSetting *setting);
    virtual void OnSettingChanged(const CSetting *setting);
    virtual void OnSettingAction(const CSetting *setting);
    virtual bool OnSettingUpdate(CSetting* &setting, const char *oldSettingId, const TiXmlNode *oldSettingNode);
    virtual void OnSettingPropertyChanged(const CSetting *setting, const char *propertyName);

  protected:
    CGameSettings() { }
    virtual ~CGameSettings() { }
  };
} // namespace GAMES
