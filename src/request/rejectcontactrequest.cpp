
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

#include "Zway/request/rejectcontactrequest.h"
#include "Zway/request/requestevent.h"
#include "Zway/store.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

/**
 * @brief RejectContactRequest::create
 * @param client
 * @param args
 * @param callback
 * @return
 */

RejectContactRequest$ RejectContactRequest::create(
        Client$ client,
        const UBJ::Object &args,
        RequestCallback callback)
{
    RejectContactRequest$ request(new RejectContactRequest(client, args, callback));

    if (!request->init()) {

        return nullptr;
    }

    return request;
}

/**
 * @brief RejectContactRequest::RejectContactRequest
 * @param client
 * @param args
 * @param callback
 */

RejectContactRequest::RejectContactRequest(
        Client$ client,
        const UBJ::Object &args,
        RequestCallback callback)
    : Request(RejectContact, args, DEFAULT_TIMEOUT, callback),
      m_client(client)
{

}

/**
 * @brief RejectContactRequest::init
 * @return
 */

bool RejectContactRequest::init()
{
    if (!m_client) {

        return false;
    }

    if (m_client->status() < Client::Authenticated) {

        return false;
    }

    m_head["contactRequestId"] = m_args["contactRequestId"];

    return true;
}

/**
 * @brief RejectContactRequest::processResponse
 * @param head
 * @return
 */

bool RejectContactRequest::processResponse(const UBJ::Object &response)
{
    uint32_t status = response["status"].toInt();

    if (status == 1) {

        // delete request

        m_client->store()->remove("contact_requests", UBJ_OBJ("id" << m_head["contactRequestId"].toInt()));

        // raise event

        m_client->postEvent(RequestEvent::create(
                Event::Undefined,
                shared_from_this(),
                response));
    }
    else
    if (status == 0) {

        // delete request

        m_client->store()->remove("contact_requests", UBJ_OBJ("id" << m_head["contactRequestId"].toInt()));

        // raise event

        m_client->postEvent(RequestEvent::create(
                Event::Undefined,
                shared_from_this(),
                response,
                ERROR_UBJ(response["message"])));
    }

    return true;
}

// ============================================================ //

}


