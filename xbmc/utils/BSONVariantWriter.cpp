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

#include "BSONVariantWriter.h"
#include "Base64.h"
#include "system.h" // include before bson.h
#include "lib/bson/src/bson.h"
#include "log.h"
#include "Variant.h"

using namespace std;

void CBSONVariantWriter::WriteBase64(const CVariant &value, string &output)
{
  vector<char> data;
  CBSONVariantWriter::Write(value, data);
  Base64::Encode(data.data(), data.size(), output);
}

string CBSONVariantWriter::WriteBase64(const CVariant &value)
{
  vector<char> data;
  CBSONVariantWriter::Write(value, data);
  return Base64::Encode(data.data(), data.size());
}

vector<char> CBSONVariantWriter::Write(const CVariant &value)
{
  vector<char> output;
  Write(value, output);
  return output;
}

void CBSONVariantWriter::Write(const CVariant &value, vector<char> &output)
{
  output.clear();

  bson document[1];
  bson_init(document);

  bool success = true;
  for (CVariant::const_iterator_map it = value.begin_map(); it != value.end_map() && success; it++)
    success &= InternalWrite(document, it->first.c_str(), it->second);

  if (bson_finish(document) != BSON_OK)
  {
    string error;
    switch (document->err)
    {
    case BSON_NOT_UTF8:
      error = "BSON_NOT_UTF8";
      break;
    case BSON_FIELD_HAS_DOT:
      error = "BSON_FIELD_HAS_DOT";
      break;
    case BSON_FIELD_INIT_DOLLAR:
      error = "BSON_FIELD_INIT_DOLLAR";
      break;
    case BSON_ALREADY_FINISHED:
      error = "BSON_ALREADY_FINISHED";
      break;
    case BSON_VALID:
      error = "BSON_VALID";
      break;
    default:
      error = "Unknown?";
      break;
    }
    CLog::Log(LOGERROR, "%s: writing the bson document returned %s", __FUNCTION__, error.c_str());
  }
  else
  {
    output = vector<char>(bson_data(document), bson_data(document) + bson_size(document));
  }
  bson_destroy(document);
}

bool CBSONVariantWriter::InternalWrite(bson *document, const char *name, const CVariant &value)
{
  bool success;

  switch (value.type())
  {
  case CVariant::VariantTypeInteger:
    success = bson_append_long(document, name, value.asInteger()) == BSON_OK;
    break;
  case CVariant::VariantTypeUnsignedInteger:
    success = bson_append_long(document, name, (int64_t)value.asUnsignedInteger()) == BSON_OK;
    break;
  case CVariant::VariantTypeDouble:
    success = bson_append_double(document, name, value.asDouble()) == BSON_OK;
    break;
  case CVariant::VariantTypeBoolean:
    success = bson_append_bool(document, name, value.asBoolean()) == BSON_OK;
    break;
  case CVariant::VariantTypeString:
    success = bson_append_string(document, name, value.asString().c_str()) == BSON_OK;
    break;
  case CVariant::VariantTypeArray:
    {
      success = bson_append_start_array(document, name) == BSON_OK;

      // For the array, we have to manually set the index values from "0" to "N"
      unsigned short i = 0;
      char index[6];
      for (CVariant::const_iterator_array it = value.begin_array(); it != value.end_array() && success; it++)
      {
        snprintf(index, sizeof(index), "%u", i++);
        success &= InternalWrite(document, index, *it);
      }
      success &= bson_append_finish_array(document) == BSON_OK;
    }
    break;
  case CVariant::VariantTypeObject:
    success = bson_append_start_object(document, name) == BSON_OK;

    for (CVariant::const_iterator_map it = value.begin_map(); it != value.end_map() && success; it++)
      success &= InternalWrite(document, it->first.c_str(), it->second);

    success &= bson_append_finish_object(document) == BSON_OK;

    break;
  case CVariant::VariantTypeNull:
  case CVariant::VariantTypeConstNull:
  default:
    success = bson_append_null(document, name) == BSON_OK;
    break;
  }

  return success;
}
