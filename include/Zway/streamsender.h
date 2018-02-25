
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

#ifndef ZWAY_CORE_STREAM_SENDER_H_
#define ZWAY_CORE_STREAM_SENDER_H_

#include "Zway/packet.h"

namespace Zway {

USING_SHARED_PTR(StreamSender)

using StreamSenderCallback = std::function<void (StreamSender$)>;

// ============================================================ //

/**
 * @brief The StreamSender class
 */

class StreamSender : public std::enable_shared_from_this<StreamSender>
{
public:

    enum Status
    {
        Idle,

        Outgoing,

        Completed,

        Error
    };

    virtual ~StreamSender();

    virtual bool process(Packet$ &pkt);

    uint32_t id();

    Packet::StreamType type();

    Status status();

    uint32_t part();

    uint32_t parts();

protected:

    StreamSender(
            uint32_t id,
            Packet::StreamType type,
            uint32_t parts = 0,
            StreamSenderCallback callback = nullptr);

    bool init(uint32_t streamSize = 0);

    virtual bool preparePacket(Packet$ &pkt, uint32_t bytesToSend, uint32_t bytesSent);

    virtual bool processPacket(Packet$ &pkt);

    virtual void invokeCallback();

protected:

    uint32_t m_id;

    Packet::StreamType m_type;

    Status m_status;

    StreamSenderCallback m_callback;

    uint32_t m_size;

    uint32_t m_part;

    uint32_t m_parts;

    MemoryBuffer$ m_body;
};

// ============================================================ //

}

#endif
