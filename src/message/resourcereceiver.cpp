
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

#include "Zway/message/resourcereceiver.h"

namespace Zway {

// ============================================================ //

/**
 * @brief ResourceReceiver::create
 * @param pkt
 * @param key
 * @param salt
 * @param callback
 * @return
 */

ResourceReceiver$ ResourceReceiver::create(
        const Packet &pkt,
        MemoryBuffer$ key,
        MemoryBuffer$ salt,
        BufferReceiverCallback callback)
{
    ResourceReceiver$ receiver(new ResourceReceiver(callback));

    if (!receiver->init(pkt, key, salt)) {

        return nullptr;
    }

    return receiver;
}

/**
 * @brief ResourceReceiver::ResourceReceiver
 * @param callback
 */

ResourceReceiver::ResourceReceiver(BufferReceiverCallback callback)
    : BufferReceiver(callback)
{

}

/**
 * @brief ResourceReceiver::init
 * @param pkt
 * @param key
 * @param salt
 * @return
 */

bool ResourceReceiver::init(const Packet &pkt, MemoryBuffer$ key, MemoryBuffer$ salt)
{
    if (!(key && salt)) {

        return false;
    }

    if (!BufferReceiver::init(pkt)) {

        return false;
    }

    m_aes.setKey(key);

    m_aes.setCtr(salt);

    return true;
}

/**
 * @brief ResourceReceiver::processPacket
 * @param pkt
 * @return
 */

bool ResourceReceiver::processPacket(Packet &pkt)
{
    if (pkt.bodySize()) {

        if (!m_aes.decrypt(pkt.body(), pkt.body(), pkt.bodySize())) {

            return false;
        }
    }

    if (!BufferReceiver::processPacket(pkt)) {

        return false;
    }

    return true;
}

// ============================================================ //

}
