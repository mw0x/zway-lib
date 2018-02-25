
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

#include "Zway/request/createaccountrequest.h"
#include "Zway/request/requestevent.h"
#include "Zway/core/crypto/digest.h"
#include "Zway/core/crypto/random.h"
#include "Zway/core/crypto/rsa.h"
#include "Zway/core/memorybuffer.h"
#include "Zway/store/store.h"
#include "Zway/client.h"


namespace Zway {

// ============================================================ //

/**
 * @brief CreateAccountRequest::create
 * @param client
 * @param args
 * @param callback
 * @return
 */

CreateAccountRequest$ CreateAccountRequest::create(
        Client$ client,
        const UBJ::Object &args,
        RequestCallback callback)
{
    CreateAccountRequest$ request(new CreateAccountRequest(client, args, callback));

    if (!request->init()) {

        return nullptr;
    }

    return request;
}

/**
 * @brief CreateAccountRequest::CreateAccountRequest
 * @param client
 * @param args
 * @param callback
 */

CreateAccountRequest::CreateAccountRequest(
        Client$ client,
        const UBJ::Object &args,
        RequestCallback callback)
    : Request(CreateAccount, args, DEFAULT_TIMEOUT, callback),
      m_client(client)
{

}

/**
 * @brief CreateAccountRequest::init
 * @return
 */

bool CreateAccountRequest::init()
{
    if (!m_client) {

        return false;
    }

    if (m_client->status() > Client::Secure) {

        return false;
    }

    // TODO input validation

    m_storePassword = m_args["password"].toStr();

    // create password

    MemoryBuffer$ password = MemoryBuffer::create(nullptr, 32);

    if (!Crypto::Random::random(password->data(), password->size(), Crypto::Random::Strong)) {

        setStatus(Error);

        return false;
    }

    // create rsa key pair

    if (!Crypto::RSA::createKeyPair(m_publicKey, m_privateKey, 2048)) {

        setStatus(Error);

        return false;

    }

    // set request

    m_head["name"] = m_args["name"];

    m_head["password"] = password;

    m_head["phone"] = m_args["phone"];

    m_head["findByName"] = m_args["findByName"];

    m_head["findByPhone"] = m_args["findByPhone"];

    return true;
}

/**
 * @brief CreateAccountRequest::processResponse
 * @param head
 * @return
 */

bool CreateAccountRequest::processResponse(const UBJ::Object &response)
{
    uint32_t status = response["status"].toInt();

    if (status == 1) {

        UBJ::Object data;

        data["accountId"] = response["accountId"];

        data["accountName"] = m_head["name"];

        data["accountPassword"] = m_head["password"];

        data["publicKey"] = m_publicKey;

        data["privateKey"] = m_privateKey;


        m_storeFilename = m_args["storeDir"].toStr() + "/" + Crypto::Digest::digestHexStr(m_head["name"].buffer()) + ".store";

        Store$ store = Store::create(m_storeFilename, m_storePassword, data);

        if (store) {

            // set initial config

            store->updateBlobData(
                        "blob1",
                        Store::ConfigNodeId,
                        UBJ_OBJ(
                            "findByName"   << m_head["findByName"] <<
                            "findByPhone"  << m_head["findByPhone"] <<
                            "notifyStatus" << true));

            store->close();

            m_client->postEvent(RequestEvent::create(
                    Event::Undefined,
                    shared_from_this(),
                    UBJ_OBJ(
                        "storeFilename" << m_storeFilename <<
                        "storePassword" << m_storePassword)));
        }
        else {

            m_client->postEvent(RequestEvent::create(
                    Event::Undefined,
                    shared_from_this(),
                    UBJ::Object(),
                    ERROR_UBJ("failed to create store")));
        }
	}
	else
    if (status == 0) {

        m_client->postEvent(RequestEvent::create(
                Event::Undefined,
                shared_from_this(),
                UBJ::Object(),
                ERROR_UBJ(response["message"])));
    }

    return true;
}

/**
 * @brief CreateAccountRequest::storeFilename
 * @return
 */

std::string CreateAccountRequest::storeFilename()
{
    return m_storeFilename;
}

// ============================================================ //

}
