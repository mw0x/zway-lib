
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

#include "Zway/message/message.h"
#include "Zway/message/resource.h"

namespace Zway {

// ============================================================ //

/**
 * @brief Message::create
 * @param msg
 * @return
 */

Message$ Message::create(const UBJ::Object &msg)
{
    return Message$(new Message(msg));
}

/**
 * @brief Message::Message
 * @param msg
 */

Message::Message(const UBJ::Object &msg)
    : UBJ::Object(msg)
{

}

/**
 * @brief Message::setId
 * @param id
 */

void Message::setId(uint32_t id)
{
    setField("id", id);
}

/**
 * @brief Message::setSrc
 * @param src
 */

void Message::setSrc(uint32_t src)
{
    setField("src", src);
}

/**
 * @brief Message::setDst
 * @param dst
 */

void Message::setDst(uint32_t dst)
{
    setField("dst", dst);
}

/**
 * @brief Message::setHistory
 * @param history
 */

void Message::setHistory(uint32_t history)
{
    setField("history", history);
}

/**
 * @brief Message::setStatus
 * @param status
 */

void Message::setStatus(Status status)
{
    setField("status", status);
}

/**
 * @brief Message::setTime
 * @param time
 */

void Message::setTime(uint32_t time)
{
    setField("time", time);
}

/**
 * @brief Message::setText
 * @param text
 */

void Message::setText(const std::string &text)
{
    setField("text", text);
}

/**
 * @brief Message::id
 * @return
 */

uint32_t Message::id()
{
    return (*this)["id"].toInt();
}

/**
 * @brief Message::src
 * @return
 */

uint32_t Message::src()
{
    return (*this)["src"].toInt();
}

/**
 * @brief Message::dst
 * @return
 */

uint32_t Message::dst()
{
    return (*this)["dst"].toInt();
}

/**
 * @brief Message::history
 * @return
 */

uint32_t Message::history()
{
    return (*this)["history"].toInt();
}

/**
 * @brief Message::status
 * @return
 */

Message::Status Message::status()
{
    return (Message::Status)(*this)["status"].toInt();
}

/**
 * @brief Message::time
 * @return
 */

uint32_t Message::time()
{
    return (*this)["time"].toInt();
}


/**
 * @brief Message::text
 * @return
 */

std::string Message::text()
{
    return (*this)["text"].toStr();
}

/**
 * @brief Message::resourceById
 * @param resourceId
 * @return
 */

Resource$ Message::resourceById(uint32_t resourceId)
{
    for (auto &res : m_resources) {

        if (res->id() == resourceId) {

            return res;
        }
    }

    return nullptr;
}

/**
 * @brief Message::addResource
 * @param res
 */

void Message::addResource(Resource$ res)
{
    if (res) {

        uint32_t id = res->id();

        m_resources.push_back(res);
    }
}

/**
 * @brief Message::addResources
 * @param resources
 */

void Message::addResources(ResourceList resources)
{
    for (auto &res : resources) {

        addResource(res);
    }
}

/**
 * @brief Message::numResources
 * @return
 */

uint32_t Message::numResources()
{
    return m_resources.size();
}

/**
 * @brief Message::resource
 * @param index
 * @return
 */

Resource$ Message::resource(uint32_t index)
{
    if (index < m_resources.size()) {

        return m_resources[index];
	}

    return nullptr;
}

// ============================================================ //

}
