
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

#ifndef ZWAY_CLIENT_H_
#define ZWAY_CLIENT_H_

#include "Zway/core/engine.h"
#include "Zway/core/crypto/crypto.h"
#include "Zway/core/thread/handler.h"
#include "Zway/types.h"

#if defined _WIN32
#include <windows.h>
#endif

//#include <gnutls/x509.h>

namespace Zway {

extern const uint16_t ZWAY_PORT;

extern const uint32_t RECONNECT_INTERVAL;

// ============================================================ //

/**
 * @brief The Sender class
 */

class Sender : public Handler<PACKET>
{
public:

    Sender(Client *client);

    uint32_t numPackets();

protected:

    void process(PACKET &packet);

    bool getElements();

    uint32_t sendPacket(PACKET pkt);

protected:

    Client *m_client;
};

/**
 * @brief The Receiver class
 */

class Receiver : public Thread
{
public:

    Receiver(Client *client);

    void cancel();

    bool busy();

    void waitPacket(uint32_t ms);

    bool fetchPacket(Packet &pkt);

    uint32_t numPackets();

protected:

    void notify();

    void run();

    uint32_t recvPacket(Packet &pkt);

protected:

    Client *m_client;

    ThreadSafe<bool> m_busy;

    ThreadSafe<std::list<Packet>> m_packetQueue;

    std::mutex m_waitMutex;

    std::condition_variable m_waitCondition;
};

/**
* @brief The Client class
*/

class Client :
        public Thread,
        public Engine,
        public std::enable_shared_from_this<Client>
{
public:

    enum Status {

        Closed,

        Started,

        Disconnected,

        Connecting,

        Connected,

        Secure,

        Authenticated
    };


#if defined _WIN32

    static void initWSA();

    static void freeWSA();

#endif


    static bool startup();

    static void cleanup();


    static CLIENT create(EVENT_HANDLER handler=nullptr, EVENT_HANDLER_CALLBACK callback=nullptr);


    virtual ~Client();


    bool start(const std::string& host, uint16_t port = ZWAY_PORT);

    bool close();


    void setStore(STORE store);

    void setEventHandler(EVENT_HANDLER handler);


    bool request(const UBJ::Object &args, REQUEST_CALLBACK callback=nullptr);


    void postEvent(EVENT event, bool immediately = false);

    bool postRequest(REQUEST request);

    bool postMessage(MESSAGE message);


    bool addStreamSender(STREAM_SENDER sender);


    Status status();


    STORE store();

    EVENT_HANDLER eventHandler();


    uint32_t contactStatus(uint32_t contactId);


protected:


    Client(EVENT_HANDLER handler=nullptr);


    void setStatus(Status status, bool event=true);


    void run();


    bool connect(const std::string& host, uint32_t port);

    bool reconnect();

    bool disconnect(bool bye=true, bool event=true);


    bool wait(uint32_t ms, uint32_t interval=1000);


    int32_t readable(uint32_t ms);

    int32_t writable(uint32_t ms);


    uint32_t send(uint8_t* data, uint32_t size);

    uint32_t recv(uint8_t* data, uint32_t size);


    bool processIncomingRequest(const UBJ::Object &request);

    STREAM_RECEIVER createStreamReceiver(const Packet &packet);

    bool processRequestTimeout(REQUEST request);


    bool processDispatchRequest(const UBJ::Object &request);

    bool processContactRequest(const UBJ::Object &request);

    bool processContactRequestAccepted(const UBJ::Object &request);

    bool processContactRequestRejected(const UBJ::Object &request);

    bool processContactStatus(const UBJ::Object &request);

    bool processPush(const UBJ::Object &request);

    bool processMessage(const UBJ::Object &request, const UBJ::Object &meta, uint32_t numResources);

  //void parseCert();


protected:


    ThreadSafe<Status> m_status;


#if defined _WIN32
    SOCKET m_socket;
#else
    int32_t m_socket;
#endif


    /*gnutls_session_t*/ void* m_session;

    /*gnutls_anon_client_credentials_t*/ void* m_anonCred;

    /*gnutls_certificate_credentials_t*/ void* m_certCred;


    std::string m_host;

    uint32_t m_port;


    Sender m_sender;

    Receiver m_receiver;

    EVENT_HANDLER m_eventHandler;


    STORE m_store;


    ThreadSafe<std::map<uint32_t, uint32_t>> m_contactStatus;


    friend class Sender;

    friend class Receiver;

    friend class LoginRequest;

    friend class LogoutRequest;

    friend class AcceptContactRequest;

};

// ============================================================ //

}

#endif
