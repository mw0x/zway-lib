
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

#ifndef ZWAY_UBJ_STORE_ACTION_H_
#define ZWAY_UBJ_STORE_ACTION_H_

#include "Zway/ubj/value.h"
#include "Zway/thread/safe.h"

#include <condition_variable>

#include <sqlite3.h>

namespace Zway { namespace UBJ { namespace Store {

USING_SHARED_PTR(Store)
USING_SHARED_PTR(Action)

// ============================================================ //

/**
 * @brief The Action class
 */

class Action : public std::enable_shared_from_this<Action>
{
public:

    virtual ~Action();

    int32_t step();

    virtual bool execute();

    virtual void reset();

    virtual void finish();


    void wait();


    sqlite3_stmt *stmt();

    std::string table();

    std::string sql(bool expanded=false);


protected:

    Action(Store$ store, const std::string &table=std::string());

    bool prepareStmt(const std::string &sql);

    std::string fieldsToReturnPart(const Array &fieldsToReturn);

    std::string wherePart(const Object &query);

    std::string orderPart(const Object &order);

    std::string insertPart(const Object &insert);

    std::string updatePart(const Object &update);


    int32_t bindUbjToStmt();


    bool rowToUbj(Object &obj);


    void notify();


protected:

    Store$ m_store;

    std::string m_table;

    bool m_vtab;

    sqlite3_stmt *m_stmt;

    std::string m_sql;

    Array m_bound;

    std::mutex m_waitMutex;

    std::condition_variable m_waitCondition;

    ThreadSafe<bool> m_done;

    friend class Cursor;

    friend class Handler;
};

// ============================================================ //

}}}

#endif
