
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

#ifndef ZWAY_CORE_PACKET_H_
#define ZWAY_CORE_PACKET_H_

#include "Zway/core/buffer.h"

namespace Zway {

extern const uint32_t MAX_PACKET_HEAD;
extern const uint32_t MAX_PACKET_BODY;

// ============================================================ //

/**
 * @brief The Packet class
 */

class Packet
{
public:

    enum StreamType {

        Undefined,

        Request,

        Resource
    };

    struct Head
    {
        uint32_t id;

        uint32_t streamId;

        uint32_t streamType;

        uint32_t part;

        uint32_t parts;

        uint32_t bodySize;
    };

    static PACKET create(uint32_t id = 0);

    Packet(uint32_t id = 0);

    Head &head();

    uint32_t id() const;

    uint32_t streamId() const;

    StreamType streamType() const;

    uint32_t part() const;

    uint32_t parts() const;

    uint32_t bodySize() const;

    uint8_t *bodyData();

    BUFFER body();

    void setId(uint32_t id);

    void setStreamId(uint32_t id);

    void setStreamType(StreamType type);

    void setPart(uint32_t part);

    void setParts(uint32_t parts);

    void setBodySize(uint32_t size);

    void setBody(BUFFER body, uint32_t size = 0);

protected:

    Head m_head;

    BUFFER m_body;
};

// ============================================================ //

}

#endif
