
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

#include "Zway/ubj/store/action.h"
#include "Zway/ubj/store/cursor.h"

namespace Zway { namespace UBJ { namespace Store {

// ============================================================ //

/**
 * @brief Cursor::create
 * @param action
 * @return
 */

Cursor$ Cursor::create(Action$ action)
{
    Cursor$ cursor(new Cursor(action));

    return cursor;
}

/**
 * @brief Cursor::Cursor
 * @param action
 */

Cursor::Cursor(Action$ action)
    : m_action(action)
{

}

/**
 * @brief Cursor::~Cursor
 */

Cursor::~Cursor()
{

}

/**
 * @brief Cursor::forEach
 * @param fn
 */

void Cursor::forEach(const std::function<void (Object&)> &fn)
{
    reset();

    for (;;) {

        Object item;

        if (!next(item)) {

            return;
        }

        fn(item);
    }
}

/**
 * @brief Cursor::forEach
 * @param fn
 * @return
 */

bool Cursor::forEach(const std::function<bool (Object&)> &fn)
{
    reset();

    for (;;) {

        Object item;

        if (!next(item)) {

            return false;
        }

        if (!fn(item)) {

            return false;
        }
    }

    return true;
}

/**
 * @brief Cursor::next
 * @param item
 * @return
 */

bool Cursor::next(Object &item)
{
    if (!m_action) {

        return false;
    }

    if (m_action->step() == SQLITE_ROW) {

        if (m_action->rowToUbj(item)) {

            return true;
        }
    }

    return false;
}

/**
 * @brief Cursor::reset
 */

void Cursor::reset()
{
    if (m_action) {

        m_action->reset();
    }
}

/**
 * @brief Cursor::action
 * @return
 */

Action$ Cursor::action()
{
    return m_action;
}

/**
 * @brief Cursor::getInt
 * @param index
 * @return
 */

int32_t Cursor::getInt(int32_t index)
{
    if (m_action) {

        return sqlite3_column_int(m_action->stmt(), index);
    }

    return 0;
}

/**
 * @brief Cursor::getLong
 * @param index
 * @return
 */

int64_t Cursor::getLong(int32_t index)
{
    if (m_action) {

        return sqlite3_column_int64(m_action->stmt(), index);
    }

    return 0;
}

// ============================================================ //

}}}
