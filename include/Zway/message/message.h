
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

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "Zway/core/ubj/value.h"

namespace Zway {

USING_SHARED_PTR(Message)
USING_SHARED_PTR(Resource)

using ResourceList = std::deque<Resource$>;

extern const uint32_t MAX_MESSAGE_PART;

// ============================================================ //

/**
 * @brief The Message class
 */

class Message : public UBJ::Object
{
public:

    enum Status
    {
        Undefined,

        Incoming,

        Outgoing,

        Sent,

        Received,

        Delivered,

        Receipted,

        Error
    };

    static Message$ create(const UBJ::Object &msg = UBJ::Object());

    void setId(uint32_t id);

    void setSrc(uint32_t src);

    void setDst(uint32_t dst);

    void setHistory(uint32_t history);

    void setStatus(Status status);

    void setTime(uint32_t time);

    void setText(const std::string &text);



    uint32_t id();

    uint32_t src();

    uint32_t dst();

    uint32_t history();

    Status status();

    uint32_t time();

    std::string text();

    void addResource(Resource$ res);

    void addResources(ResourceList resources);

    uint32_t numResources();

    Resource$ resource(uint32_t index);

    Resource$ resourceById(uint32_t resourceId);

protected:

    Message(const UBJ::Object &msg);

protected:

    ResourceList m_resources;

};

// ============================================================ //

}

#endif
