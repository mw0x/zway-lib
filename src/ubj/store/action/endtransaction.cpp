
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

#include "Zway/ubj/store/action/endtransaction.h"

namespace Zway { namespace UBJ { namespace Store {

// ============================================================ //

/**
 * @brief EndTransaction::EndTransaction
 * @param store
 * @param callback
 */

EndTransaction::EndTransaction(
        Store$ store,
        const EndTransaction::Callback &callback)
    : Action(store),
      m_callback(callback)
{
    m_sql = "END TRANSACTION";
}

/**
 * @brief EndTransaction::execute
 * @return
 */

bool EndTransaction::execute()
{
    if (!prepareStmt(m_sql)) {

        if (m_callback) {

            m_callback(true);

            notify();
        }

        return false;
    }

    if (step() == SQLITE_DONE) {

        //std::cerr << m_sql << " - SUCCESS\n";

        finish();

        if (m_callback) {

            m_callback(false);
        }

        notify();

        return true;
    }
    else {

        //std::cerr << m_sql << " - FAILURE - " << sqlite3_errmsg(m_store->db()) << "\n";

        finish();

        if (m_callback) {

            m_callback(true);
        }

        notify();

        return false;
    }
}

// ============================================================ //

}}}
