
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

#include "Zway/core/packet.h"

#include <cstring>

namespace Zway {

const uint32_t MAX_PACKET_BODY = 65536;

// ============================================================ //

/**
 * @brief Packet::create
 * @param id
 * @return
 */

PACKET Packet::create(uint32_t id)
{
    return PACKET(new Packet(id));
}

/**
 * @brief Packet::Packet
 * @param id
 */

Packet::Packet(uint32_t id)
{
    memset(&m_head, 0, sizeof(m_head));

    m_head.id = id;
}

/**
 * @brief Packet::head
 * @return
 */

Packet::Head &Packet::head()
{
    return m_head;
}

/**
 * @brief Packet::id
 * @return
 */

uint32_t Packet::id() const
{
    return m_head.id;
}

/**
 * @brief Packet::streamId
 * @return
 */

uint32_t Packet::streamId() const
{
    return m_head.streamId;
}

/**
 * @brief Packet::streamType
 * @return
 */

Packet::StreamType Packet::streamType() const
{
    return (StreamType)m_head.streamType;
}

/**
 * @brief Packet::part
 * @return
 */

uint32_t Packet::part() const
{
    return m_head.part;
}

/**
 * @brief Packet::parts
 * @return
 */

uint32_t Packet::parts() const
{
    return m_head.parts;
}

/**
 * @brief Packet::bodySize
 * @return
 */

uint32_t Packet::bodySize() const
{
    return m_head.bodySize;
}

/**
 * @brief Packet::bodyData
 * @return
 */

uint8_t* Packet::bodyData()
{
    return m_body->data();
}

/**
 * @brief Packet::body
 * @return
 */

BUFFER Packet::body()
{
    return m_body;
}

/**
 * @brief Packet::setId
 * @param id
 */

void Packet::setId(uint32_t id)
{
    m_head.id = id;
}

/**
 * @brief Packet::setStreamId
 * @param id
 */

void Packet::setStreamId(uint32_t id)
{
    m_head.streamId = id;
}

/**
 * @brief Packet::setStreamType
 * @param type
 */

void Packet::setStreamType(StreamType type)
{
    m_head.streamType = type;
}

/**
 * @brief Packet::setPart
 * @param part
 */

void Packet::setPart(uint32_t part)
{
    m_head.part = part;
}

/**
 * @brief Packet::setParts
 * @param parts
 */

void Packet::setParts(uint32_t parts)
{
    m_head.parts = parts;
}

/**
 * @brief Packet::setBodySize
 * @param size
 */

void Packet::setBodySize(uint32_t size)
{
    m_head.bodySize = size;
}

/**
 * @brief Packet::setBody
 * @param body
 * @param size
 */

void Packet::setBody(BUFFER body, uint32_t size)
{
    if (body) {

        m_head.bodySize = size ? size : body->size();
    }

    m_body = body;
}

// ============================================================ //

}
