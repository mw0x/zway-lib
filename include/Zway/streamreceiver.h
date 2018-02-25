
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

#ifndef ZWAY_CORE_STREAM_RECEIVER_H_
#define ZWAY_CORE_STREAM_RECEIVER_H_

#include "Zway/packet.h"

namespace Zway {

USING_SHARED_PTR(StreamReceiver)

// ============================================================ //

/**
 * @brief The StreamReceiver class
 */

class StreamReceiver : public std::enable_shared_from_this<StreamReceiver>
{
public:

    enum Status
    {
        Idle,

        Incoming,

        Completed,

        Error
    };

    virtual ~StreamReceiver();

    virtual bool process(Packet &pkt);

    uint32_t id();

    Packet::StreamType type();

    Status status();

    uint32_t parts();

protected:

    StreamReceiver();

    bool init(const Packet &pkt);

    virtual bool processPacket(Packet &pkt);

    virtual void invokeCallback();

protected:

    uint32_t m_id;

    Packet::StreamType m_type;

    Status m_status;

    uint32_t m_part;

    uint32_t m_parts;
};

// ============================================================ //

}

#endif
