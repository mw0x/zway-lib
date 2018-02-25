
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

#include "Zway/event/event.h"

namespace Zway {

// ============================================================ //

/**
 * @brief Event::create
 * @param type
 * @param data
 * @param error
 * @param callback
 * @return
 */

Event$ Event::create(
        Type type,
        const UBJ::Object &data,
        const UBJ::Object &error,
        EventCallback callback)
{
    return Event$(new Event(type, data, error, callback));
}

/**
 * @brief Event::Event
 * @param type
 * @param data
 * @param error
 * @param callback
 */

Event::Event(
        Type type,
        const UBJ::Object &data,
        const UBJ::Object &error,
        EventCallback callback)
    : m_type(type),
      m_data(data),
      m_error(error),
      m_callback(callback)
{

}

/**
 * @brief Event::~Event
 */

Event::~Event()
{

}

/**
 * @brief Event::dispatch
 */

void Event::dispatch()
{
    if (m_callback) {

        m_callback(shared_from_this());
    }
}

/**
 * @brief Event::type
 * @return
 */

Event::Type Event::type() const
{
    return m_type;
}

/**
 * @brief Event::data
 * @return
 */

UBJ::Object &Event::data()
{
    return m_data;
}

/**
 * @brief Event::error
 * @return
 */

UBJ::Object &Event::error()
{
    return m_error;
}

// ============================================================ //

}
