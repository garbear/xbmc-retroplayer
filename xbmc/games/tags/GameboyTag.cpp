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

#include "GameboyTag.h"
#include "GameInfoTagLoader.h"
#include "filesystem/File.h"
#include "utils/log.h"

using namespace GAME_INFO;
using namespace XFILE;

// Constants and file format data from VBA-M project
// http://sourceforge.net/projects/vbam
// This code references VBA-M at r1118, later commits may contain fixes

/* static */
void CGameboyTag::Load(const CStdString& strPath, CGameInfoTag &gameInfoTag)
{
  // Open the file and read in the data
  CFile file;
  if (!file.Open(strPath))
    return;
  
  char data[0x147 + 2];
  if (file.Read(data, sizeof(data)) != sizeof(data))
    return;

  // Correct the platform if the extension was wrong
  if (data[0x143] & 0x80)
    gameInfoTag.SetPlatform(CGameInfoTagLoader::GetPlatformByID(PLATFORM_GAMEBOY_COLOR).name);
  //else if (data[0x146] == 0x03)
  //  m_gameInfoTag.SetPlatform(CGameInfoTagLoader::GetPlatformByID(PLATFORM_SUPER_GAMEBOY).name);
  else
    gameInfoTag.SetPlatform(CGameInfoTagLoader::GetPlatformByID(PLATFORM_GAMEBOY).name);

  // Title: $134, 15 bytes
  for (unsigned int i = 0x134; i < 0x134 + 15; i++)
  {
    if ((0 < data[i] && data[i] < 32) || 126 < data[i]) // zero is ok
    {
      CLog::Log(LOGINFO, "CGameboyTag: Non-ASCII data encountered, assuming it's not a game image");
      return;
    }
  }
  gameInfoTag.SetTitle(CStdString(std::string(data + 0x134, 15)).Trim());

  // Publisher: $144, 2 bytes
  gameInfoTag.SetPublisher(CGameboyTag::TranslatePublisher(data + 0x144));

  // Cartridge Type: $147, 1 byte
  CStdString type;
  switch ((unsigned char)data[0x147])
  {
  case 0x00:
    type = "ROM";
    break;
  case 0x01:
    type = "ROM+MBC1";
    break;
  case 0x02:
    type = "ROM+MBC1+RAM";
    break;
  case 0x03:
    type = "ROM+MBC1+RAM+BATT";
    break;
  case 0x05:
    type = "ROM+MBC2";
    break;
  case 0x06:
    type = "ROM+MBC2+BATT";
    break;
  case 0x0b:
    type = "ROM+MMM01";
    break;
  case 0x0c:
    type = "ROM+MMM01+RAM";
    break;
  case 0x0d:
    type = "ROM+MMM01+RAM+BATT";
    break;
  case 0x0f:
    type = "ROM+MBC3+TIMER+BATT";
    break;
  case 0x10:
    type = "ROM+MBC3+TIMER+RAM+BATT";
    break;
  case 0x11:
    type = "ROM+MBC3";
    break;
  case 0x12:
    type = "ROM+MBC3+RAM";
    break;
  case 0x13:
    type = "ROM+MBC3+RAM+BATT";
    break;
  case 0x19:
    type = "ROM+MBC5";
    break;
  case 0x1a:
    type = "ROM+MBC5+RAM";
    break;
  case 0x1b:
    type = "ROM+MBC5+RAM+BATT";
    break;
  case 0x1c:
    type = "ROM+MBC5+RUMBLE";
    break;
  case 0x1d:
    type = "ROM+MBC5+RUMBLE+RAM";
    break;
  case 0x1e:
    type = "ROM+MBC5+RUMBLE+RAM+BATT";
    break;
  case 0x22:
    type = "ROM+MBC7+BATT";
    break;
  case 0x55:
    type = "GameGenie";
    break;
  case 0x56:
    type = "GameShark V3.0";
    break;
  case 0xfc:
    type = "ROM+POCKET CAMERA";
    break;
  case 0xfd:
    type = "ROM+BANDAI TAMA5";
    break;
  case 0xfe:
    type = "ROM+HuC-3";
    break;
  case 0xff:
    type = "ROM+HuC-1";
    break;
  }
  gameInfoTag.SetCartridgeType(type);
}

const char *CGameboyTag::TranslatePublisher(const char *code)
{
  if (!code) return "";

  static struct
  {
    const char *code;
    const char *name;
  }
    companies[] =
  {
    { "01", "Nintendo" },
    { "02", "Rocket Games" },
    { "08", "Capcom" },
    { "09", "Hot B Co." },
    { "0A", "Jaleco" },
    { "0B", "Coconuts Japan" },
    { "0C", "Coconuts Japan/G.X.Media" },
    { "0H", "Starfish" },
    { "0L", "Warashi Inc." },
    { "0N", "Nowpro" },
    { "0P", "Game Village" },
    { "13", "Electronic Arts Japan" },
    { "18", "Hudson Soft Japan" },
    { "19", "S.C.P." },
    { "1A", "Yonoman" },
    { "1G", "SMDE" },
    { "1P", "Creatures Inc." },
    { "1Q", "TDK Deep Impresion" },
    { "20", "Destination Software" },
    { "22", "VR 1 Japan" },
    { "25", "San-X" },
    { "28", "Kemco Japan" },
    { "29", "Seta" },
    { "2H", "Ubisoft Japan" },
    { "2K", "NEC InterChannel" },
    { "2L", "Tam" },
    { "2M", "Jordan" },
    { "2N", "Smilesoft" },
    { "2Q", "Mediakite" },
    { "36", "Codemasters" },
    { "37", "GAGA Communications" },
    { "38", "Laguna" },
    { "39", "Telstar Fun and Games" },
    { "41", "Ubi Soft Entertainment" },
    { "42", "Sunsoft" },
    { "47", "Spectrum Holobyte" },
    { "49", "IREM" },
    { "4D", "Malibu Games" },
    { "4F", "Eidos/U.S. Gold" },
    { "4J", "Fox Interactive" },
    { "4K", "Time Warner Interactive" },
    { "4Q", "Disney" },
    { "4S", "Black Pearl" },
    { "4X", "GT Interactive" },
    { "4Y", "RARE" },
    { "4Z", "Crave Entertainment" },
    { "50", "Absolute Entertainment" },
    { "51", "Acclaim" },
    { "52", "Activision" },
    { "53", "American Sammy Corp." },
    { "54", "Take 2 Interactive" },
    { "55", "Hi Tech" },
    { "56", "LJN LTD." },
    { "58", "Mattel" },
    { "5A", "Mindscape/Red Orb Ent." },
    { "5C", "Taxan" },
    { "5D", "Midway" },
    { "5F", "American Softworks" },
    { "5G", "Majesco Sales Inc" },
    { "5H", "3DO" },
    { "5K", "Hasbro" },
    { "5L", "NewKidCo" },
    { "5M", "Telegames" },
    { "5N", "Metro3D" },
    { "5P", "Vatical Entertainment" },
    { "5Q", "LEGO Media" },
    { "5S", "Xicat Interactive" },
    { "5T", "Cryo Interactive" },
    { "5W", "Red Storm Ent./BKN Ent." },
    { "5X", "Microids" },
    { "5Z", "Conspiracy Entertainment Corp." },
    { "60", "Titus Interactive Studios" },
    { "61", "Virgin Interactive" },
    { "62", "Maxis" },
    { "64", "LucasArts Entertainment" },
    { "67", "Ocean" },
    { "69", "Electronic Arts" },
    { "6E", "Elite Systems Ltd." },
    { "6F", "Electro Brain" },
    { "6G", "The Learning Company" },
    { "6H", "BBC" },
    { "6J", "Software 2000" },
    { "6L", "BAM! Entertainment" },
    { "6M", "Studio 3" },
    { "6Q", "Classified Games" },
    { "6S", "TDK Mediactive" },
    { "6U", "DreamCatcher" },
    { "6V", "JoWood Productions" },
    { "6W", "SEGA" },
    { "6X", "Wannado Edition" },
    { "6Y", "LSP" },
    { "6Z", "ITE Media" },
    { "70", "Infogrames" },
    { "71", "Interplay" },
    { "72", "JVC Musical Industries Inc" },
    { "73", "Parker Brothers" },
    { "75", "SCI" },
    { "78", "THQ" },
    { "79", "Accolade" },
    { "7A", "Triffix Ent. Inc." },
    { "7C", "Microprose Software" },
    { "7D", "Universal Interactive Studios" },
    { "7F", "Kemco" },
    { "7G", "Rage Software" },
    { "7H", "Encore" },
    { "7J", "Zoo" },
    { "7K", "BVM" },
    { "7L", "Simon & Schuster Interactive" },
    { "7M", "Asmik Ace Entertainment Inc./AIA" },
    { "7N", "Empire Interactive" },
    { "7Q", "Jester Interactive" },
    { "7T", "Scholastic" },
    { "7U", "Ignition Entertainment" },
    { "7W", "Stadlbauer" },
    { "80", "Misawa" },
    { "83", "LOZC" },
    { "8B", "Bulletproof Software" },
    { "8C", "Vic Tokai Inc." },
    { "8J", "General Entertainment" },
    { "8N", "Success" },
    { "8P", "SEGA Japan" },
    { "91", "Chun Soft" },
    { "92", "Video System" },
    { "93", "BEC" },
    { "96", "Yonezawa/S'pal" },
    { "97", "Kaneko" },
    { "99", "Victor Interactive Software" },
    { "9A", "Nichibutsu/Nihon Bussan" },
    { "9B", "Tecmo" },
    { "9C", "Imagineer" },
    { "9F", "Nova" },
    { "9H", "Bottom Up" },
    { "9L", "Hasbro Japan" },
    { "9N", "Marvelous Entertainment" },
    { "9P", "Keynet Inc." },
    { "9Q", "Hands-On Entertainment" },
    { "A0", "Telenet" },
    { "A1", "Hori" },
    { "A4", "Konami" },
    { "A6", "Kawada" },
    { "A7", "Takara" },
    { "A9", "Technos Japan Corp." },
    { "AA", "JVC" },
    { "AC", "Toei Animation" },
    { "AD", "Toho" },
    { "AF", "Namco" },
    { "AG", "Media Rings Corporation" },
    { "AH", "J-Wing" },
    { "AK", "KID" },
    { "AL", "MediaFactory" },
    { "AP", "Infogrames Hudson" },
    { "AQ", "Kiratto. Ludic Inc" },
    { "B0", "Acclaim Japan" },
    { "B1", "ASCII" },
    { "B2", "Bandai" },
    { "B4", "Enix" },
    { "B6", "HAL Laboratory" },
    { "B7", "SNK" },
    { "B9", "Pony Canyon Hanbai" },
    { "BA", "Culture Brain" },
    { "BB", "Sunsoft" },
    { "BD", "Sony Imagesoft" },
    { "BF", "Sammy" },
    { "BG", "Magical" },
    { "BJ", "Compile" },
    { "BL", "MTO Inc." },
    { "BN", "Sunrise Interactive" },
    { "BP", "Global A Entertainment" },
    { "BQ", "Fuuki" },
    { "C0", "Taito" },
    { "C2", "Kemco" },
    { "C3", "Square Soft" },
    { "C5", "Data East" },
    { "C6", "Tonkin House" },
    { "C8", "Koei" },
    { "CA", "Konami/Palcom/Ultra" },
    { "CB", "Vapinc/NTVIC" },
    { "CC", "Use Co.,Ltd." },
    { "CD", "Meldac" },
    { "CE", "FCI/Pony Canyon" },
    { "CF", "Angel" },
    { "CM", "Konami Computer Entertainment Osaka" },
    { "CP", "Enterbrain" },
    { "D1", "Sofel" },
    { "D2", "Quest" },
    { "D3", "Sigma Enterprises" },
    { "D4", "Ask Kodansa" },
    { "D6", "Naxat" },
    { "D7", "Copya System" },
    { "D9", "Banpresto" },
    { "DA", "TOMY" },
    { "DB", "LJN Japan" },
    { "DD", "NCS" },
    { "DF", "Altron Corporation" },
    { "DH", "Gaps Inc." },
    { "DN", "ELF" },
    { "E2", "Yutaka" },
    { "E3", "Varie" },
    { "E5", "Epoch" },
    { "E7", "Athena" },
    { "E8", "Asmik Ace Entertainment Inc." },
    { "E9", "Natsume" },
    { "EA", "King Records" },
    { "EB", "Atlus" },
    { "EC", "Epic/Sony Records" },
    { "EE", "IGS" },
    { "EL", "Spike" },
    { "EM", "Konami Computer Entertainment Tokyo" },
    { "EN", "Alphadream Corporation" },
    { "F0", "A Wave" },
    { "G1", "PCCW" },
    { "G4", "KiKi Co Ltd" },
    { "G5", "Open Sesame Inc." },
    { "G6", "Sims" },
    { "G7", "Broccoli" },
    { "G8", "Avex" },
    { "G9", "D3 Publisher" },
    { "GB", "Konami Computer Entertainment Japan" },
    { "GD", "Square-Enix" },
    { "HY", "Sachen" },
    { NULL, NULL }
  };

  for (int i = 0; companies[i].code; i++)
  {
    if (companies[i].code[0] == code[0] && companies[i].code[1] == code[1])
      return companies[i].name;
  }
  return "";
}
