
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

#include "Zway/ubj/store/action/remove.h"
#include "Zway/ubj/store/store.h"

#include <sstream>

namespace Zway { namespace UBJ { namespace Store {

// ============================================================ //

/**
 * @brief RemoveAction::RemoveAction
 * @param store
 * @param table
 * @param where
 * @param callback
 */

RemoveAction::RemoveAction(
        Store$ store,
        const std::string &table,
        const Object &where,
        const RemoveAction::Callback &callback)
    : Action(store, table),
      m_callback(callback)
{
    std::stringstream sql;

    sql << "DELETE FROM " << table;

    std::string whereStr = wherePart(where);

    if (!whereStr.empty()) {

        sql << " WHERE " << whereStr;
    }

    m_sql = sql.str();
}

/**
 * @brief RemoveAction::execute
 * @return
 */

bool RemoveAction::execute()
{
    if (!prepareStmt(m_sql)) {

        if (m_callback) {

            m_callback(true, 0);

            notify();
        }

        return false;
    }

    //std::cerr << sql(true) << "\n";

    if (step() == SQLITE_DONE) {

        uint32_t numRowsAffected = sqlite3_changes(m_store->db());

        finish();

        if (m_callback) {

            m_callback(false, numRowsAffected);
        }

        notify();

        return true;
    }
    else {

        finish();

        if (m_callback) {

            m_callback(true, 0);
        }

        notify();

        return false;
    }
}

// ============================================================ //

}}}
