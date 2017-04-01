
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

#ifndef EVENT_HANDLER_H_
#define EVENT_HANDLER_H_

#include "Zway/event/event.h"
#include "Zway/core/thread/handler.h"

namespace Zway {

// ============================================================ //

/**
 * @brief The ClientEvent class
 */

class ClientEvent
{
public:

    ClientEvent(CLIENT client, EVENT event)
        : m_client(client),
          m_event(event) {}

    CLIENT m_client;

    EVENT m_event;
};

/**
 * @brief The EventHandler class
 */

class EventHandler : public Handler<ClientEvent>
{
public:

    EventHandler();

    void clearHandlers();

    void addHandler(CLIENT client, EVENT_HANDLER_CALLBACK handler);

    void removeHandler(CLIENT client);

protected:

    void process(ClientEvent &event);

protected:

    ThreadSafe<std::map<CLIENT, EVENT_HANDLER_CALLBACK>> m_handlers;
};

// ============================================================ //

}

#endif
