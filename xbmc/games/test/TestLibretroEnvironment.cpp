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

#include "games/libretro/LibretroEnvironment.h"

#include "gtest/gtest.h"

using namespace GAMES;

TEST(TestLibretroEnvironment, ParseVariable)
{
  // https://github.com/libretro/fba-libretro/commit/e79a483fafb5223f4a9a229c9d5835f549cf859e
  // https://github.com/libretro/desmume-libretro/commit/8fcda7c85bd3abc8cfe01bf1dd6eb9c6287d4753#L0L185
  // https://github.com/libretro/fceu-next/commit/91549d03714db9b5e1647cdbfa798b0fc9ebb77c
  // https://github.com/libretro/Genesis-Plus-GX/commit/d8af16f193c712ba053f07d9fe3350ee85d8cfbb
  // https://github.com/libretro/snes9x-next/commit/64e2e4e47d3fb0f59f799ce3fa25340421445f93

  CStdString description;
  CStdStringArray values;
  CStdString strDefault;
  retro_variable var = {"bool_test", "bool test; true|false"};

  TiXmlElement setting("setting");
  CLibretroEnvironment::ParseVariable(var, setting, strDefault);
  //EXPECT_EQ(type, SETTING_TYPE_BOOL);
  EXPECT_STREQ("bool test", description.c_str());
  EXPECT_EQ(values.size(), 2);
  EXPECT_STREQ("true", strDefault.c_str());

  var.value = "true|false; bool test";
  //type = CLibretroEnvironment::ParseVariable(, description, values);
  //EXPECT_NE(type, SETTING_TYPE_BOOL);
  EXPECT_STREQ("true|false", description.c_str());
  EXPECT_STREQ("bool test", strDefault.c_str());

  var.value = "bool test; no|yes";
  //type = CLibretroEnvironment::ParseVariable(, description, values);
  //EXPECT_EQ(type, SETTING_TYPE_BOOL);
  EXPECT_STREQ("bool test", description.c_str());
  EXPECT_EQ(values.size(), 2);
  EXPECT_STREQ("false", strDefault.c_str());
  if (values.size() == 2)
  {
    EXPECT_STREQ("false", values[0].c_str());
    EXPECT_STREQ("true", values[1].c_str());
  }
}
