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

#include "GBATag.h"
#include "GameboyTag.h"
#include "filesystem/File.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/StdString.h"

using namespace GAME_INFO;
using namespace XFILE;


// Constants and file format data from VBA-M project
// http://sourceforge.net/projects/vbam
// This code references VBA-M at r1118, later commits may contain fixes

void CGBATag::Load(const CStdString& strPath, CGameInfoTag &gameInfoTag)
{
  CStdString strExtension;
  URIUtils::GetExtension(strPath, strExtension);

  if (strExtension.Equals(".elf"))
  {
    // ELF files not supported yet
    return;
  }

  // Open the file and read in the data
  CFile file;
  if (!file.Open(strPath))
    return;
  
  // Useful data is all ASCII, fortunately. Some stats like main unit code,
  // device type, version and CRC are omitted here.
  unsigned char data[0xb0 + 3]; // Until the end of the publisher ID bytes
  if (file.Read(data, sizeof(data)) != sizeof(data))
    return;

  // The properties below must all be ASCII text
  if (!AllASCII(data + 0xa0, 12) ||
      !AllASCII(data + 0xac, 4) ||
      !AllASCII(data + 0xb0, 2))
  {
    CLog::Log(LOGERROR, "CGBATag: Non-ASCII data encountered while reading image parameters");
    return;
  }

  // Title: $a0, 12 bytes
  gameInfoTag.SetTitle(std::string(reinterpret_cast<char*>(data) + 0xa0, 12));

  // ID: $ac, 4 bytes
  gameInfoTag.SetID(std::string(reinterpret_cast<char*>(data) + 0xac, 4));

  // Publisher: $b0, 2 bytes
  gameInfoTag.SetPublisher(CGameboyTag::TranslatePublisher(reinterpret_cast<char*>(data) + 0xb0));
}

bool CGBATag::AllASCII(unsigned char *b, unsigned int size)
{
  for (unsigned int i = 0; i < size; i++)
    if (b[i] < 32 || b[i] > 126)
      return false;
  return true;
}
