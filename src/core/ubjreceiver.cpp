
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

#include "Zway/core/ubjreceiver.h"
#include "Zway/core/memorybuffer.h"

namespace Zway {

// ============================================================ //

/**
 * @brief UbjReceiver::create
 * @param pkt
 * @param callback
 * @return
 */

UbjReceiver$ UbjReceiver::create(const Packet &pkt, UbjReceiverCallback callback)
{
    UbjReceiver$ receiver(new UbjReceiver(callback));

    if (!receiver->init(pkt)) {

        return nullptr;
    }

    return receiver;
}

/**
 * @brief UbjReceiver::UbjReceiver
 * @param callback
 */

UbjReceiver::UbjReceiver(UbjReceiverCallback callback)
    : BufferReceiver(),
      m_callback(callback)
{

}

/**
 * @brief UbjReceiver::processPacket
 * @param pkt
 * @return
 */

bool UbjReceiver::processPacket(Packet &pkt)
{
    if (!BufferReceiver::processPacket(pkt)) {

        return false;
    }

    if (pkt.part() == pkt.parts()) {

        if (!UBJ::Value::read(m_value, m_buffer->data(), m_bytesReceived)) {

            // ...
        }
    }

    return true;
}

/**
 * @brief UbjReceiver::value
 * @return
 */

UBJ::Value &UbjReceiver::value()
{
    return m_value;
}

/**
 * @brief UbjReceiver::invokeCallback
 */

void UbjReceiver::invokeCallback()
{
    if (m_callback) {

        m_callback(std::dynamic_pointer_cast<UbjReceiver>(shared_from_this()), m_value);
    }
}

// ============================================================ //

}
