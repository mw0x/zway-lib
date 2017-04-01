
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

#ifndef ZWAY_CORE_BUFFER_RECEIVER_H_
#define ZWAY_CORE_BUFFER_RECEIVER_H_

#include "Zway/core/streamreceiver.h"

namespace Zway {

// ============================================================ //

/**
 * @brief The BufferReceiver class
 */

class BufferReceiver : public StreamReceiver
{
public:

    static BUFFER_RECEIVER create(
            const Packet &pkt,
            BUFFER buffer,
            BUFFER_RECEIVER_CALLBACK callback = nullptr);

    BUFFER buffer();

    uint32_t bytesReceived();

protected:

    BufferReceiver(BUFFER_RECEIVER_CALLBACK callback = nullptr);

    bool init(const Packet &pkt, BUFFER buffer = nullptr);

    bool processPacket(Packet &pkt);

    void invokeCallback();

protected:

    BUFFER_RECEIVER_CALLBACK m_callback;

    Zway::BUFFER m_buffer;

    uint32_t m_bytesReceived;

};

// ============================================================ //

}

#endif
