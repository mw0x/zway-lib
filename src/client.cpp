
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

#include "Zway/crypto/crypto.h"
#include "Zway/crypto/rsa.h"
#include "Zway/memorybuffer.h"
#include "Zway/event/eventhandler.h"
#include "Zway/message/message.h"
#include "Zway/message/resource.h"
#include "Zway/message/resourcereceiver.h"
#include "Zway/request/createaccountrequest.h"
#include "Zway/request/loginrequest.h"
#include "Zway/request/logoutrequest.h"
#include "Zway/request/addcontactrequest.h"
#include "Zway/request/createaddcoderequest.h"
#include "Zway/request/findcontactrequest.h"
#include "Zway/request/acceptcontactrequest.h"
#include "Zway/request/rejectcontactrequest.h"
#include "Zway/request/dispatchrequest.h"
#include "Zway/request/pushrequest.h"
#include "Zway/request/requestevent.h"
#include "Zway/store.h"
#include "Zway/client.h"

#if !defined _WIN32
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#endif

#include <iostream>

#include <gnutls/gnutls.h>

namespace Zway {

const uint16_t ZWAY_PORT = 5557;

const uint32_t RECONNECT_INTERVAL = 30000;

// ============================================================ //

#if defined _WIN32

void Client::initWSA()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
}

// ============================================================ //

void Client::freeWSA()
{
    WSACleanup();
}

#endif

/**
 * @brief Client::startup
 * @param client
 * @param host
 * @param port
 * @return
 */

bool Client::startup()
{
    // init gnutls

    int res = gnutls_global_init();

    if (res < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    return true;
}

/**
 * @brief Client::cleanup
 */

void Client::cleanup()
{
    gnutls_global_deinit();
}

/**
 * @brief Client::create
 * @param handler
 * @param callback
 * @return
 */

Client$ Client::create(EventHandler$ handler, EventHandlerCallback callback)
{
    Client$ client(new Client(handler));

    if (handler && callback) {

        handler->addHandler(client, callback);
    }

    return client;
}

/**
 * @brief Client::Client
 * @param handler
 */

Client::Client(EventHandler$ handler)
    : m_status(Closed),
      m_socket(-1),
      m_session(nullptr),
      m_anonCred(nullptr),
      m_certCred(nullptr),
      m_port(0),
      m_sender(this),
      m_receiver(this),
      m_eventHandler(handler)
{

}

/**
 * @brief Client::~Client
 */

Client::~Client()
{
    close();
}

/**
 * @brief Client::start
 * @param host
 * @param port
 * @return
 */

bool Client::start(const std::string& host, uint16_t port)
{
    if (status() >= Started) {

        return false;
    }

    int res = gnutls_certificate_allocate_credentials((gnutls_certificate_credentials_t*)&m_certCred);

    if (res < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    res = gnutls_anon_allocate_client_credentials((gnutls_anon_client_credentials_t*)&m_anonCred);

    if (res < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    m_host = host;

    m_port = port;

    // run client

    Thread::start();

    // run sender and receiver in paused mode

    m_sender.start(true);

    m_receiver.start(true);

    // set status

    setStatus(Started);

    return true;
}

/**
 * @brief Client::close
 * @return
 */

bool Client::close()
{
    if (status() == Closed) {

        return false;
    }

    // shutdown sender and receiver

    m_receiver.cancelAndJoin();

    m_sender.cancelAndJoin();

    // shutdown engine

    finish();

    // shutdown client

    cancelAndJoin();

    disconnect();

    // release gnutls resources

    if (m_certCred) {

        gnutls_certificate_free_credentials((gnutls_certificate_credentials_t)m_certCred);
    }

    if (m_anonCred) {

        gnutls_anon_free_client_credentials((gnutls_anon_client_credentials_t)m_anonCred);
    }

    m_certCred = nullptr;

    m_anonCred = nullptr;

    m_host.clear();

    m_port = 0;

    setStatus(Closed);

    return true;
}

/**
 * @brief Client::setStore
 * @param store
 */

void Client::setStore(Store$ store)
{
    m_store = store;
}

/**
 * @brief Client::setEventHandler
 * @param handler
 */

void Client::setEventHandler(EventHandler$ handler)
{
    m_eventHandler = handler;
}

/**
 * @brief Client::request
 * @param args
 * @param callback
 * @return
 */

bool Client::request(const UBJ::Object &args, RequestCallback callback)
{
    Request$ request;

    switch (args["requestType"].toInt()) {
    case Request::Dispatch:
        request = DispatchRequest::create(shared_from_this(), args, callback);
        break;
    case Request::CreateAccount:
        request = CreateAccountRequest::create(shared_from_this(), args, callback);
        break;
    case Request::Login:
        request = LoginRequest::create(shared_from_this(), callback);
        break;
    case Request::Logout:
        request = LogoutRequest::create(shared_from_this(), callback);
        break;
    case Request::AddContact:
        request = AddContactRequest::create(shared_from_this(), args, callback);
        break;
    case Request::CreateAddCode:
        request = CreateAddCodeRequest::create(shared_from_this(), args, callback);
        break;
    case Request::FindContact:
        request = FindContactRequest::create(shared_from_this(), args, callback);
        break;
    case Request::AcceptContact:
        request = AcceptContactRequest::create(shared_from_this(), args, callback);
        break;
    case Request::RejectContact:
        request = RejectContactRequest::create(shared_from_this(), args, callback);
        break;
    }

    if (request) {

        return postRequest(request);
    }

    return false;
}

/**
 * @brief Client::postEvent
 * @param event
 * @param immediately
 */

void Client::postEvent(Event$ event, bool immediately)
{
    if (m_eventHandler) {

        m_eventHandler->post(ClientEvent(shared_from_this(), event));
    }
}

/**
 * @brief Client::postRequest
 * @param request
 * @return
 */

bool Client::postRequest(Request$ request)
{
    if (request) {

        if (!request->id()) {

            request->setId(Crypto::mkId());
        }

        return Engine::postRequest(request);
    }

    return false;
}

/**
 * @brief Client::postMessage
 * @param msg
 * @return
 */

bool Client::postMessage(Message$ message)
{
    if (!m_store) {

        return false;
    }

    if (!message) {

        return false;
    }

    message->setHistory(m_store->latestHistory(message->dst()));

    // create request

    auto request = PushRequest::create(shared_from_this(), message, 0,
                [] (RequestEvent$ event, Request$ req) {

                    if (event->error().empty()) {

                        // ...
                    }
                });

    if (!request) {

        return false;
    }

    // post request

    if (!postRequest(request)) {

        return false;
    }

    // raise event

    postEvent(Event::create(Event::MessageOutgoing, UBJ_OBJ("message" << *message)));

    return true;
}

/**
 * @brief Client::addStreamSender
 * @param sender
 * @return
 */

bool Client::addStreamSender(StreamSender$ sender)
{
    if (!Engine::addStreamSender(sender)) {

        return false;
    }

    m_sender.notify();

    return true;
}

/**
 * @brief Client::status
 * @return
 */

Client::Status Client::status()
{
    MutexLocker locker(m_status);

    return m_status;
}

/**
 * @brief Client::store
 * @return
 */

Store$ Client::store()
{
    return m_store;
}

/**
 * @brief Client::eventHandler
 * @return
 */

EventHandler$ Client::eventHandler()
{
    return m_eventHandler;
}

/**
 * @brief Client::contactStatus
 * @param contactId
 * @return
 */

uint32_t Client::contactStatus(uint32_t contactId)
{
    MutexLocker lock(m_contactStatus);

    if (m_contactStatus->find(contactId) != m_contactStatus->end()) {

        return (*m_contactStatus)[contactId];
    }

    return 0;
}

/**
 * @brief Client::setStatus
 * @param status
 */

void Client::setStatus(Status status, bool event)
{
    {
        MutexLocker locker(m_status);

        m_status = status;
    }

    if (event) {

        postEvent(Event::create(Event::Status, UBJ_OBJ("status" << status)));
    }
}

/**
 * @brief Client::run
 */

void Client::run()
{
    for (;;) {

        if (canceled()) {

            break;
        }

        if (status() == Started ||
            status() == Disconnected) {

            reconnect();

            continue;
        }

        Packet pkt;

        if (m_receiver.fetchPacket(pkt)) {

            processIncomingPacket(pkt);
        }
        else {

            m_receiver.waitPacket(1000);
        }

        process();
    }
}

/**
 * @brief Client::connect
 * @param host
 * @param port
 * @return
 */

bool Client::connect(const std::string& host, uint32_t port)
{
    uint32_t a = inet_addr(host.c_str());

    if (a == INADDR_NONE) {

        struct hostent* he = gethostbyname(host.c_str());

        if (!he) {

            //postEvent(MAKE_ERROR(0, "Failed to resolve hostname"));

            return false;
        }

        a = *(uint32_t*)he->h_addr;
    }

#if defined _WIN32

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s == INVALID_SOCKET) {

        //postEvent(MAKE_ERROR(0, "Failed to create socket"));

        return false;
    }

    u_long on = 1;

    {
        int32_t ret = ioctlsocket(s, FIONBIO, &on);

        if (ret != NO_ERROR) {

            // ...
        }
    }

#else

    int32_t s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s == -1) {

        //postEvent(MAKE_ERROR(0, "Failed to create socket"));

        return false;
    }

    int32_t flags = fcntl(s, F_GETFL, 0);

    fcntl(s, F_SETFL, flags | O_NONBLOCK);

#endif

    // setup address and connect

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = a;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);

    setStatus(Connecting);

    int32_t res = ::connect(s, (struct sockaddr*)&addr, sizeof(addr));

#if defined _WIN32

    if (WSAGetLastError() == WSAEWOULDBLOCK) {

#else

    if (errno == EINPROGRESS) {

#endif
        // wait for socket to be connected

        uint32_t timeout = 10000;
        uint32_t ms = 0;

        while (ms < timeout) {

            if (canceled()) {

                return false;
            }

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 200000;

            fd_set ws;
            FD_ZERO(&ws);
            FD_SET(s, &ws);

            if (select(s + 1, nullptr, &ws, nullptr, &tv) > 0) {

                res = -1;

#if defined _WIN32

                uint32_t len = sizeof(int32_t);

                getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&res, (int32_t*)&len);
#else

                socklen_t len = sizeof(int32_t);

                getsockopt(s, SOL_SOCKET, SO_ERROR, &res, &len);
#endif

                break;
            }

            ms += 200;
        }
    }

    if (res) {

#if defined _WIN32

        closesocket(s);

#else

        ::close(s);

#endif
        setStatus(Disconnected);

        postEvent(ERROR_EVENT(Event::ConnectionFailure, "Connection failed"));

        return false;
    }

    m_socket = s;

    setStatus(Connected);

    // init tls session

    if ((res = gnutls_init((gnutls_session_t*)&m_session, GNUTLS_CLIENT)) < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    // set priority

    if ((res = gnutls_priority_set_direct((gnutls_session_t)m_session, "NORMAL:+VERS-TLS1.2", nullptr)) < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    // set credentials

    if ((res = gnutls_credentials_set((gnutls_session_t)m_session, GNUTLS_CRD_ANON, m_anonCred)) < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    if ((res = gnutls_credentials_set((gnutls_session_t)m_session, GNUTLS_CRD_CERTIFICATE, m_certCred)) < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    // assign socket

    gnutls_transport_set_ptr((gnutls_session_t)m_session, (gnutls_transport_ptr_t)(intptr_t)m_socket);

    // perform handshake

    do {

        res = gnutls_handshake((gnutls_session_t)m_session);
    }
    while (res < 0 && gnutls_error_is_fatal(res) == 0);

    if (res < 0) {

        //postEvent(MAKE_ERROR(res, gnutls_strerror(res)));

        return false;
    }

    // TODO: verify server certificate here

    //parseCert();

    setStatus(Secure);


    m_sender.resume();

    m_receiver.resume();


    postEvent(Event::create(Event::ConnectionSuccess));


    request(UBJ_OBJ("requestType" << Request::Login));


    return true;
}

/**
 * @brief Client::reconnect
 * @return
 */

bool Client::reconnect()
{
    for (;;) {

        if (connect(m_host, m_port)) {

            return true;
        }

        if (!wait(RECONNECT_INTERVAL)) {

            return false;
        }
    }
}

/**
 * @brief Client::disconnect
 * @param bye
 * @param event
 * @return
 */

bool Client::disconnect(bool bye, bool event)
{


    m_sender.suspend();

    m_receiver.suspend();


    if (m_session) {

        if (bye) {

            int32_t res = gnutls_bye((gnutls_session_t)m_session, GNUTLS_SHUT_RDWR);

            if (gnutls_error_is_fatal(res)) {

                // ...
            }
        }

        gnutls_deinit((gnutls_session_t)m_session);

        m_session = nullptr;
    }


    if (m_socket) {

#if defined _WIN32

        closesocket(m_socket);

#else

        ::close(m_socket);

#endif

        m_socket = -1;

        setStatus(Disconnected);

        if (event) {

            postEvent(Event::create(Event::Disconnected));
        }

        return true;
    }

    return false;
}

/**
 * @brief Client::wait
 * @param ms
 * @return
 */

bool Client::wait(uint32_t ms, uint32_t interval)
{
    uint32_t t = 0;

    while (t < ms) {

        if (canceled()) {

            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

        t += interval;
    }

    return true;
}

/**
 * @brief Client::readable
 * @param ms
 * @return
 */

int32_t Client::readable(uint32_t ms)
{
    // check if there is data left from previous read

    if (gnutls_record_check_pending((gnutls_session_t)m_session)) {

        return 1;
    }

    // check socket read readiness

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(m_socket, &rfds);

    struct timeval tv;
    tv.tv_sec = ms >= 1000 ? ms / 1000 : 0;
    tv.tv_usec = ms >= 1000 ? (ms % 1000) * 1000 : ms * 1000;

    int32_t res = select(m_socket+1, &rfds, nullptr, nullptr, &tv);

    if (res == -1) {

        return -1;
    }

    if (FD_ISSET(m_socket, &rfds)) {

        return 1;
    }

    return 0;
}

/**
 * @brief Client::writable
 * @param ms
 * @return
 */

int32_t Client::writable(uint32_t ms)
{
    // check socket write readiness

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(m_socket, &wfds);

    struct timeval tv;
    tv.tv_sec = ms >= 1000 ? ms / 1000 : 0;
    tv.tv_usec = ms >= 1000 ? (ms % 1000) * 1000 : ms * 1000;

    int32_t res = select(m_socket+1, nullptr, &wfds, nullptr, &tv);

    if (FD_ISSET(m_socket, &wfds)) {

        return 1;
    }

    return 0;
}

/**
 * @brief Client::send
 * @param data
 * @param size
 * @return
 */

uint32_t Client::send(uint8_t* data, uint32_t size)
{
    uint32_t s = 0;

    while (s < size) {

        if (canceled()) {

            break;
        }

        int32_t res = writable(200);

        if (res == -1) {

            return -1;
        }
        else
        if (res == 0) {

            continue;
        }

        int32_t ret = gnutls_record_send((gnutls_session_t)m_session, &data[s], size - s);

        if (ret == GNUTLS_E_AGAIN) {

        }

        if (gnutls_error_is_fatal(ret)) {

            return -1;
        }

        if (ret > 0) {

            s += ret;
        }
    }

    return s;
}

/**
 * @brief Client::recv
 * @param data
 * @param size
 * @return
 */

uint32_t Client::recv(uint8_t* data, uint32_t size)
{
    uint32_t s = 0;

    while (s < size) {

        if (canceled()) {

            break;
        }

        int32_t res = readable(200);

        if (res == -1) {

            return -1;
        }
        else
        if (res == 0) {

            continue;
        }

        int32_t ret = gnutls_record_recv((gnutls_session_t)m_session, &data[s], size - s);

        if (gnutls_error_is_fatal(ret)) {

            return -1;
        }

        if (ret > 0) {

            s += ret;
        }
    }

    return s;
}

/**
 * @brief Client::processIncomingRequest
 * @param request
 * @return
 */

bool Client::processIncomingRequest(const UBJ::Object &request)
{
    switch (request["requestType"].toInt()) {

        case Request::Dispatch:

            return processDispatchRequest(request);

        case Request::AddContact:

            return processContactRequest(request);

        case Request::AcceptContact:

            return processContactRequestAccepted(request);

        case Request::RejectContact:

            return processContactRequestRejected(request);

        case Request::ContactStatus:

            return processContactStatus(request);

        case Request::Push:

            return processPush(request);
    }

    return false;
}

/**
 * @brief Client::createStreamReceiver
 * @param pkt
 * @return
 */

StreamReceiver$ Client::createStreamReceiver(const Packet &packet)
{
    if (packet.streamType() == Packet::Resource) {

        uint32_t resourceId = packet.streamId();

        // get resource record

        UBJ::Object resource;

        if (!m_store->query(
                    "resources",
                    UBJ_OBJ("id" << resourceId << "status" << 0),
                    &resource)) {

            // ...

            return nullptr;
        }

        // get request record

        UBJ::Object request;

        if (!m_store->query(
                    "requests",
                    UBJ_OBJ("id" << resource["request"]),
                    &request)) {

            // ...

            return nullptr;
        }

        // extract key

        MemoryBuffer$ key = request["data"]["key"].buffer();

        // generate salt

        MemoryBuffer$ salt = PushRequest::resourceSalt(request["data"]["salt"].buffer(), resourceId);

        // create resource receiver

        ResourceReceiver$ receiver = ResourceReceiver::create(
                    packet, key, salt,
                    [this, request] (BufferReceiver$ receiver, MemoryBuffer$ buffer, uint32_t bytesReceived) {

                        if (receiver->status() == ResourceReceiver::Completed) {

                            uint64_t blobId = m_store->queryInt("resources", "data", UBJ_OBJ("id" << receiver->id()));

                            if (blobId) {

                                m_store->removeBlob("blob3", blobId);
                            }

                            // create resource blob

                            blobId = m_store->createBlob("blob3", buffer);

                            // ...

                            // update resource

                            if (!m_store->update(
                                        "resources",
                                        UBJ_OBJ("status" << Resource::Received << "data" << blobId),
                                        UBJ_OBJ("id" << receiver->id()))) {

                                // ...
                            }

                            UBJ::Object resource;

                            if (!m_store->query(
                                        "resources",
                                        UBJ_OBJ("id" << receiver->id()),
                                        &resource)) {

                                // ...
                            }

                            uint32_t numResources = m_store->count(
                                        "resources",
                                        UBJ_OBJ("request" << request["id"]));

                            uint32_t numCompleted = m_store->count(
                                        "resources",
                                        UBJ_OBJ("request" << request["id"] << "status" << Resource::Received));

                            if (numCompleted == numResources) {

                                // remove request

                                if (!m_store->remove(
                                            "requests",
                                            UBJ_OBJ("id" << request["id"]))) {

                                    // ...
                                }

                                // update message status

                                if (!m_store->update(
                                            "messages",
                                            UBJ_OBJ("status" << Message::Received << "time" << (uint64_t)time(nullptr)),
                                            UBJ_OBJ("id" << request["id"]))) {

                                    // ...
                                }

                                UBJ::Object message;

                                if (!m_store->query(
                                            "messages",
                                            UBJ_OBJ("id" << request["id"]),
                                            &message)) {

                                    // ...
                                }

                                postEvent(Event::create(Event::ResourceReceived, UBJ_OBJ(
                                                            "message" << message << "resource" << resource)));

                                postEvent(Event::create(Event::MessageReceived, UBJ_OBJ(
                                                            "message" << message)));
                            }
                            else {

                                UBJ::Object message;

                                if (!m_store->query(
                                            "messages",
                                            UBJ_OBJ("id" << request["id"]),
                                            &message)) {

                                    // ...
                                }

                                postEvent(Event::create(Event::ResourceReceived, UBJ_OBJ(
                                                            "message" << message << "resource" << resource)));
                            }
                        }
                        else {

                            // ...
                        }
                    });

        if (receiver) {

            // update resource status

            if (!m_store->update(
                        "resources",
                        UBJ_OBJ("status" << Resource::Status::Incoming),
                        UBJ_OBJ("id" << resourceId))) {

                // ...
            }

            UBJ::Object resource;

            if (!m_store->query(
                        "resources",
                        UBJ_OBJ("id" << resourceId),
                        &resource)) {

                // ...

                return nullptr;
            }

            UBJ::Object message;

            if (!m_store->query(
                        "messages",
                        UBJ_OBJ("id" << resource["request"]),
                        &message)) {

                // ...
            }

            postEvent(Event::create(Event::ResourceIncoming, UBJ_OBJ(
                                        "message" << message << "resource" << resource)));

            return receiver;
        }
    }
    else {

        StreamReceiver$ receiver = Engine::createStreamReceiver(packet);

        return receiver;
    }

    return nullptr;
}

/**
 * @brief Client::processRequestTimeout
 * @param request
 * @return
 */

bool Client::processRequestTimeout(Request$ request)
{
    postEvent(RequestEvent::create(Event::RequestTimeout, request, {}, ERROR_UBJ("Request timeout")));

    return true;
}

/**
 * @brief Client::processDispatchRequest
 * @param request
 * @return
 */

bool Client::processDispatchRequest(const UBJ::Object &request)
{
    uint32_t requestId = request["requestId"].toInt();

    uint32_t dispatchId = request["dispatchId"].toInt();

    uint32_t dispatchType = request["dispatchType"].toInt();

    if (dispatchType == Resource::Receipted) {

        if (!m_store->update("resources",
                             UBJ_OBJ("status" << Resource::Delivered << "time" << (uint64_t)time(nullptr)),
                             UBJ_OBJ("id" << dispatchId))) {

            // ...
        }

        UBJ::Object resource;

        if (!m_store->query("resources", UBJ_OBJ("id" << dispatchId), &resource)) {

            // ...
        }


        uint32_t messageId = resource["request"].toInt();


        uint32_t numResources = m_store->count(
                    "resources",
                    UBJ_OBJ("request" << messageId));

        uint32_t numCompleted = m_store->count(
                    "resources",
                    UBJ_OBJ("request" << messageId << "status" << Resource::Delivered));


        if (numCompleted == numResources) {

            if (!m_store->update("messages",
                                 UBJ_OBJ("status" << Message::Delivered << "time" << (uint64_t)time(nullptr)),
                                 UBJ_OBJ("id" << messageId))) {

                // ...
            }
        }

        UBJ::Object message;

        if (!m_store->query("messages", UBJ_OBJ("id" << resource["request"]), &message)) {


        }


        postEvent(Event::create(Event::ResourceDelivered, UBJ_OBJ("message" << message << "resource" << resource)));


        if (numCompleted == numResources) {

            postEvent(Event::create(Event::MessageDelivered, UBJ_OBJ("message" << message)));
        }

    }

    postRequestSuccess(requestId);

    return true;
}

/**
 * @brief Client::processContactRequest
 * @param request
 * @return
 */

bool Client::processContactRequest(const UBJ::Object &request)
{
    uint32_t requestId = request["requestId"].toInt();

    UBJ::Object rec;

    if (m_store->query("contact_requests", UBJ_OBJ("id" << requestId), &rec)) {

        postEvent(Event::create(Event::ContactRequest, rec));
    }
    else {

        if (request.hasField("addCode")) {

            m_store->remove("add_codes", UBJ_OBJ("addCode" << request["addCode"]));
        }

        if (!m_store->addContactRequest(
                    UBJ_OBJ(
                        "id"      << request["requestId"] <<
                        "time"    << (uint64_t)time(nullptr) <<
                        "dst"     << m_store->accountId() <<
                        "name"    << request["name"] <<
                        "phone"   << request["phone"] <<
                        "addCode" << request["addCode"]))) {

            return false;
        }

        postEvent(Event::create(Event::ContactRequest, request));
    }

    postRequestSuccess(requestId);

    return true;
}

/**
 * @brief Client::processContactRequestAccepted
 * @param request
 * @return
 */

bool Client::processContactRequestAccepted(const UBJ::Object &request)
{
    uint32_t requestId = request["requestId"].toInt();

    uint32_t contactRequestId = request["contactRequestId"].toInt();

    UBJ::Object rec;

    if (m_store->query("contact_requests", UBJ_OBJ("id" << contactRequestId), &rec)) {

        // update request

        m_store->update(
                    "contact_requests",
                    UBJ_OBJ("result" << Request::AcceptContact),
                    UBJ_OBJ("id" << contactRequestId));

        // add contact

        if (!m_store->addContact(
                UBJ_OBJ(
                    "id"        << request["contactId"] <<
                    "name"      << request["name"] <<
                    "phone"     << request["phone"] <<
                    "publicKey" << request["publicKey"] <<
                    "color"     << rec["color"]))) {

            // ...
        }

        // set contact status

        {
            MutexLocker lock(m_contactStatus);

            (*m_contactStatus)[request["contactId"].toInt()] = request["contactStatus"].toInt();
        }

        // raise event

        postEvent(RequestEvent::create(Event::ContactRequestAccepted, nullptr, request));

        // send response

        postRequestSuccess(requestId);

        return true;
    }
    else {

        postRequestFailure(requestId);
    }

    return false;
}

/**
 * @brief Client::processContactRequestRejected
 * @param request
 * @return
 */

bool Client::processContactRequestRejected(const UBJ::Object &request)
{
    uint32_t requestId = request["requestId"].toInt();

    uint32_t contactRequestId = request["contactRequestId"].toInt();

    UBJ::Object rec;

    if (m_store->query("contact_requests", UBJ_OBJ("id" << contactRequestId), &rec)) {

        // update request

        m_store->update(
                    "contact_requests",
                    UBJ_OBJ("result" << Request::RejectContact),
                    UBJ_OBJ("id" << contactRequestId));

        postEvent(RequestEvent::create(Event::ContactRequestRejected, nullptr, request));

        postRequestSuccess(requestId);

        return true;
    }
    else {

        postRequestFailure(requestId);
    }

    return false;
}

/**
 * @brief Client::processContactStatus
 * @param request
 * @return
 */

bool Client::processContactStatus(const UBJ::Object &request)
{
    // transfer contact status into map

    {
        MutexLocker lock(m_contactStatus);

        for (auto &it : request["contactStatus"].toArray()) {

            (*m_contactStatus)[it["contactId"].toInt()] = it["status"].toInt();
        }
    }

    postEvent(Event::create(Event::ContactStatus, request));

    return true;
}

/**
 * @brief Client::processPush
 * @param request
 * @return
 */

bool Client::processPush(const UBJ::Object &request)
{
    uint32_t requestId = request["requestId"].toInt();

    if (request.hasField("key") &&
        request.hasField("salt") &&
        request.hasField("meta")) {

        // decrypt message key

        MemoryBuffer$ key = Crypto::RSA::decrypt(m_store->privateKey(), request["key"].buffer());

        if (!key) {

            // TODO error code

            postRequestFailure(requestId);

            return false;
        }

        MemoryBuffer$ salt = request["salt"].buffer();

        if (!salt) {

            // TODO error code

            postRequestFailure(requestId);

            return false;
        }

        // decrypt meta data

        UBJ::Object meta;

        MemoryBuffer$ metaBuffer = request["meta"].buffer();

        if (!metaBuffer) {

            // TODO error code

            postRequestFailure(requestId);

            return false;
        }

        Crypto::AES aes;

        aes.setKey(key);

        aes.setCtr(salt);

        if (!aes.decrypt(metaBuffer, metaBuffer, metaBuffer->size())) {

            // TODO error code

            postRequestFailure(requestId);

            return false;
        }

        if (!UBJ::Value::read(meta, metaBuffer)) {

            // TODO error code

            postRequestFailure(requestId);

            return false;
        }

        uint32_t numResources = 0;

        if (meta.hasField("resources")) {

            numResources = meta["resources"].numItems();
        }

        UBJ::Array resourceIds;

        if (numResources) {

            // store request

            if (m_store->count("requests", UBJ_OBJ("id" << requestId)) == 0) {

                if (!m_store->insert("requests",
                                       UBJ_OBJ(
                                           "id"   << requestId <<
                                           "type" << request["type"] <<
                                           "data" << UBJ_OBJ(
                                               "key"       << key <<
                                               "salt"      << request["salt"] <<
                                               "resources" << request["resources"])))) {

                    // TODO error code

                    postRequestFailure(requestId);

                    return false;
                }
            }
            else {

                // ...
            }

            // store resources

            for (auto &it : meta["resources"].toArray()) {

                /*
                if (m_store->count("resources", UBJ_OBJ("hash" << it["hash"]))) {

                    continue;
                }
                */

                if (m_store->count("resources", UBJ_OBJ("id" << it["id"])) == 0) {

                    if (!m_store->insert("resources",
                                      UBJ_OBJ(
                                          "id"      << it["id"] <<
                                          "request" << requestId <<
                                          "status"  << 0 <<
                                          "time"    << 0 <<
                                          "name"    << it["name"] <<
                                          "size"    << it["size"] <<
                                          "hash"    << it["hash"]))) {

                        // TODO error code

                        postRequestFailure(requestId);

                        return false;
                    }
                }
                else {

                    // ...
                }

                resourceIds << it["id"];
            }
        }

        // process message

        processMessage(request, meta, numResources);

        // send response

        postRequestSuccess(requestId, UBJ_OBJ("resources" << resourceIds));
    }

    return true;
}

/**
 * @brief Client::processMessage
 * @param request
 * @param meta
 * @param numResources
 * @return
 */

bool Client::processMessage(const UBJ::Object &request, const UBJ::Object &meta, uint32_t numResources)
{
    if (meta.hasField("text") && !meta["text"].toStr().empty()) {

        uint32_t src = request["src"].toInt();

        // store message

        UBJ::Object message = UBJ_OBJ(
                    "id"      << request["requestId"] <<
                    "src"     << src <<
                    "dst"     << m_store->accountId() <<
                    "history" << m_store->latestHistory(src) <<
                    "status"  << (uint32_t)(numResources ? Message::Incoming : Message::Received) <<
                    "time"    << (uint64_t)time(nullptr) <<
                    "text"    << meta["text"]);

        if (m_store->count("messages", UBJ_OBJ("id" << request["requestId"])) == 0) {

            if (!m_store->insert("messages", message)) {

                return false;
            }
        }
        else {

            // ...
        }

        // raise event

        postEvent(Event::create(Event::MessageIncoming, UBJ_OBJ("message" << message)));

        if (!numResources) postEvent(Event::create(Event::MessageReceived, UBJ_OBJ("message" << message)));
    }

    return true;
}

// ============================================================ //

/**
 * @brief Sender::Sender
 * @param client
 */

Sender::Sender(Client *client)
    : m_client(client)
{

}

/**
 * @brief Sender::numPackets
 * @return
 */

uint32_t Sender::numPackets()
{
    MutexLocker lock(m_queue);

    return m_queue->size();
}

/**
 * @brief Sender::process
 * @param packet
 */

void Sender::process(Packet$ &packet)
{
    sendPacket(packet);
}

/**
 * @brief Sender::getElements
 * @return
 */

bool Sender::getElements()
{
    if (m_client->numStreamSenders()) {

        MutexLocker lock(m_queue);

        m_client->processStreamSenders(true, [this] (Packet$ pkt) -> bool {

            m_queue->push_back(pkt);

            return true;
        });

        if (!m_queue->empty()) {

            return true;
        }
    }

    return false;
}

/**
 * @brief Sender::sendPacket
 * @param pkt
 * @return
 */

uint32_t Sender::sendPacket(Packet$ pkt)
{
    uint32_t s = 0;

    uint32_t r = m_client->send((uint8_t*)&pkt->head(), sizeof(Packet::Head));

    if (r < sizeof(Packet::Head)) {

        return -1;
    }

    s += r;

    if (pkt->bodySize() > 0) {

        r = m_client->send(pkt->bodyData(), pkt->bodySize());

        if (r < pkt->bodySize()) {

            return -1;
        }

        s += r;
    }

    return s;
}

// ============================================================ //

/**
 * @brief Receiver::Receiver
 * @param client
 */

Receiver::Receiver(Client *client)
    : m_client(client)
{

}

/**
 * @brief Receiver::cancel
 */

void Receiver::cancel()
{
    Thread::cancel();

    notify();
}

/**
 * @brief Receiver::busy
 * @return
 */

bool Receiver::busy()
{
    MutexLocker locker(m_busy);

    return m_busy;
}

/**
 * @brief Receiver::wait
 */

void Receiver::waitPacket(uint32_t ms)
{
    if (running() && !canceled() && !numPackets()) {

        std::unique_lock<std::mutex> lock(m_waitMutex);

        m_waitCondition.wait_for(lock, std::chrono::milliseconds(ms));
    }
}

/**
 * @brief Receiver::fetchPacket
 * @param pkt
 * @return
 */

bool Receiver::fetchPacket(Packet &pkt)
{
    MutexLocker lock(m_packetQueue);

    if (!m_packetQueue->empty()) {

        pkt = m_packetQueue->front();

        m_packetQueue->pop_front();

        return true;
    }

    return false;
}

/**
 * @brief Receiver::numPackets
 * @return
 */

uint32_t Receiver::numPackets()
{
    MutexLocker lock(m_packetQueue);

    return m_packetQueue->size();
}

/**
 * @brief Receiver::notify
 */

void Receiver::notify()
{
    std::unique_lock<std::mutex> lock(m_waitMutex);

    m_waitCondition.notify_all();
}

/**
 * @brief Receiver::run
 */

void Receiver::run()
{
    for (;;) {

        if (canceled()) {

            break;
        }

        if (suspended()) {

            waitResume();

            continue;
        }

        if (m_client->status() >= Client::Secure &&
            m_client->readable(1000) > 0) {

            Packet pkt;

            int32_t res = recvPacket(pkt);

            if (res <= 0) {

                m_client->disconnect(false);

                continue;
            }
            else {

                {
                    MutexLocker lock(m_packetQueue);

                    m_packetQueue->push_back(pkt);
                }

                notify();
            }
        }
    }
}

/**
 * @brief Receiver::recvPacket
 * @param pkt
 * @return
 */

uint32_t Receiver::recvPacket(Packet &pkt)
{
    uint32_t s = 0;

    uint32_t r = m_client->recv((uint8_t*)&pkt.head(), sizeof(Packet::Head));

    if (r == 0) {

        // connection closed

        return 0;
    }

    if (r < sizeof(Packet::Head)) {

        return -1;
    }

    s += r;

    if (pkt.bodySize() > 0) {

        if (pkt.bodySize() > MAX_PACKET_BODY) {

            return -1;
        }

        MemoryBuffer$ body = MemoryBuffer::create(nullptr, pkt.bodySize());

        if (!body) {

            return -1;
        }

        r = m_client->recv(body->data(), pkt.bodySize());

        if (r < pkt.bodySize()) {

            return -1;
        }

        pkt.setBody(body);

        s += r;
    }

    return s;
}

// ============================================================ //

/*
void Client::parseCert()
{
    char serial[40];
    char dn[256];
    size_t size;
    unsigned int algo, bits;
    time_t expiration_time, activation_time;
    const gnutls_datum_t* cert_list;
    unsigned int cert_list_size = 0;
    gnutls_x509_crt_t cert;
    gnutls_datum_t cinfo;

    if (gnutls_certificate_type_get(m_session) != GNUTLS_CRT_X509) {

        return;
    }

    cert_list = gnutls_certificate_get_peers(m_session, &cert_list_size);

    std::cout << "Peer provided " << cert_list_size << " certificate(s)\n";

    if (cert_list_size > 0) {

        int ret;

        gnutls_x509_crt_init(&cert);

        gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER);

        std::cout << "Certificate info:\n";

        ret = gnutls_x509_crt_print(cert, GNUTLS_CRT_PRINT_ONELINE, &cinfo);
        if (ret == 0) {

            std::cout << cinfo.data << "\n";

            gnutls_free(cinfo.data);
        }

        gnutls_x509_crt_deinit(cert);
    }
}
*/

// ============================================================ //

}
