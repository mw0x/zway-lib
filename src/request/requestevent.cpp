
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

#include "Zway/request/requestevent.h"
#include "Zway/request.h"

namespace Zway {

// ============================================================ //

/**
 * @brief RequestEvent::cast
 * @param event
 * @return
 */

RequestEvent$ RequestEvent::cast(Event$ event)
{
    return std::dynamic_pointer_cast<RequestEvent>(event);
}

/**
 * @brief RequestEvent::create
 * @param type
 * @param request
 * @param data
 * @param error
 * @param callback
 * @return
 */

RequestEvent$ RequestEvent::create(
        Type type,
        Request$ request,
        const UBJ::Value &data,
        const UBJ::Value &error)
{
    return RequestEvent$(new RequestEvent(type, request, data, error));
}

/**
 * @brief RequestEvent::RequestEvent
 * @param type
 * @param request
 * @param data
 * @param error
 * @param callback
 */

RequestEvent::RequestEvent(
        Type type,
        Request$ request,
        const UBJ::Value &data,
        const UBJ::Value &error)
    : Event(type, data, error),
      m_request(request)
{

}

/**
 * @brief RequestEvent::dispatch
 */

void RequestEvent::dispatch()
{
    if (m_request) {

        m_request->invokeCallback(std::dynamic_pointer_cast<RequestEvent>(shared_from_this()));
    }
}

/**
 * @brief RequestEvent::request
 * @return
 */

Request$ RequestEvent::request()
{
    return m_request;
}

// ============================================================ //

}
