
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

#include "Zway/request/pullrequest.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

/**
 * @brief PullRequest::create
 * @param client
 * @param messageId
 * @param resourceId
 * @param callback
 * @return
 */

PullRequest$ PullRequest::create(Client *client, uint32_t messageId, uint32_t resourceId, RequestCallback callback)
{
    return PullRequest$(new PullRequest(client, messageId, resourceId, callback));
}

/**
 * @brief PullRequest::PullRequest
 * @param client
 * @param messageId
 * @param resourceId
 * @param callback
 */

PullRequest::PullRequest(Client *client, uint32_t messageId, uint32_t resourceId, RequestCallback callback)
    : Request(Pull, {}, DEFAULT_TIMEOUT, callback)
{
    m_head["messageId"] = messageId;

    m_head["resourceId"] = resourceId;
}

/**
 * @brief PullRequest::processResponse
 * @param head
 * @return
 */

bool PullRequest::processResponse(const UBJ::Object &response)
{
    uint32_t status = response["status"].toInt();

    if (status == 1) {

        // raise event

        /*
        m_client->postEvent(RequestEvent::create(
                Event::LoginSuccess,
                shared_from_this(),
                head,
                UBJ::Object(),
                [this] (EVENT event) {
                    invokeCallback(event);
                }));
                */
    }
    else
    if (status == 0) {

        /*
        m_client->postEvent(RequestEvent::create(
                Event::LoginFailure,
                shared_from_this(),
                UBJ::Object(),
                ERROR_INFO(head["message"]),
                [this] (EVENT event) {
                    invokeCallback(event);
                }));
                */
    }

    return true;
}

// ============================================================ //

}
