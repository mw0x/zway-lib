
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

#include "Zway/core/engine.h"
#include "Zway/core/ubjreceiver.h"

namespace Zway {

// ============================================================ //

/**
 * @brief Engine::~Engine
 */

Engine::~Engine()
{

}

/**
 * @brief Engine::process
 */

void Engine::process()
{
    std::list<REQUEST> remove;

    {
        MutexLocker locker(m_requests);

        if (!m_requests->empty()) {

            uint64_t t = time(nullptr);

            for (auto &it : *m_requests) {

                REQUEST &req = it.second;

                if (req->checkTimeout(t)) {

                    remove.push_back(req);
                }
            }
        }
    }

    for (auto &it : remove) {

        processRequestTimeout(it);
    }

    {
        MutexLocker locker(m_requests);

        for (auto &it : remove) {

            m_requests->erase(it->id());
        }
    }
}

/**
 * @brief Engine::finish
 */

void Engine::finish()
{
    {
        MutexLocker locker(m_requests);

        m_requests->clear();
    }

    // cancel pending stream senders

    {
        MutexLocker locker(m_streamSenders);

        m_streamSenders->clear();
    }

    // cancel pending stream receivers

    {
        MutexLocker locker(m_streamReceivers);

        m_streamReceivers->clear();
    }
}

/**
 * @brief Engine::addStreamSender
 * @param sender
 * @return
 */

bool Engine::addStreamSender(STREAM_SENDER sender)
{
    if (!sender) {

        return false;
    }

    if (!sender->id()) {

        return false;
    }

    MutexLocker locker(m_streamSenders);

    if (m_streamSenders->find(sender->id()) != m_streamSenders->end()) {

        return false;
    }

    (*m_streamSenders)[sender->id()] = sender;

    return true;
}

/**
 * @brief Engine::addUbjSender
 * @param id
 * @param type
 * @param value
 * @return
 */

bool Engine::addUbjSender(uint32_t id, Packet::StreamType type, const UBJ::Value &value)
{
    return addStreamSender(UbjSender::create(id, type, value));
}

/**
 * @brief Engine::postRequest
 * @param request
 * @param time
 * @return
 */

bool Engine::postRequest(REQUEST request)
{
    if (!request) {

        return false;
    }


    if (requestPending(request->type(), request->id())) {

        return false;
    }


    STREAM_SENDER sender = request->start();

    if (!sender) {

        return false;
    }

    if (!addStreamSender(sender)) {

        return false;
    }

    MutexLocker locker(m_requests);

    (*m_requests)[request->id()] = request;

    return true;
}

/**
 * @brief Engine::postRequestSuccess
 * @param requestId
 * @param head
 * @return
 */

bool Engine::postRequestSuccess(uint32_t requestId, const UBJ::Object &head)
{
    UBJ::Object data = head;

    data["requestId"] = requestId;

    data["status"] = 1;

    return addUbjSender(
                requestId,
                Packet::Request,
                data);
}

/**
 * @brief Engine::postRequestFailure
 * @param requestId
 * @param code
 * @param msg
 * @return
 */

bool Engine::postRequestFailure(uint32_t requestId, uint32_t code, const std::string &msg)
{
    return addUbjSender(
                requestId,
                Packet::Request,
                UBJ_OBJ(
                    "requestId" << requestId <<
                    "status"    << 0 <<
                    "code"      << code <<
                    "message"   << msg));
}

/**
 * @brief Engine::requestPending
 * @param type
 * @param id
 * @return
 */

bool Engine::requestPending(Request::Type type, uint32_t id)
{
    MutexLocker lock(m_requests);

    if (id) {

        for (auto &it: *m_requests) {

            if (it.second->type() == type && it.second->id() == id) {

                return true;
            }
        }
    }
    else {

        for (auto &it: *m_requests) {

            if (it.second->type() == type) {

                return true;
            }
        }
    }

    return false;
}

/**
 * @brief Engine::numStreamSenders
 * @return
 */

uint32_t Engine::numStreamSenders()
{
    MutexLocker locker(m_streamSenders);

    return m_streamSenders->size();
}

/**
 * @brief Engine::createStreamReceiver
 * @param pkt
 * @return
 */

STREAM_RECEIVER Engine::createStreamReceiver(const Packet &pkt)
{
    if (pkt.streamType() == Packet::Request) {

        // create request receiver

        return UbjReceiver::create(pkt, [this] (UBJ_RECEIVER receiver, UBJ::Value &data) {

            UBJ::Object head(data);

            if (receiver->status() == StreamReceiver::Completed) {

                REQUEST request;

                if (head.hasField("requestId")) {

                    uint32_t requestId = head["requestId"].toInt();

                    {
                        MutexLocker locker(m_requests);

                        if (m_requests->find(requestId) != m_requests->end()) {

                            request = (*m_requests)[requestId];
                        }
                    }
                }

                // process request

                if (request) {

                    if (!request->processResponse(head)) {

                        // ...
                    }


                    request->setStatus(Request::Completed);


                    MutexLocker lock(m_requests);

                    m_requests->erase(request->id());

                }
                else
                if (head.hasField("requestType")) {

                    // incoming request

                    if (!processIncomingRequest(head)) {

                        // ...
                    }
                }
                else {

                    // ...
                }
            }
        });
    }

    return nullptr;
}

/**
 * @brief Engine::processIncomingPacket
 * @param pkt
 * @return
 */

bool Engine::processIncomingPacket(Packet &pkt)
{
    MutexLocker locker(m_streamReceivers);

    STREAM_RECEIVER receiver;

    if (m_streamReceivers->find(pkt.streamId()) == m_streamReceivers->end()) {

        receiver = createStreamReceiver(pkt);

        if (receiver) {

            (*m_streamReceivers)[pkt.streamId()] = receiver;
        }
        else {

            // ...
        }
    }
    else {

        receiver = (*m_streamReceivers)[pkt.streamId()];
    }

    // process receiver

    if (receiver) {

        if (receiver->process(pkt)) {

            if (receiver->status() == StreamReceiver::Completed) {

                m_streamReceivers->erase(receiver->id());

                // ...
            }

            return true;
        }
        else {

            m_streamReceivers->erase(receiver->id());

            // ...
        }
    }

    return false;
}

/**
 * @brief Engine::processIncomingRequest
 * @param request
 * @return
 */

bool Engine::processIncomingRequest(const UBJ::Object &request)
{
    return false;
}

/**
 * @brief Engine::processRequestTimeout
 * @param request
 * @return
 */

bool Engine::processRequestTimeout(REQUEST request)
{
    return true;
}

/**
 * @brief Engine::processStreamSenders
 * @param numRounds
 * @param copyBody
 * @param packetCallback
 * @return
 */

int32_t Engine::processStreamSenders(bool copyBody, std::function<bool (PACKET)> packetCallback)
{
    int32_t res=0;

    // get snapshot of stream senders in order to prevent
    // deadlocks when callbacks post new ones

    STREAM_SENDER_MAP senders;

    {
        MutexLocker locker(m_streamSenders);

        senders = *m_streamSenders;
    }

    if (senders.empty()) {

        return 0;
    }

    for (auto &it : senders) {

        STREAM_SENDER sender = it.second;

        PACKET pkt;

        if (sender->process(pkt)) {

            if (pkt) {

                if (copyBody) {

                    pkt->setBody(pkt->body()->copy(), pkt->bodySize());
                }

                if (!packetCallback(pkt)) {

                    // ...

                    return 0;
                }

                res++;
            }

            if (sender->status() == StreamSender::Completed) {

                {
                    MutexLocker locker(m_streamSenders);

                    m_streamSenders->erase(sender->id());
                }

                break;
            }
        }
        else {

            {
                MutexLocker locker(m_streamSenders);

                m_streamSenders->erase(sender->id());
            }

            break;
        }
    }

    return res;
}

// ============================================================ //

}
