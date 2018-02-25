
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

#ifndef ZWAY_CRYPTO_SECMEM_H
#define ZWAY_CRYPTO_SECMEM_H

#include "Zway/core/buffer.h"
#include "Zway/core/thread/thread.h"
#include <map>

namespace Zway {

// ============================================================ //

/**
 * @brief The SecMem class
 */

class SecMem
{
public:

    static bool setup(uint32_t size=0);

    static bool cleanup();

    static uint8_t *malloc(uint32_t size);

    static bool free(uint8_t* ptr);

    static uint32_t getPageSize();

    static uint32_t getLockedSize();

    static uint32_t getLockedSizeUsed();

    static uint32_t getLockedSizeAvailable();

private:

    SecMem();

    bool getOffset(uint32_t size, uint32_t *offset);

private:

    uint8_t* m_lockedData;

    uint32_t m_lockedSize;

    ThreadSafe<std::map<uint8_t*, uint32_t> > m_lockedMallocs;

    static SecMem* instance;
};

// ============================================================ //

}

#endif
