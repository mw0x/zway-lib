
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

#include "Zway/core/request.h"
#include "Zway/core/ubjsender.h"

namespace Zway {

const uint32_t DEFAULT_TIMEOUT = 20;

// ============================================================ //

/**
 * @brief Request::create
 * @param type
 * @param args
 * @param timeout
 * @return
 */

Request$ Request::create(Request::Type type, UBJ::Object &args, uint32_t timeout, RequestCallback callback)
{
    Request$ request(new Request(type, args, timeout, callback));

    return request;
}

/**
 * @brief Request::Request
 * @param type
 * @param args
 * @param timeout
 * @param callback
 */

Request::Request(Type type, const UBJ::Object &args, uint32_t timeout, RequestCallback callback)
    : m_type(type),
      m_status(Outgoing),
      m_id(args["requestId"].toInt()),
      m_timeout(timeout),
      m_time(0),
      m_args(args),
      m_callback(callback)
{

}

/**
 * @brief Request::~Request
 */

Request::~Request()
{

}

/**
 * @brief Request::start
 * @param time
 * @return
 */

StreamSender$ Request::start()
{
    m_head["requestId"] = m_id;

    m_head["requestType"] = m_type;

    StreamSender$ sender = UbjSender::create(m_id, Packet::Request, m_head, [this] (StreamSender$ sender) {

        if (sender->status() == StreamSender::Completed) {

            setStatus(WaitingForResponse);
        }
    });

    if (!sender) {

        setStatus(Error);

        return nullptr;
    }

    m_time = time(nullptr);

    setStatus(Outgoing);

    return sender;
}

/**
 * @brief Request::processResponse
 * @param response
 * @return
 */

bool Request::processResponse(const UBJ::Object &response)
{
    return true;
}

/**
 * @brief Request::setId
 * @param id
 */

void Request::setId(uint32_t id)
{
    m_head["requestId"] = id;

    m_id = id;
}

/**
 * @brief Request::setStatus
 * @param status
 */

void Request::setStatus(Request::Status status)
{
    m_status = status;
}

/**
 * @brief Request::checkTimeout
 * @param time
 * @return
 */

bool Request::checkTimeout(uint64_t time)
{
    if (m_timeout > 0 && m_time > 0 && time >= m_time + m_timeout) {

        setStatus(Timeout);

        return true;
    }

    return false;
}

/**
 * @brief Request::id
 * @return
 */

uint32_t Request::id()
{
    return m_id;
}

/**
 * @brief Request::type
 * @return
 */

Request::Type Request::type()
{
    return m_type;
}

/**
 * @brief Request::status
 * @return
 */

Request::Status Request::status()
{
    return m_status;
}

/**
 * @brief Request::invokeCallback
 * @param event
 */

void Request::invokeCallback(RequestEvent$ event)
{
    if (m_callback) {

        m_callback(event, shared_from_this());
    }
}

// ============================================================ //

}
