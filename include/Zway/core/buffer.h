
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

#ifndef ZWAY_CORE_BUFFER_H_
#define ZWAY_CORE_BUFFER_H_

#include "Zway/types.h"

namespace Zway {

// ============================================================ //

/**
 * @brief The Buffer class
 */

class Buffer
{
public:

    static BUFFER create(const uint8_t* data, uint32_t size);

    static BUFFER create(BUFFER buffer);

    virtual ~Buffer();

    virtual void release();

    virtual void clear();

    virtual bool read(uint8_t* data, uint32_t size, uint32_t offset, uint32_t *bytesRead);

    virtual bool write(const uint8_t *data, uint32_t size, uint32_t offset, uint32_t *bytesWritten);

    virtual void flush();

    BUFFER copy();

    bool empty();

    virtual uint8_t* data();

    virtual uint32_t size();

protected:

    Buffer();

    bool init(const uint8_t* data, uint32_t size);

protected:

    uint8_t* m_data;

    uint32_t m_size;
};

// ============================================================ //

}

#endif
