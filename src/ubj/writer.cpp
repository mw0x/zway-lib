
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

#include "Zway/ubj/writer.h"
#include "Zway/memorybuffer.h"

#include <vector>

namespace Zway { namespace UBJ {

// ============================================================ //

/**
 * @brief Writer::write
 * @param val
 * @return
 */

MemoryBuffer$ Writer::write(const Value &val)
{
    std::vector<uint8_t> buf;

    auto writeCb = [] (const void* data, size_t size, size_t count, void* userdata) -> size_t {

        std::vector<uint8_t>* buf = (std::vector<uint8_t>*)userdata;

        buf->insert(buf->end(), (uint8_t*)data, (uint8_t*)data + size * count);

        return size * count;
    };

    auto closeCb = [] (void* /*userdata*/) -> int {

        return 0;
    };

    auto errorCb = [] (const char* /*error_msg*/) {

    };

    ubjw_context_t* ctx = ubjw_open_callback(&buf, writeCb, closeCb, errorCb);

    if (!ctx) {

        return nullptr;
    }

    buf.reserve(1024);

    if (val.m_type == UBJ_OBJECT) {

        writeObject(val, ctx);
    }
    else
    if (val.m_type == UBJ_ARRAY) {

        writeArray(val, ctx);
    }

    return MemoryBuffer::create(&buf[0], buf.size());
}

/**
 * @brief Writer::writeObject
 * @param val
 * @param ctx
 */

void Writer::writeObject(const Value &val, ubjw_context_t *ctx)
{
    Object obj(val);

    ubjw_begin_object(ctx, UBJ_MIXED, obj.size());

    for (auto &it : obj) {

        writeValue(it.first, it.second, ctx);
    }

    ubjw_end(ctx);
}

/**
 * @brief Writer::writeArray
 * @param val
 * @param ctx
 */

void Writer::writeArray(const Value &val, ubjw_context_t *ctx)
{
    if (!val.bufferSize()) {

        Array arr(val);

        ubjw_begin_array(ctx, UBJ_MIXED, arr.size());

        for (auto &it : arr) {

            writeValue(std::string(), it, ctx);
        }

        ubjw_end(ctx);
    }
    else {

        ubjw_write_buffer(ctx, val.bufferData(), UBJ_INT8, val.bufferSize());
    }
}

/**
 * @brief Writer::writeValue
 * @param key
 * @param val
 * @param ctx
 */

void Writer::writeValue(const std::string &key, const Value &val, ubjw_context_t *ctx)
{
    if (!key.empty()) {

        ubjw_write_key(ctx, key.c_str());
    }

    switch (val.m_type) {
    case UBJ_NULLTYPE:
        ubjw_write_null(ctx);
        break;
    case UBJ_INT32:
        ubjw_write_int32(ctx, val.toInt());
        break;
    case UBJ_INT64:
        ubjw_write_int64(ctx, val.toLong());
        break;
    case UBJ_FLOAT32:
        ubjw_write_float32(ctx, val.toFloat());
        break;
    case UBJ_FLOAT64:
        ubjw_write_float64(ctx, val.toDouble());
        break;
    case UBJ_BOOL_TRUE:
    case UBJ_BOOL_FALSE:
        ubjw_write_bool(ctx, val.m_type == UBJ_BOOL_TRUE ? 1 : 0);
        break;
    case UBJ_STRING:
        ubjw_write_string(ctx, val.toStr().c_str());
        break;
    case UBJ_OBJECT:
        writeObject(val, ctx);
        break;
    case UBJ_ARRAY:
        writeArray(val, ctx);
        break;
    default:
        break;
    }
}

// ============================================================ //

}

}
