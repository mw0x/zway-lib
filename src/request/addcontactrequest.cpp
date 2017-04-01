
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

#include "Zway/request/addcontactrequest.h"
#include "Zway/store/store.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

/**
 * @brief AddContactRequest::create
 * @param client
 * @param args
 * @param callback
 * @return
 */

ADD_CONTACT_REQUEST AddContactRequest::create(
        CLIENT client,
        const UBJ::Object &args,
        REQUEST_CALLBACK callback)
{
    ADD_CONTACT_REQUEST request(new AddContactRequest(client, args, callback));

    if (!request->init()) {

        return nullptr;
    }

    return request;
}

/**
 * @brief AddContactRequest::AddContactRequest
 * @param client
 * @param args
 * @param callback
 */

AddContactRequest::AddContactRequest(
        CLIENT client,
        const UBJ::Object &args,
        REQUEST_CALLBACK callback)
    : Request(AddContact, args, DEFAULT_TIMEOUT, callback),
      m_client(client)
{

}

/**
 * @brief AddContactRequest::init
 * @return
 */

bool AddContactRequest::init()
{
    if (!m_client) {

        return false;
    }

    if (m_client->status() < Client::Authenticated) {

        return false;
    }

    m_head["addCode"] = m_args["addCode"];

    m_head["name"] = m_args["name"];

    m_head["phone"] = m_args["phone"];

    UBJ::Object data;

    if (m_client->store()->getBlobData("blob1", Store::DataNodeId, data)) {

        m_head["publicKey"] = data["publicKey"];
    }
    else {

        return false;
    }

    return true;
}

/**
 * @brief AddContactRequest::processResponse
 * @param head
 * @return
 */

bool AddContactRequest::processResponse(const UBJ::Object &response)
{
    uint32_t status = response["status"].toInt();

    if (status == 1) {

        if (!m_client->store()->addContactRequest(
                    UBJ_OBJ(
                        "id"      << response["requestId"] <<
                        "time"    << (uint64_t)time(nullptr) <<
                        "src"     << m_client->store()->accountId() <<
                        "name"    << response["name"] <<
                        "phone"   << response["phone"] <<
                        "addCode" << response["addCode"]))) {

            // ...
        }

        m_client->postEvent(RequestEvent::create(
                Event::Undefined,
                shared_from_this(),
                response,
                UBJ::Object()));
	}
	else
    if (status == 0) {

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
