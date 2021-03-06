
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

#ifndef ZWAY_CORE_ENGINE_H_
#define ZWAY_CORE_ENGINE_H_

#include "Zway/packet.h"
#include "Zway/request.h"
#include "Zway/thread/safe.h"

namespace Zway {

USING_SHARED_PTR(StreamReceiver)

using StreamReceiverMap = std::map<uint32_t, StreamReceiver$>;

using StreamSenderList = std::list<StreamSender$>;

using RequestMap = std::map<uint32_t, Request$>;

// ============================================================ //

/**
 * @brief The Engine class
 */

class Engine
{
public:

    virtual ~Engine();

    void process();

    void finish();

    virtual bool addStreamSender(StreamSender$ sender);

    bool addUbjSender(uint32_t id, Packet::StreamType type, const UBJ::Value &value);

    bool postRequest(Request$ request);

    bool postRequestSuccess(uint32_t requestId, const UBJ::Object &head = UBJ::Object());

    bool postRequestFailure(uint32_t requestId, uint32_t code = 0, const std::string &msg = std::string());

    bool requestPending(Request::Type type, uint32_t id=0);

    uint32_t numStreamSenders();

protected:

    virtual StreamReceiver$ createStreamReceiver(const Packet &pkt);

    bool processIncomingPacket(Packet &pkt);

    virtual bool processIncomingRequest(const UBJ::Object &request);

    virtual bool processRequestTimeout(Request$ request);

    int32_t processStreamSenders(bool copyBody, std::function<bool (Packet$)> packetCallback);

    void removeStreamSender(StreamSender$ sender);

protected:

    ThreadSafe<StreamReceiverMap> m_streamReceivers;

    ThreadSafe<StreamSenderList> m_streamSenders;

    ThreadSafe<RequestMap> m_requests;
};

// ============================================================ //

}

#endif
