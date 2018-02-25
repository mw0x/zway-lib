
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

#include "Zway/request/loginrequest.h"
#include "Zway/request/requestevent.h"
#include "Zway/store.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

/**
 * @brief LoginRequest::create
 * @param client
 * @param callback
 * @return
 */

LoginRequest$ LoginRequest::create(Client$ client, RequestCallback callback)
{
    LoginRequest$ request(new LoginRequest(client, callback));

    if (!request->init()) {

        return nullptr;
    }

    return request;
}

/**
 * @brief LoginRequest::LoginRequest
 * @param client
 * @param callback
 */

LoginRequest::LoginRequest(Client$ client, RequestCallback callback)
    : Request(Login, {}, DEFAULT_TIMEOUT, callback),
      m_client(client)
{

}

/**
 * @brief LoginRequest::init
 * @return
 */

bool LoginRequest::init()
{
    if (!m_client) {

        return false;
    }

    if (!m_client->store() ||
        m_client->status() < Client::Secure ||
        m_client->status() >= Client::Authenticated) {

        return false;
    }

    m_head["account"] = m_client->store()->accountId();

    m_head["password"] = m_client->store()->accountPassword();

    UBJ::Object config;

    if (!m_client->store()->getConfig(config, true)) {

        return false;
    }

    m_head["config"] = config;

    return true;
}

/**
 * @brief LoginRequest::processResponse
 * @param head
 * @return
 */

bool LoginRequest::processResponse(const UBJ::Object &response)
{
    uint32_t status = response["status"].toInt();

    if (status == 1) {

        // set status

        m_client->setStatus(Client::Authenticated);

        // transfer contact status into map

        {
            MutexLocker lock(m_client->m_contactStatus);

            m_client->m_contactStatus->clear();

            for (auto &it : response["contactStatus"].toArray()) {

                (*m_client->m_contactStatus)[it["contactId"].toInt()] = it["status"].toInt();
            }
        }


        for (auto &it : response["inbox"].toArray()) {

            m_client->store()->updateInbox(it["contactId"].toInt(), it["requestIds"]);
        }


        // raise event

        m_client->postEvent(RequestEvent::create(
                Event::LoginSuccess,
                shared_from_this(),
                response));
    }
    else
    if (status == 0) {

        m_client->postEvent(RequestEvent::create(
                Event::LoginFailure,
                shared_from_this(),
                response,
                ERROR_UBJ(response["message"])));
    }

    return true;
}

// ============================================================ //

}
