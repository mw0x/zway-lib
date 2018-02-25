
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

#ifndef EVENTS_H_
#define EVENTS_H_

#include "Zway/ubj/value.h"

namespace Zway {

USING_SHARED_PTR(Event)

using EventCallback = std::function<void (Event$)>;

// ============================================================ //

/**
 * @brief The Event class
 */

class Event : public std::enable_shared_from_this<Event>
{
public:

    enum Type
    {
        Undefined,

        Status,

        RequestTimeout,

        ConnectionSuccess,
        ConnectionFailure,
        ConnectionInterrupted,
        Disconnected,

        LoginSuccess,
        LoginFailure,

        ContactRequest,
        ContactRequestAccepted,
        ContactRequestRejected,
        ContactStatus,

        MessageIncoming,
        MessageOutgoing,
        MessageReceived,
        MessageSent,
        MessageDelivered,
        MessageReceipted,

        ResourceIncoming,
        ResourceOutgoing,
        ResourceReceived,
        ResourceSent,
        ResourceDelivered,
        ResourceReceipted
    };

    static Event$ create(
            Type type,
            const UBJ::Object &data = {},
            const UBJ::Object &error = {},
            EventCallback callback = nullptr);

    virtual ~Event();

    virtual void dispatch();

    Type type() const;

    UBJ::Object &data();

    UBJ::Object &error();

protected:

    Event(
        Type type,
        const UBJ::Object &data,
        const UBJ::Object &error,
        EventCallback callback = nullptr);

protected:

    Type m_type;

    UBJ::Object m_data;

    UBJ::Object m_error;

    EventCallback m_callback;

};

// ============================================================ //

#define ERROR_UBJ(message) \
    (UBJ_OBJ("message" << message << "file" << __FILE__ << "line" << __LINE__))

#define ERROR_EVENT(id, message) \
    (Zway::Event::create(id, Zway::UBJ::Object(), ERROR_UBJ(message)))

// ============================================================ //

}

#endif
