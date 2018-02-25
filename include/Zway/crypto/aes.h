
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

#ifndef ZWAY_CRYPTO_AES_H_
#define ZWAY_CRYPTO_AES_H_

#include "Zway/types.h"

namespace Zway {

USING_SHARED_PTR(MemoryBuffer)

namespace Crypto {

// ============================================================ //

/**
 * @brief The AES class
 */

class AES
{
public:

    typedef std::function<bool (uint8_t*, uint32_t, uint32_t, uint32_t)> Callback;

    AES();

    ~AES();

    void setKey(MemoryBuffer$ key);

    void setCtr(MemoryBuffer$ ctr);

    bool encrypt(void* src, void* dst, uint32_t size, Callback callback=nullptr);

    bool encrypt(MemoryBuffer$ src, MemoryBuffer$ dst, uint32_t size, Callback callback=nullptr);

    bool decrypt(void* src, void* dst, uint32_t size, Callback callback=nullptr);

    bool decrypt(MemoryBuffer$ src, MemoryBuffer$ dst, uint32_t size, Callback callback=nullptr);

    int getCtr();

private:

    uint8_t* m_ctx;
};

// ============================================================ //

}

}

#endif
