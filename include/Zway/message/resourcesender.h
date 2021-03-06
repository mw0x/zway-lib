
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

#ifndef RESOURCE_SENDER_H_
#define RESOURCE_SENDER_H_

#include "Zway/ubjsender.h"
#include "Zway/crypto/aes.h"

namespace Zway {

USING_SHARED_PTR(Resource)
USING_SHARED_PTR(ResourceSender)

// ============================================================ //

/**
 * @brief The ResourceSender class
 */

class ResourceSender : public StreamSender
{
public:

    static ResourceSender$ create(
            Resource$ res,
            MemoryBuffer$ key,
            MemoryBuffer$ salt,
            StreamSenderCallback callback = nullptr);

protected:

    ResourceSender(Resource$ res, StreamSenderCallback callback);

    bool init(MemoryBuffer$ key, MemoryBuffer$ salt);

    bool preparePacket(Packet$ &pkt, uint32_t bytesToSend, uint32_t bytesSent);

    bool processPacket(Packet$ &pkt);

    void invokeCallback();

protected:

    Resource$ m_res;

    Crypto::AES m_aes;

};

// ============================================================ //

}

#endif
