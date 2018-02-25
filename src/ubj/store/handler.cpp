
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

#include "Zway/ubj/store/handler.h"
#include "Zway/ubj/store/store.h"

namespace Zway { namespace UBJ { namespace Store {

// ============================================================ //

/**
 * @brief Handler::create
 * @param store
 * @return
 */

Handler$ Handler::create(Store$ store)
{
    return Handler$(new Handler(store));
}

/**
 * @brief Handler::Handler
 * @param store
 */

Handler::Handler(Store$ store)
    : m_store(store)
{
    sqlite3_busy_handler(m_store->db(), &Handler::busyHandler, this);
}

/**
 * @brief Handler::process
 * @param action
 */

void Handler::process(Action$ &action)
{
    action->execute();
}

/**
 * @brief Handler::busyHandler
 * @param data
 * @param arg
 * @return
 */

int Handler::busyHandler(void *data, int arg)
{
    Handler *handler = (Handler*)data;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (arg == 10) {

        // handle to long busy time

        return 0;
    }

    if (handler->canceled()) {

        return 0;
    }

    // go on waiting

    return 1;
}

// ============================================================ //

}}}
