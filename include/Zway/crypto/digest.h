
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

#ifndef ZWAY_CRYPTO_DIGEST_H_
#define ZWAY_CRYPTO_DIGEST_H_

#include "Zway/types.h"

namespace Zway {

USING_SHARED_PTR(MemoryBuffer)

namespace Crypto {

// ============================================================ //

/**
 * @brief The Digest class
 */

class Digest
{
public:

    enum {
        DIGEST_MD5_SIZE = 16,
        DIGEST_SHA256_SIZE = 32
    };

    enum DigestType {
        DIGEST_MD5,
        DIGEST_SHA256
    };

    Digest(DigestType type = DIGEST_MD5);

    ~Digest();

    void update(uint8_t* data, uint32_t size);

    void update(MemoryBuffer$ buf);

    void result(uint8_t* digest, uint32_t size);

    MemoryBuffer$ result(uint32_t size);

    MemoryBuffer$ result();

    static MemoryBuffer$ digest(uint8_t* data, uint32_t size, DigestType type = DIGEST_MD5);

    static MemoryBuffer$ digest(MemoryBuffer$ data, DigestType type = DIGEST_MD5);

    static std::string digestHexStr(uint8_t *data, uint32_t size, DigestType type = DIGEST_MD5);

    static std::string digestHexStr(MemoryBuffer$ data, DigestType type = DIGEST_MD5);

    static uint32_t size(DigestType type);

protected:

    DigestType m_type;

    uint8_t* m_ctx;
};

// ============================================================ //

}

}

#endif
