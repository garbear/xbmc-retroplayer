/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "BSONVariantParser.h"
#include "Base64.h"
#include "system.h" // include before bson.h
#include "lib/bson/src/bson.h"

using namespace std;

CVariant CBSONVariantParser::ParseBase64(const std::string &bsonBase64)
{
  string bsonDecoded;
  Base64::Decode(bsonBase64, bsonDecoded);
  return Parse(bsonDecoded.c_str());
}

CVariant CBSONVariantParser::Parse(const char *bsonData)
{
  CVariant output;

  bson document[1];
  bson_init_finished_data(document, const_cast<char*>(bsonData));

  bson_iterator it[1];
  bson_iterator_init(it, document);

  bson_type type;
  type = bson_iterator_next(it);

  while (type != BSON_EOO)
  {
    output[bson_iterator_key(it)] = InternalParse(type, it);
    type = bson_iterator_next(it);
  }

  return output;
}

CVariant CBSONVariantParser::InternalParse(int type, bson_iterator *it)
{
  switch (type)
  {
  case BSON_INT:
    return CVariant(bson_iterator_int(it));
  case BSON_LONG:
    return CVariant(bson_iterator_long(it));
  case BSON_DOUBLE:
    return CVariant(bson_iterator_double(it));
  case BSON_BOOL:
    return CVariant(bson_iterator_bool(it));
  case BSON_STRING:
    return CVariant(bson_iterator_string(it));
  case BSON_ARRAY:
  case BSON_OBJECT:
    {
      CVariant output;

      bson_iterator subit[1];
      bson_iterator_subiterator(it, subit);

      bson_type subtype;
      subtype = bson_iterator_next(subit);

      while (subtype != BSON_EOO)
      {
        if (type == BSON_ARRAY)
          output.push_back(InternalParse(subtype, subit));
        else
          output[bson_iterator_key(subit)] = InternalParse(subtype, subit);
        subtype = bson_iterator_next(subit);
      }

      return output;
    }
  case BSON_NULL:
  default:
    return CVariant();
  }
}
