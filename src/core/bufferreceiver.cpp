
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

#include "Zway/core/bufferreceiver.h"
#include "Zway/core/memorybuffer.h"

namespace Zway {

// ============================================================ //

/**
 * @brief BufferReceiver::create
 * @param pkt
 * @param buffer
 * @param callback
 * @return
 */

BufferReceiver$ BufferReceiver::create(
        const Packet &pkt,
        MemoryBuffer$ buffer,
        BufferReceiverCallback callback)
{
    BufferReceiver$ receiver(new BufferReceiver(callback));

    if (!receiver->init(pkt, buffer)) {

        return nullptr;
    }

    return receiver;
}

/**
 * @brief BufferReceiver::BufferReceiver
 * @param callback
 */

BufferReceiver::BufferReceiver(BufferReceiverCallback callback)
    : StreamReceiver(),
      m_callback(callback),
      m_bytesReceived(0)
{

}

/**
 * @brief BufferReceiver::init
 * @param pkt
 * @param buffer
 * @return
 */

bool BufferReceiver::init(const Packet &pkt, MemoryBuffer$ buffer)
{
    if (!StreamReceiver::init(pkt)) {

        return false;
    }

    m_buffer = buffer ? buffer : MemoryBuffer::create(nullptr, pkt.parts() * MAX_PACKET_BODY);

    if (!m_buffer) {

        return false;
    }

    return true;
}

/**
 * @brief BufferReceiver::processPacket
 * @param pkt
 * @return
 */

bool BufferReceiver::processPacket(Packet &pkt)
{
    if (!StreamReceiver::processPacket(pkt)) {

        return false;
    }

    if (!m_buffer->write(pkt.bodyData(), pkt.bodySize(), (pkt.part()-1) * MAX_PACKET_BODY, nullptr)) {

        return false;
    }

    m_bytesReceived += pkt.bodySize();

    return true;
}

/**
 * @brief BufferReceiver::buffer
 * @return
 */

MemoryBuffer$ BufferReceiver::buffer()
{
    return m_buffer;
}

/**
 * @brief BufferReceiver::bytesReceived
 * @return
 */

uint32_t BufferReceiver::bytesReceived()
{
    return m_bytesReceived;
}

/**
 * @brief BufferReceiver::invokeCallback
 */

void BufferReceiver::invokeCallback()
{
    if (m_callback) {

        m_callback(std::dynamic_pointer_cast<BufferReceiver>(shared_from_this()), m_buffer, m_bytesReceived);
    }
}

// ============================================================ //

}
