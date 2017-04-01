
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

#include "Zway/message/resourcesender.h"
#include "Zway/message/resource.h"

namespace Zway {

// ============================================================ //

/**
 * @brief ResourceSender::create
 * @param res
 * @param key
 * @param salt
 * @param callback
 * @return
 */

RESOURCE_SENDER ResourceSender::create(
        RESOURCE res,
        BUFFER key,
        BUFFER salt,
        STREAM_SENDER_CALLBACK callback)
{
    RESOURCE_SENDER sender(new ResourceSender(res, callback));

    if (!sender->init(key, salt)) {

        return nullptr;
    }

    return sender;
}

/**
 * @brief ResourceSender::ResourceSender
 * @param res
 * @param callback
 */

ResourceSender::ResourceSender(RESOURCE res, STREAM_SENDER_CALLBACK callback)
    : StreamSender(0, Packet::Resource, 0, callback),
      m_res(res)
{

}

/**
 * @brief ResourceSender::init
 * @param key
 * @param salt
 * @return
 */

bool ResourceSender::init(BUFFER key, BUFFER salt)
{
    if (!m_res) {

        return false;
    }

    if (!m_res->open()) {

        return false;
    }

    if (!(key && salt)) {

        return false;
    }

    if (!StreamSender::init(m_res->size())) {

        return false;
    }

    m_id = m_res->id();

    m_aes.setKey(key);

    m_aes.setCtr(salt);

    return true;
}

/**
 * @brief ResourceSender::preparePacket
 * @param pkt
 * @param bytesToSend
 * @param bytesSent
 * @return
 */

bool ResourceSender::preparePacket(PACKET &pkt, uint32_t bytesToSend, uint32_t bytesSent)
{
    if (!StreamSender::preparePacket(pkt, bytesToSend, bytesSent)) {

        return false;
    }

    // read chunk from resource into body buffer

    if (!m_res->read(m_body, bytesToSend, bytesSent)) {

        return false;
    }

    // set packet body

    pkt->setBody(m_body, bytesToSend);

    return true;
}

/**
 * @brief ResourceSender::processPacket
 * @param pkt
 * @return
 */

bool ResourceSender::processPacket(PACKET &pkt)
{
    if (pkt->bodySize()) {

        if (!m_aes.encrypt(pkt->body(), pkt->body(), pkt->bodySize())) {

            return false;
        }
    }

    return true;
}

/**
 * @brief ResourceSender::invokeCallback
 */

void ResourceSender::invokeCallback()
{
    if (m_callback) {

        m_callback(std::dynamic_pointer_cast<ResourceSender>(shared_from_this()));
    }
}

// ============================================================ //

}
