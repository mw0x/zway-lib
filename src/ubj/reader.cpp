
// ============================================================ //
//
//   d88888D db   d8b   db  .d8b.  db    db
//   YP  d8' 88   I8I   88 d8' `8b `8b  d8'
//      d8'  88   I8I   88 88ooo88  `8bd8'
//     d8'   Y8   I8I   88 88~~~88    88
//    d8' db `8b d8'8b d8' 88   88    88
//   d88888P  `8b8' `8d8'  YP   YP    YP
//
//   open-source, cross-platform, crypto-messenger
//
//   Copyright (C) 2018 Marc Weiler
//
//   This library is free software; you can redistribute it and/or
//   modify it under the terms of the GNU Lesser General Public
//   License as published by the Free Software Foundation; either
//   version 2.1 of the License, or (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   Lesser General Public License for more details.
//
// ============================================================ //

#include "Zway/ubj/reader.h"
#include "Zway/memorybuffer.h"

namespace Zway { namespace UBJ {

// ============================================================ //

/**
 * @brief Reader::read
 * @param val
 * @param buf
 * @return
 */

bool Reader::read(Value &val, const MemoryBuffer$ &buf)
{
    return read(val, buf->data(), buf->size());
}

/**
 * @brief Reader::read
 * @param val
 * @param data
 * @param size
 * @return
 */

bool Reader::read(Value &val, const uint8_t *data, uint32_t size)
{
    if (!(data && size)) {

        return false;
    }

    ubjr_context_t* ctx = ubjr_open_memory(data, data + size);

    ubjr_dynamic_t d =  ubjr_read_dynamic(ctx);

    if (d.type != UBJ_OBJECT && d.type != UBJ_ARRAY) {

        ubjr_close_context(ctx);

        return false;
    }

    readDynamic(val, d);

    ubjr_close_context(ctx);

    return true;
}

/**
 * @brief Reader::readDynamic
 * @param val
 * @param dyn
 */

void Reader::readDynamic(Value &val, ubjr_dynamic_t &dyn)
{
    switch (dyn.type) {
    case UBJ_NULLTYPE:
        break;
    case UBJ_INT32:
        val = (int32_t)dyn.integer;
        break;
    case UBJ_INT64:
        val = (int64_t)dyn.integer;
        break;
    case UBJ_FLOAT32:
        val = (float)dyn.real;
        break;
    case UBJ_FLOAT64:
        val = (double)dyn.real;
        break;
    case UBJ_BOOL_TRUE:
    case UBJ_BOOL_FALSE:
        val.m_type = dyn.type;
        break;
    case UBJ_STRING:
        val = std::string(dyn.string);
        break;
    case UBJ_OBJECT:
        val.m_type = dyn.type;
        readObject(val, dyn.container_object);
        break;
    case UBJ_ARRAY:
        val.m_type = dyn.type;
        readArray(val, dyn.container_array);
        break;
    default:
        break;
    }
}

/**
 * @brief Reader::readObject
 * @param val
 * @param obj
 */

void Reader::readObject(Value &val, ubjr_object_t &obj)
{
    Object res;

    for (uint32_t i=0; i<obj.size; i++) {

        ubjr_string_t k = obj.keys[i];

        ubjr_dynamic_t d = ubjr_object_lookup(&obj, k);

        readDynamic(res[k], d);
    }

    val = res;
}

/**
 * @brief Reader::readArray
 * @param val
 * @param arr
 */

void Reader::readArray(Value &val, ubjr_array_t &arr)
{
    if (arr.type == UBJ_MIXED) {

        Array res;

        for (uint32_t i=0; i<arr.size; i++) {

            ubjr_dynamic_t* dyn = (ubjr_dynamic_t*)((uint8_t*)arr.values + i * ubjr_local_type_size(arr.type));

            Value tmp;

            readDynamic(tmp, *dyn);

            res << tmp;
        }

        val = res;
    }
    else
    if (arr.type == UBJ_INT8) {

        val.m_buffer = MemoryBuffer::create((uint8_t*)arr.values, arr.size);
    }
}

// ============================================================ //

}

}
