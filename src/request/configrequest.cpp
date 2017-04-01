
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

#include "Zway/request/configrequest.h"
#include "Zway/request/requestevent.h"
#include "Zway/client.h"

namespace Zway {

// ============================================================ //

CONFIG_REQUEST ConfigRequest::create(CLIENT client, const UBJ::Object &config, REQUEST_CALLBACK callback)
{
    return CONFIG_REQUEST(new ConfigRequest(client, config, callback));
}

// ============================================================ //

ConfigRequest::ConfigRequest(CLIENT client, const UBJ::Object &config, REQUEST_CALLBACK callback)
    : Request(Config, {}, DEFAULT_TIMEOUT, callback),
      m_client(client)
{
    m_head["config"] = config;
}

// ============================================================ //

bool ConfigRequest::processResponse(const UBJ::Object &response)
{
    uint32_t status = response["status"].toInt();

    m_client->postEvent(RequestEvent::create(
            Event::Undefined,
            shared_from_this(),
            status == 0 ? UBJ::Object() : response,
            status == 1 ? UBJ::Object() : ERROR_UBJ(response["message"])));

    return true;
}

// ============================================================ //

}
