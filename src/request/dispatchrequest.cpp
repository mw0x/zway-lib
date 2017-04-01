
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

#include "Zway/request/dispatchrequest.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

/**
 * @brief DispatchRequest::create
 * @param client
 * @param args
 * @param callback
 * @return
 */

DISPATCH_REQUEST DispatchRequest::create(
        CLIENT client,
        const UBJ::Object &args,
        REQUEST_CALLBACK callback)
{
    DISPATCH_REQUEST request(new DispatchRequest(client, args, callback));

    if (!request->init()) {

        return nullptr;
    }

    return request;
}

/**
 * @brief DispatchRequest::DispatchRequest
 * @param client
 * @param args
 * @param callback
 */

DispatchRequest::DispatchRequest(
        CLIENT client,
        const UBJ::Object &args,
        REQUEST_CALLBACK callback)
    : Request(Dispatch, args, DEFAULT_TIMEOUT, callback),
      m_client(client)
{

}

/**
 * @brief DispatchRequest::init
 * @return
 */

bool DispatchRequest::init()
{
    if (!m_client) {

        return false;
    }

    if (m_client->status() < Client::Authenticated) {

        return false;
    }

    m_head = m_args;

    return true;
}

/**
 * @brief DispatchRequest::processResponse
 * @param head
 * @return
 */

bool DispatchRequest::processResponse(const UBJ::Object &response)
{
    uint32_t status = response["status"].toInt();

    m_client->postEvent(RequestEvent::create(
            Event::Undefined,
            shared_from_this(),
            response,
            status == 1 ? UBJ::Object() : ERROR_UBJ(response["message"])));

    return true;
}

// ============================================================ //

}


