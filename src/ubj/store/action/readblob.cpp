
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

#include "Zway/ubj/store/action/readblob.h"
#include "Zway/ubj/store/blob.h"
#include "Zway/ubj/store/store.h"

namespace Zway { namespace UBJ { namespace Store {

// ============================================================ //

/**
 * @brief ReadBlobAction::ReadBlobAction
 * @param store
 * @param table
 * @param id
 * @param callback
 */

ReadBlobAction::ReadBlobAction(
        Store$ store,
        const std::string &table,
        uint64_t id,
        const Callback &callback)
    : Action(store, table),
      m_id(id),
      m_callback(callback)
{

}

/**
 * @brief ReadBlobAction::execute
 * @return
 */

bool ReadBlobAction::execute()
{
    Blob$ blob = m_store->openBlob(m_table, m_id, true, true);

    if (blob) {

        if (m_callback) {

            m_callback(false, blob);
        }

        blob->close();

        notify();

        return true;
    }
    else {

        if (m_callback) {

            m_callback(true, 0);
        }

        notify();

        return false;
    }
}

// ============================================================ //

}}}
