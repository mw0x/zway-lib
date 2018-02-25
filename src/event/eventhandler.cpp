
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

#include "Zway/event/eventhandler.h"

namespace Zway {

// ============================================================ //

/**
 * @brief EventHandler::EventHandler
 */

EventHandler::EventHandler()
{

}

/**
 * @brief EventHandler::clearHandlers
 */

void EventHandler::clearHandlers()
{
    MutexLocker lock(m_handlers);

    m_handlers->clear();
}

/**
 * @brief EventHandler::addHandler
 * @param id
 * @param handler
 */

void EventHandler::addHandler(Client$ client, EventHandlerCallback handler)
{
    if (handler) {

        MutexLocker locker(m_handlers);

        (*m_handlers)[client] = handler;
    }
}

/**
 * @brief EventHandler::removeHandler
 * @param client
 */

void EventHandler::removeHandler(Client$ client)
{
    MutexLocker locker(m_handlers);

    m_handlers->erase(client);
}

/**
 * @brief EventDispatcher::process
 * @param event
 */

void EventHandler::process(ClientEvent &event)
{
    event.m_event->dispatch();

    EventHandlerCallback handler;

    {
        MutexLocker locker(m_handlers);

        if (m_handlers->find(event.m_client) != m_handlers->end()) {

            handler = m_handlers->at(event.m_client);
        }
    }

    if (handler) {

        handler(event.m_client, event.m_event);
    }
}

// ============================================================ //

}
