
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

#include "Zway/request/pushrequest.h"
#include "Zway/request/requestevent.h"
#include "Zway/message/resourcesender.h"
#include "Zway/message/resource.h"
#include "Zway/core/memorybuffer.h"
#include "Zway/core/crypto/crypto.h"
#include "Zway/core/crypto/random.h"
#include "Zway/core/crypto/rsa.h"
#include "Zway/store/store.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

/**
 * @brief PushRequest::resourceSalt
 * @param salt
 * @param resourceId
 * @return
 */

MemoryBuffer$ PushRequest::resourceSalt(MemoryBuffer$ salt, uint32_t resourceId)
{
    if (!salt) {

        return nullptr;
    }

    MemoryBuffer$ res = salt->copy();

    if (!res) {

        return nullptr;
    }

    uint32_t *p3 = (uint32_t*)(res->data() + 8);

    uint32_t *p4 = (uint32_t*)(res->data() + 12);

    (*p3) = resourceId;

    (*p4) = 0;

    return res;
}

/**
 * @brief PushRequest::create
 * @param client
 * @param msg
 * @param id
 * @param callback
 * @return
 */

PushRequest$ PushRequest::create(Client$ client, Message$ msg, uint32_t id, RequestCallback callback)
{
    PushRequest$ request(new PushRequest(client, msg, id, callback));

    if (!request->init()) {

        return nullptr;
    }

    return request;
}

/**
 * @brief PushRequest::PushRequest
 * @param client
 * @param msg
 * @param id
 * @param callback
 */

PushRequest::PushRequest(Client$ client, Message$ msg, uint32_t id, RequestCallback callback)
    : Request(Push, UBJ_OBJ("requestId" << id), DEFAULT_TIMEOUT, callback),
      m_client(client),
      m_msg(msg)
{

}

/**
 * @brief PushRequest::init
 * @return
 */

bool PushRequest::init()
{
    if (!m_msg) {

        return false;
    }

    // create message id

    if (!m_id) {

        m_id = Crypto::mkId();
    }

    // create message key

    m_key = MemoryBuffer::create(nullptr, 32);

    if (!m_key) {

        return false;
    }

    if (!Crypto::Random::random(m_key->data(), m_key->size(), Crypto::Random::Strong)) {

        return false;
    }

    // create salt

    m_salt = MemoryBuffer::create(nullptr, 16);

    if (!m_salt) {

        return false;
    }

    if (!Crypto::Random::random(m_salt->data(), 12, Crypto::Random::Strong)) {

        return false;
    }

    // prepare keys

    UBJ::Array keys;

    // encrypt message key with own public key

    /*
    BUFFER keyEnc = Crypto::RSA::encrypt(m_client->store()->publicKey(), m_key);

    if (!keyEnc) {

        return false;
    }

    keys << UBJ_OBJ("dst" << m_client->store()->accountId() << "key" << keyEnc);
    */

    // encrypt message key with contact public keys

    UBJ::Array dsts = UBJ_ARR(m_msg->dst());

    for (auto &v : dsts) {

        uint32_t dst = v.toInt();

        // get contact public key

        UBJ::Object contact;

        if (m_client->store()->query("contacts", UBJ_OBJ("id" << dst), &contact)) {

            if (!contact.hasField("publicKey")) {

                return false;
            }

            // encrypt message key

            MemoryBuffer$ keyEnc = Crypto::RSA::encrypt(contact["publicKey"], m_key);

            if (!keyEnc) {

                return false;
            }

            keys << UBJ_OBJ("dst" << dst << "key" << keyEnc);
        }
    }

    UBJ::Object meta;

    if (m_msg && m_msg->hasField("meta")) {

        meta = (*m_msg)["meta"];
    }

    // process resources

    UBJ::Array resources;

    UBJ::Array metaResources;

    for (uint32_t i=0; i<m_msg->numResources(); ++i) {

        Resource$ res = m_msg->resource(i);

        // create resource id

        if (!res->id()) {

            res->setId(Crypto::mkId());
        }

        if (res->size() > 0) {

            resources <<
                    UBJ_OBJ(
                        "id"    << res->id() <<
                        "parts" << res->parts());

            metaResources <<
                    UBJ_OBJ(
                        "id"    << res->id() <<
                        "name"  << res->name() <<
                        "size"  << res->size() <<
                        "hash"  << res->hash());
        }
    }

    m_head["resources"] = resources;

    meta["resources"] = metaResources;

    // encrypt meta data

    MemoryBuffer$ metaBuf = UBJ::Value::write(meta);

    if (!metaBuf) {

        return false;
    }

    Crypto::AES aes;

    aes.setKey(m_key);

    aes.setCtr(m_salt);

    if (!aes.encrypt(metaBuf, metaBuf, metaBuf->size())) {

        return false;
    }

    m_head["meta"] = metaBuf;

    m_head["salt"] = m_salt;

    m_head["keys"] = keys;


    m_msg->setId(m_id);

    m_msg->setStatus(Message::Outgoing);

    // create request record

    // ...

    // create message record

    if (!m_client->store()->insert(
                "messages",
                UBJ_OBJ(
                    "id"      << m_id <<
                    "src"     << m_msg->src() <<
                    "dst"     << m_msg->dst() <<
                    "history" << m_msg->history() <<
                    "status"  << m_msg->status() <<
                    "text"    << m_msg->text()))) {

        return false;
    }

    return true;
}

/**
 * @brief PushRequest::pushResources
 * @param ids
 * @param callback
 * @param index
 */

void PushRequest::pushResources(PushRequest$ request, const UBJ::Array &ids, const std::function<void (PushRequest$)> &callback, uint32_t index)
{
    if (ids.empty()) {

        // no resources to push

        callback(request);

        return;
    }

    uint32_t id = ids[index].toInt();

    Resource$ res = request->message()->resourceById(id);

    if (res) {

        MemoryBuffer$ salt = resourceSalt(request->m_salt, id);

        auto sender = ResourceSender::create(
                    res, request->m_key, salt,
                    [request, ids, id, callback, index] (StreamSender$ sender) {

            if (sender->status() == StreamSender::Completed) {

                // update resource status

                if (!request->m_client->store()->update(
                            "resources",
                            UBJ_OBJ("status" << Resource::Sent << "time" << (uint64_t)time(nullptr)),
                            UBJ_OBJ("id" << id))) {

                    // ...
                }


                UBJ::Object resource;

                if (!request->m_client->store()->query(
                            "resources",
                            UBJ_OBJ("id" << id),
                            &resource)) {

                    // ...
                }

                UBJ::Object message;

                if (!request->m_client->store()->query(
                            "messages",
                            UBJ_OBJ("id" << request->m_id),
                            &message)) {

                    // ...
                }

                // raise event

                request->m_client->postEvent(Event::create(Event::ResourceSent, UBJ_OBJ(
                                                      "message" << message << "resource" << resource)));


                // next resource

                if (index + 1 < ids.size()) {

                    pushResources(request, ids, callback, index + 1);
                }
                else {

                    callback(request);
                }
            }
        });

        if (!sender) {

            return;
        }

        // update resource status

        if (!request->m_client->store()->update(
                    "resources",
                    UBJ_OBJ("status" << Resource::Outgoing),
                    UBJ_OBJ("id" << id))) {

            // ...
        }

        UBJ::Object resource;

        if (!request->m_client->store()->query(
                    "resources",
                    UBJ_OBJ("id" << id),
                    &resource)) {

            // ...
        }

        UBJ::Object message;

        if (!request->m_client->store()->query(
                    "messages",
                    UBJ_OBJ("id" << request->m_id),
                    &message)) {

            // ...
        }

        // raise event

        request->m_client->postEvent(Event::create(Event::ResourceOutgoing, UBJ_OBJ(
                                              "message" << message << "resource" << resource)));

        // send resource

        if (!request->m_client->addStreamSender(sender)) {

            // ...
        }
    }
}

/**
 * @brief PushRequest::updateMessage
 * @param status
 * @param message
 * @return
 */

bool PushRequest::updateMessage(Message::Status status, UBJ::Object *message)
{
    if (m_client->store()->update(
                "messages",
                UBJ_OBJ("status" << status << "time" << (uint64_t)time(nullptr)),
                UBJ_OBJ("id" << m_id)) <= 0) {

        return false;
    }

    if (message) {

        if (!m_client->store()->query("messages", UBJ_OBJ("id" << m_id), message)) {

            return false;
        }
    }

    return true;
}

/**
 * @brief PushRequest::processResponse
 * @param head
 * @return
 */

bool PushRequest::processResponse(const UBJ::Object &response)
{
    uint32_t status = response["status"].toInt();

    if (status == 1) {

        UBJ::Array resources(response["resources"]);

        // process resources

        if (!resources.empty()) {

            // create temporary records

            for (auto &it : resources) {

                Resource$ resource = m_msg->resourceById(it.toInt());

                std::string name = resource->name();

                if (resource->type() == Resource::FileSystem) {

                    name = (*resource)["path"].toStr();
                }

                if (!m_client->store()->insert(
                            "resources",
                            UBJ_OBJ(
                                "id"      << resource->id() <<
                                "type"    << resource->type() <<
                                "request" << m_id <<
                                "status"  << Resource::Outgoing <<
                                "name"    << name))) {

                    // ...
                }
            }


            pushResources(
                        std::dynamic_pointer_cast<PushRequest>(shared_from_this()),
                        response["resources"],
                        [response, resources] (PushRequest$ request) {

                uint32_t numResources = request->m_client->store()->count(
                            "resources",
                            UBJ_OBJ("request" << request->m_id));

                uint32_t numCompleted = request->m_client->store()->count(
                            "resources",
                            UBJ_OBJ("request" << request->m_id << "status" << Resource::Sent));

                UBJ::Object message;

                if (numCompleted == numResources) {

                    request->updateMessage(Message::Sent, &message);
                }

                // request event

                request->m_client->postEvent(RequestEvent::create(
                        Event::Undefined,
                        request,
                        response));

                // message event

                if (numCompleted == numResources) {

                    request->m_client->postEvent(Event::create(Event::MessageSent, UBJ_OBJ("message" << message)));
                }
            });


        }
        else {

            UBJ::Object message;

            updateMessage(Message::Delivered, &message);

            m_client->postEvent(RequestEvent::create(
                    Event::Undefined,
                    shared_from_this(),
                    response));

            m_client->postEvent(Event::create(Event::MessageSent, UBJ_OBJ("message" << message)));

            m_client->postEvent(Event::create(Event::MessageDelivered, UBJ_OBJ("message" << message)));
        }
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

/**
 * @brief PushRequest::message
 * @return
 */

Message$ PushRequest::message()
{
    return m_msg;
}

// ============================================================ //

}
