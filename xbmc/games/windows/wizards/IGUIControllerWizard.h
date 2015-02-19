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

#include <string>

namespace GAME
{

class IGUIControllerWizardCallbacks
{
public:
  virtual ~IGUIControllerWizardCallbacks(void) { }

  virtual void Focus(unsigned int iFeature) = 0;
  virtual void SetLabel(unsigned int iFeature, const std::string& strLabel) = 0;
  virtual void ResetLabel(unsigned int iFeature) = 0;
  virtual void End(void) = 0;
};

/*!
 * \brief FSM that walks the user through mapping controllers
 */
class IGUIControllerWizard
{
public:
  virtual ~IGUIControllerWizard(void) { }

  /*!
   * \brief Start wizard
   */
  virtual void Run(unsigned int iStartFeature = 0) = 0;

  /*!
   * \brief Abort a running wizard
   * \return true if the wizard was aborted
   */
  virtual bool Abort(void) = 0;

  virtual void OnFocus(unsigned int featureIndex) = 0;
};

}
