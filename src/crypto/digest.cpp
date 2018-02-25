
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

#include "Zway/core/crypto/crypto.h"
#include "Zway/core/crypto/digest.h"
#include "Zway/core/crypto/secmem.h"
#include "Zway/core/memorybuffer.h"

#include <cstring>

#include <nettle/md5.h>
#include <nettle/sha2.h>

namespace Zway { namespace Crypto {

// ============================================================ //

/**
 * @brief Digest::Digest
 * @param type
 */

Digest::Digest(DigestType type)
    : m_type(type),
      m_ctx(0)
{
    uint32_t ctxSize = 0;

    switch (type) {

        case DIGEST_MD5:

            ctxSize = sizeof(md5_ctx);

            break;

        case DIGEST_SHA256:

            ctxSize = sizeof(sha256_ctx);

            break;
    }

    if (SecMem::getLockedSize()) {

        m_ctx = SecMem::malloc(ctxSize);
    }
    else {

        m_ctx = new uint8_t[ctxSize];
    }

    if (m_ctx) {

        switch (type) {

            case DIGEST_MD5:

                md5_init((md5_ctx*)m_ctx);

                break;

            case DIGEST_SHA256:

                sha256_init((sha256_ctx*)m_ctx);

                break;
        }
    }
}

/**
 * @brief Digest::~Digest
 */

Digest::~Digest()
{
    if (m_ctx) {

        if (SecMem::getLockedSize()) {

            SecMem::free(m_ctx);
        }
        else {

            switch (m_type) {

                case DIGEST_MD5:

                    memset(m_ctx, 0, sizeof(md5_ctx));

                    break;

                case DIGEST_SHA256:

                    memset(m_ctx, 0, sizeof(sha256_ctx));

                    break;
            }

            delete[] m_ctx;
        }
    }
}

/**
 * @brief Digest::update
 * @param data
 * @param size
 */

void Digest::update(uint8_t* data, uint32_t size)
{
    if (m_ctx) {

        switch (m_type) {

            case DIGEST_MD5:

                md5_update((md5_ctx*)m_ctx, size, data);

                break;

            case DIGEST_SHA256:

                sha256_update((sha256_ctx*)m_ctx, size, data);

                break;
        }
    }
}

/**
 * @brief Digest::update
 * @param buf
 */

void Digest::update(MemoryBuffer$ buf)
{
    update(buf->data(), buf->size());
}

/**
 * @brief Digest::result
 * @param digest
 * @param size
 */

void Digest::result(uint8_t* digest, uint32_t size)
{
    if (m_ctx) {

        switch (m_type) {

            case DIGEST_MD5:

                md5_digest((md5_ctx*)m_ctx, size, digest);

                break;

            case DIGEST_SHA256:

                sha256_digest((sha256_ctx*)m_ctx, size, digest);

                break;
        }
    }
}

/**
 * @brief Digest::result
 * @param size
 * @return
 */

MemoryBuffer$ Digest::result(uint32_t size)
{
    if (m_ctx) {

        MemoryBuffer$ buf = MemoryBuffer::create(nullptr, size);

        if (buf) {

            result(buf->data(), size);

            return buf;
        }
    }

    return nullptr;
}

/**
 * @brief Digest::result
 * @return
 */

MemoryBuffer$ Digest::result()
{
    return result(Digest::size(m_type));
}

/**
 * @brief Digest::digest
 * @param data
 * @param size
 * @param type
 * @return
 */

MemoryBuffer$ Digest::digest(uint8_t *data, uint32_t size, Digest::DigestType type)
{
    MemoryBuffer$ res;

    if (data && size) {

        Digest d(type);

        d.update(data, size);

        res = MemoryBuffer::create(nullptr, Digest::size(type));

        if (res) {

            d.result(res->data(), res->size());
        }
    }

    return res;
}

/**
 * @brief Digest::digest
 * @param data
 * @param type
 * @return
 */

MemoryBuffer$ Digest::digest(MemoryBuffer$ data, DigestType type)
{
    MemoryBuffer$ res;

    if (data) {

        res = digest(data->data(), data->size(), type);
    }

    return res;
}

/**
 * @brief Digest::digestHexStr
 * @param data
 * @param size
 * @param type
 * @return
 */

std::string Digest::digestHexStr(uint8_t *data, uint32_t size, Digest::DigestType type)
{
    std::string res;

    MemoryBuffer$ buf = digest(data, size, type);

    if (buf) {

        res = hexStr(buf);
    }

    return res;
}

/**
 * @brief Digest::digestHexStr
 * @param data
 * @param type
 * @return
 */

std::string Digest::digestHexStr(MemoryBuffer$ data, Digest::DigestType type)
{
    std::string res;

    if (data) {

        res = digestHexStr(data->data(), data->size(), type);
    }

    return res;
}

/**
 * @brief Digest::size
 * @param type
 * @return
 */

uint32_t Digest::size(Digest::DigestType type)
{
    switch (type) {

        case DIGEST_MD5:

            return DIGEST_MD5_SIZE;

        case DIGEST_SHA256:

            return DIGEST_SHA256_SIZE;
    }

    return 0;
}

// ============================================================ //

}

}
