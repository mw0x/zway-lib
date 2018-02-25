
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

#ifndef ZWAY_CORE_REQUEST_H_
#define ZWAY_CORE_REQUEST_H_

#include "Zway/ubj/value.h"

namespace Zway {

USING_SHARED_PTR(Request)
USING_SHARED_PTR(RequestEvent)
USING_SHARED_PTR(StreamSender)

using RequestCallback = std::function<void (RequestEvent$, Request$)>;

extern const uint32_t DEFAULT_TIMEOUT;

// ============================================================ //

/**
 * @brief The Request class
 */

class Request : public std::enable_shared_from_this<Request>
{
public:

    enum Type
    {
        Dispatch = 1100,


        CreateAccount = 2100,

        Login = 2200,

        Logout = 2300,

        Config = 2400,


        AddContact = 3100,

        CreateAddCode = 3200,

        FindContact = 3300,

        AcceptContact = 3400,

        RejectContact = 3500,

        ContactStatus = 3600,


        Push = 4100,

        Pull = 4200
    };

    enum Status
    {
        Outgoing,

        WaitingForResponse,

        Completed,

        Timeout,

        Error
    };

    static Request$ create(Type type, UBJ::Object &args, uint32_t timeout = DEFAULT_TIMEOUT, RequestCallback callback = nullptr);

    virtual ~Request();

    StreamSender$ start();

    virtual bool processResponse(const UBJ::Object &response);

    void setId(uint32_t id);

    void setStatus(Status status);

    bool checkTimeout(uint64_t time);

    uint32_t id();

    Type type();

    Status status();

    void invokeCallback(RequestEvent$ event);

protected:

    Request(Type type, const UBJ::Object &args, uint32_t timeout = DEFAULT_TIMEOUT, RequestCallback callback = nullptr);

protected:

    Type m_type;

    Status m_status;

    uint32_t m_id;

    uint32_t m_timeout;

    uint64_t m_time;

    UBJ::Object m_args;

    UBJ::Object m_head;

    RequestCallback m_callback;
};

// ============================================================ //

}

#endif
