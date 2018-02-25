
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

#include "Zway/ubjsender.h"
#include "Zway/memorybuffer.h"

namespace Zway {

// ============================================================ //

/**
 * @brief UbjSender::create
 * @param id
 * @param type
 * @param value
 * @param callback
 * @return
 */

UbjSender$ UbjSender::create(
        uint32_t id,
        Packet::StreamType type,
        const UBJ::Value &value,
        StreamSenderCallback callback)
{
    UbjSender$ sender(new UbjSender(id, type, value, callback));

    if (!sender->init(value)) {

        return nullptr;
    }

    return sender;
}

/**
 * @brief UbjSender::UbjSender
 * @param id
 * @param type
 * @param value
 * @param callback
 */

UbjSender::UbjSender(
        uint32_t id,
        Packet::StreamType type,
        const UBJ::Value &value,
        StreamSenderCallback callback)
    : BufferSender(id, type, nullptr, callback)
{

}

/**
 * @brief UbjSender::init
 * @param value
 * @return
 */

bool UbjSender::init(const UBJ::Value &value)
{
    m_buffer = UBJ::Value::write(value);

    if (!m_buffer) {

        return false;
    }

    if (!BufferSender::init()) {

        return false;
    }

    return true;
}

// ============================================================ //

}
