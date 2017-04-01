
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

#ifndef PUSH_REQUEST_H_
#define PUSH_REQUEST_H_

#include "Zway/core/request.h"
#include "Zway/message/message.h"

namespace Zway {

// ============================================================ //

/**
 * @brief The PushRequest class
 */

class PushRequest : public Request
{
public:


    static BUFFER resourceSalt(BUFFER salt, uint32_t resourceId);


    static PUSH_REQUEST create(CLIENT client, MESSAGE msg, uint32_t id=0, REQUEST_CALLBACK callback=nullptr);

    bool processResponse(const UBJ::Object &response);

    MESSAGE message();

protected:

    PushRequest(CLIENT client, Zway::MESSAGE msg, uint32_t id, REQUEST_CALLBACK callback);

    bool init();


    bool updateMessage(Message::Status status, UBJ::Object *message = nullptr);


    static void pushResources(PUSH_REQUEST request, const UBJ::Array &resourceIds, const std::function<void (PUSH_REQUEST)> &callback, uint32_t index=0);


protected:

    CLIENT m_client;

    MESSAGE m_msg;

    BUFFER m_key;

    BUFFER m_salt;
};

// ============================================================ //

}

#endif
