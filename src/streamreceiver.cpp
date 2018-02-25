
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

#include "Zway/streamreceiver.h"

namespace Zway {

// ============================================================ //

/**
 * @brief StreamReceiver::StreamReceiver
 */

StreamReceiver::StreamReceiver()
    : m_id(0),
      m_type(Packet::Undefined),
      m_status(Idle),
      m_part(0),
      m_parts(0)
{

}

/**
 * @brief StreamReceiver::init
 * @param pkt
 * @return
 */

bool StreamReceiver::init(const Packet &pkt)
{
    // TODO input validation

    m_id = pkt.streamId();

    m_type = pkt.streamType();

    m_part = pkt.part();

    m_parts = pkt.parts();

    return true;
}

/**
 * @brief StreamReceiver::~StreamReceiver
 */

StreamReceiver::~StreamReceiver()
{

}

/**
 * @brief StreamReceiver::process
 * @param pkt
 * @return
 */

bool StreamReceiver::process(Packet &pkt)
{
    if (pkt.part() > 1 && pkt.part() != m_part+1) {

        // ...

        return false;
    }

    if (!processPacket(pkt)) {

        m_status = Error;

        invokeCallback();

        return false;
    }

    m_part = pkt.part();

    if (m_part == m_parts) {

        m_status = Completed;

        invokeCallback();
    }

    return true;
}

/**
 * @brief StreamReceiver::processPacket
 * @param pkt
 * @return
 */

bool StreamReceiver::processPacket(Packet &pkt)
{
    return true;
}

/**
 * @brief StreamReceiver::invokeCallback
 */

void StreamReceiver::invokeCallback()
{

}

/**
 * @brief StreamReceiver::id
 * @return
 */

uint32_t StreamReceiver::id()
{
    return m_id;
}

/**
 * @brief StreamReceiver::type
 * @return
 */

Packet::StreamType StreamReceiver::type()
{
    return m_type;
}

/**
 * @brief StreamReceiver::status
 * @return
 */

StreamReceiver::Status StreamReceiver::status()
{
    return m_status;
}

/**
 * @brief StreamReceiver::parts
 * @return
 */

uint32_t StreamReceiver::parts()
{
    return m_parts;
}

// ============================================================ //

}
