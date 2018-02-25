
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

#include "Zway/buffersender.h"
#include "Zway/memorybuffer.h"

namespace Zway {

// ============================================================ //

/**
 * @brief BufferSender::create
 * @param id
 * @param type
 * @param buffer
 * @param callback
 * @return
 */

BufferSender$ BufferSender::create(
        uint32_t id,
        Packet::StreamType type,
        Buffer$ buffer,
        StreamSenderCallback callback)
{
    BufferSender$ sender(new BufferSender(id, type, buffer, callback));

    if (!sender->init()) {

        return nullptr;
    }

    return sender;
}

/**
 * @brief BufferSender::BufferSender
 * @param id
 * @param type
 * @param buffer
 * @param callback
 */

BufferSender::BufferSender(
        uint32_t id,
        Packet::StreamType type,
        Buffer$ buffer,
        StreamSenderCallback callback)
    : StreamSender(id, type, 0, callback),
      m_buffer(buffer)
{

}

/**
 * @brief BufferSender::init
 * @return
 */

bool BufferSender::init()
{
    if (!m_buffer) {

        return false;
    }

    if (!StreamSender::init(m_buffer->size())) {

        return false;
    }

    return true;
}

/**
 * @brief BufferSender::preparePacket
 * @param pkt
 * @param bytesToSend
 * @param bytesSent
 * @return
 */

bool BufferSender::preparePacket(Packet$ &pkt, uint32_t bytesToSend, uint32_t bytesSent)
{
    if (!StreamSender::preparePacket(pkt, bytesToSend, bytesSent)) {

        return false;
    }

    uint32_t bytesRead = 0;

    if (!m_buffer->read(m_body->data(), bytesToSend, bytesSent, &bytesRead)) {

        return false;
    }

    if (bytesRead) {

        pkt->setBody(m_body, bytesRead);
    }
    else {

        pkt.reset();
    }

    return true;
}

// ============================================================ //

}
