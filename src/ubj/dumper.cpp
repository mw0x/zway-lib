
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
//   Copyright (C) 2016 Marc Weiler
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

#include "Zway/ubj/dumper.h"

#include <sstream>

namespace Zway { namespace UBJ {

// ============================================================ //

/**
 * @brief Dumper::dump
 * @param val
 * @param indent
 * @param depth
 * @return
 */

std::string Dumper::dump(const Value &val, int indent, int depth)
{
    std::stringstream ss;

    std::string ind = std::string(indent * depth, ' ');

    switch (val.m_type) {

        case UBJ_OBJECT: {

            Object obj(val);

            if (!obj.empty()) {

                ss << "{\n";

                for (auto &it : obj) {

                    ss << std::string(indent * (depth+1), ' ') << it.first << " : " << dump(it.second, indent, depth + 1) << "\n";
                }

                ss << ind << "}";
            }
            else {

                ss << "{}";
            }

            break;
        }

        case UBJ_ARRAY:

            if (!val.bufferSize()) {

                Array arr(val);

                if (!arr.empty()) {

                    ss << "[\n";

                    for (size_t i = 0; i < arr.size(); ++i) {

                        ss << std::string(indent * (depth+1), ' ') << i << " : " << dump(arr[i], indent, depth + 1) << "\n";
                    }

                    ss << ind << "]";
                }
                else {

                    ss << "[]";
                }
            }
            else {

                ss << "DATA " << val.bufferSize();
            }

            break;

        case UBJ_BOOL_TRUE:

            ss << "TRUE";

            break;

        case UBJ_BOOL_FALSE:

            ss << "FALSE";

            break;

        case UBJ_INT32:

            ss << val.toInt();

            break;

        case UBJ_INT64:

            ss << val.toLong();

            break;

        case UBJ_FLOAT32:

            ss << val.toFloat();

            break;

        case UBJ_FLOAT64:

            ss << val.toDouble();

            break;

        case UBJ_STRING:

            ss << '"' << val.toStr() << '"';

            break;

        case UBJ_NULLTYPE:

            ss << "NULL";

            break;

        default:

            break;
    }

    return ss.str();
}

// ============================================================ //

}}
