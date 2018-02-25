
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

#include "Zway/streamsender.h"
#include "Zway/memorybuffer.h"

namespace Zway {

// ============================================================ //

/**
 * @brief StreamSender::StreamSender
 * @param id
 * @param type
 * @param parts
 * @param callback
 */

StreamSender::StreamSender(
        uint32_t id,
        Packet::StreamType type,
        uint32_t parts,
        StreamSenderCallback callback)
    : m_id(id),
      m_type(type),
      m_status(Idle),
      m_callback(callback),
      m_size(0),
      m_part(0),
      m_parts(parts)
{

}

/**
 * @brief StreamSender::init
 * @param streamSize
 * @return
 */

bool StreamSender::init(uint32_t streamSize)
{
    m_body = MemoryBuffer::create(nullptr, MAX_PACKET_BODY);

    if (!m_body) {

        return false;
    }

    if (streamSize) {

        m_size = streamSize;

        m_parts = 1;

        if (m_size >= MAX_PACKET_BODY) {

            m_parts = m_size / MAX_PACKET_BODY + (streamSize % MAX_PACKET_BODY ? 1 : 0);
        }
    }

    return true;
}

/**
 * @brief StreamSender::~StreamSender
 */

StreamSender::~StreamSender()
{

}

/**
 * @brief StreamSender::process
 * @param pkt
 * @return
 */

bool StreamSender::process(Packet$ &pkt)
{
    uint32_t bytesSent = m_part * MAX_PACKET_BODY;

    uint32_t bytesToSend = m_size > 0 && m_size - bytesSent < MAX_PACKET_BODY ? m_size - bytesSent : MAX_PACKET_BODY;

    m_body->clear();

    // create packet

    pkt = Packet::create();

    // prepare packet

    if (pkt) {

        if (!preparePacket(pkt, bytesToSend, bytesSent)) {

            m_status = Error;

            invokeCallback();

            return false;
        }
    }
    else {

        m_status = Error;

        invokeCallback();

        return false;
    }

    // process packet

    if (pkt) {

        if (!processPacket(pkt)) {

            m_status = Error;

            invokeCallback();

            return false;
        }

        m_part++;
    }

    // TODO make completed decision in subclass

    if (m_parts > 0 && m_part == m_parts) {

        m_status = Completed;

        invokeCallback();
    }

    return true;
}

/**
 * @brief StreamSender::preparePacket
 * @param pkt
 * @return
 */

bool StreamSender::preparePacket(Packet$ &pkt, uint32_t bytesToSend, uint32_t bytesSent)
{
    pkt->setStreamId(m_id);

    pkt->setStreamType(m_type);

    pkt->setPart(m_part+1);

    pkt->setParts(m_parts);

    return true;
}

/**
 * @brief StreamSender::processPacket
 * @param pkt
 * @return
 */

bool StreamSender::processPacket(Packet$ &pkt)
{
    return true;
}

/**
 * @brief StreamSender::invokeCallback
 */

void StreamSender::invokeCallback()
{
    if (m_callback) {

        m_callback(shared_from_this());
    }
}

/**
 * @brief StreamSender::id
 * @return
 */

uint32_t StreamSender::id()
{
    return m_id;
}

/**
 * @brief StreamSender::type
 * @return
 */

Packet::StreamType StreamSender::type()
{
    return m_type;
}

/**
 * @brief StreamSender::status
 * @return
 */

StreamSender::Status StreamSender::status()
{
    return m_status;
}

/**
 * @brief StreamSender::part
 * @return
 */

uint32_t StreamSender::part()
{
    return m_part;
}

/**
 * @brief StreamSender::parts
 * @return
 */

uint32_t StreamSender::parts()
{
    return m_parts;
}

// ============================================================ //

}
