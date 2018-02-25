
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

#include "Zway/ubj/store/action/count.h"

#include <sstream>

namespace Zway { namespace UBJ { namespace Store {

// ============================================================ //

/**
 * @brief CountAction::CountAction
 * @param store
 * @param table
 * @param query
 * @param callback
 */

CountAction::CountAction(
        Store$ store,
        const std::string &table,
        const Object &query,
        const Callback &callback)
    : Action(store, table),
      m_callback(callback)
{
    std::stringstream sql;

    sql << "SELECT COUNT(1) FROM " << table;

    std::string whereStr = wherePart(query);

    if (!whereStr.empty()) {

        sql << " WHERE " << whereStr;
    }

    m_sql = sql.str();
}

/**
 * @brief CountAction::execute
 * @return
 */

bool CountAction::execute()
{
    if (!prepareStmt(m_sql)) {

        if (m_callback) {

            m_callback(true, 0);

            notify();
        }

        return false;
    }

    //std::cerr << sql(true) << "\n";

    if (step() == SQLITE_ROW) {

        uint64_t count = sqlite3_column_int64(m_stmt, 0);

        finish();

        if (m_callback) {

            m_callback(false, count);
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
