
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

#include "Zway/core/crypto/aes.h"
#include "Zway/core/memorybuffer.h"

#include <cstring>

#include <nettle/aes.h>
#include <nettle/ctr.h>

//#include <byteswap.h>

namespace Zway { namespace Crypto {

// ============================================================ //

typedef struct CTR_CTX(aes_ctx, AES_BLOCK_SIZE) AES_CTR_CTX;

/**
 * @brief AES::AES
 */

AES::AES()
{
    /*
    if (SecMem::getLockedSize()) {

        m_ctx = SecMem::malloc(sizeof(AES_CTR_CTX));
    }
    else {
    */
        m_ctx = new uint8_t[sizeof(AES_CTR_CTX)];
    /*
    }
    */
}

/**
 * @brief AES::~AES
 */

AES::~AES()
{
    if (m_ctx) {

        /*
        if (SecMem::getLockedSize()) {

            SecMem::free(m_ctx);
        }
        else {
        */
            memset(m_ctx, 0, sizeof(AES_CTR_CTX));

            delete[] m_ctx;
        /*
        }
        */

        m_ctx = 0;
    }
}

/**
 * @brief AES::setKey
 * @param key
 */

void AES::setKey(MemoryBuffer$ key)
{
    if (key) {

        aes_set_encrypt_key(&((AES_CTR_CTX*)m_ctx)->ctx, AES_KEY_SIZE, key->data());
    }
}

/**
 * @brief AES::setCtr
 * @param ctr
 */

void AES::setCtr(MemoryBuffer$ ctr)
{
    if (ctr) {

        CTR_SET_COUNTER((AES_CTR_CTX*)m_ctx, ctr->data());
    }
}

/**
 * @brief AES::encrypt
 * @param src
 * @param dst
 * @param size
 * @param callback
 * @return
 */

bool AES::encrypt(void *src, void *dst, uint32_t size, Callback callback)
{
    /*
    cbc_encrypt(
            &m_ctx,
            (nettle_crypt_func*)aes_encrypt,
            AES_BLOCK_SIZE,
            m_ctx.iv,
            size,
            data,
            data);
    */

    uint32_t chunkSize = 1024;

    uint32_t bytesEncrypted = 0;

    MemoryBuffer$ tmp;

    if (!dst) {

        tmp = MemoryBuffer::create(nullptr, chunkSize);

        if (!tmp) {

            return false;
        }
    }

    while (bytesEncrypted < size) {

        uint32_t bytesToEncrypt = chunkSize;

        if (size - bytesEncrypted < chunkSize) {

            bytesToEncrypt = size - bytesEncrypted;
        }

        uint8_t *d = dst ? (uint8_t*)dst + bytesEncrypted : tmp->data();

        uint8_t *s = (uint8_t*)src + bytesEncrypted;

        ctr_crypt(
                &((AES_CTR_CTX*)m_ctx)->ctx,
                (nettle_cipher_func*)aes_encrypt,
                AES_BLOCK_SIZE,
                ((AES_CTR_CTX*)m_ctx)->ctr,
                bytesToEncrypt,
                d,
                s);

        bytesEncrypted += bytesToEncrypt;

        if (callback) {

            if (!callback(d, bytesToEncrypt, bytesEncrypted, size)) {

                return false;
            }
        }
    }

    return true;
}

/**
 * @brief AES::encrypt
 * @param src
 * @param dst
 * @param size
 * @param callback
 * @return
 */

bool AES::encrypt(MemoryBuffer$ src, MemoryBuffer$ dst, uint32_t size, Callback callback)
{
    return encrypt(src->data(), dst ? dst->data() : nullptr, size, callback);
}

/**
 * @brief AES::decrypt
 * @param src
 * @param dst
 * @param size
 * @param callback
 * @return
 */

bool AES::decrypt(void *src, void *dst, uint32_t size, Callback callback)
{
    /*
    cbc_decrypt(
            &m_ctx,
            (nettle_crypt_func*)aes_decrypt,
            AES_BLOCK_SIZE,
            m_ctx.iv,
            size,
            data,
            data);
    */

    uint32_t chunkSize = 1024;

    uint32_t bytesDecrypted = 0;

    MemoryBuffer$ tmp;

    if (!dst) {

        tmp = MemoryBuffer::create(nullptr, chunkSize);

        if (!tmp) {

            return false;
        }
    }

    while (bytesDecrypted < size) {

        uint32_t bytesToDecrypt = chunkSize;

        if (size - bytesDecrypted < chunkSize) {

            bytesToDecrypt = size - bytesDecrypted;
        }

        uint8_t *d = dst ? (uint8_t*)dst + bytesDecrypted : tmp->data();

        uint8_t *s = (uint8_t*)src + bytesDecrypted;

        ctr_crypt(
                &((AES_CTR_CTX*)m_ctx)->ctx,
                (nettle_cipher_func*)aes_encrypt,
                AES_BLOCK_SIZE,
                ((AES_CTR_CTX*)m_ctx)->ctr,
                bytesToDecrypt,
                d,
                s);

        bytesDecrypted += bytesToDecrypt;

        if (callback) {

            if (!callback(d, bytesToDecrypt, bytesDecrypted, size)) {

                return false;
            }
        }
    }

    return true;
}

/**
 * @brief AES::decrypt
 * @param src
 * @param dst
 * @param size
 * @param callback
 * @return
 */

bool AES::decrypt(MemoryBuffer$ src, MemoryBuffer$ dst, uint32_t size, Callback callback)
{
    return decrypt(src->data(), dst ? dst->data() : nullptr, size, callback);
}

/**
 * @brief AES::getCtr
 * @return
 */

int AES::getCtr()
{
    AES_CTR_CTX *ctx = (AES_CTR_CTX*)m_ctx;

    int res;

    memcpy(&res, ctx->ctr + 12, 4);

    //return bswap_32(res);

    return 0;
}

// ============================================================ //

}

}
