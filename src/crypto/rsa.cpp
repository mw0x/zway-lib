
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

#include "Zway/crypto/rsa.h"
#include "Zway/crypto/random.h"
#include "Zway/crypto/digest.h"
#include "Zway/memorybuffer.h"

#include <nettle/rsa.h>
#include <nettle/yarrow.h>

namespace Zway { namespace Crypto {

// ============================================================ //

/**
 * @brief mpzFromHexStr
 * @param str
 * @param z
 */

void mpzFromHexStr(const std::string& str, mpz_t* z)
{
    mpz_init(*z);

    mpz_set_str(*z, str.c_str(), 16);
}

/**
 * @brief mpzToHexStr
 * @param z
 * @return
 */

std::string mpzToHexStr(mpz_t z)
{
    std::string res;

    uint32_t len = mpz_sizeinbase(z, 16) + 1;

    res.resize(len);

    mpz_get_str(&res[0], 16, z);

    return res;
}

/**
 * @brief publicKeyToUbj
 * @param publicKey
 * @return
 */

UBJ::Value publicKeyToUbj(struct rsa_public_key& publicKey)
{
    UBJ::Object res;

    res["e"] = mpzToHexStr(publicKey.e);

    res["n"] = mpzToHexStr(publicKey.n);

    res["s"] = (uint64_t)publicKey.size;

    return res;
}

/**
 * @brief privateKeyToUbj
 * @param privateKey
 * @return
 */

UBJ::Value privateKeyToUbj(struct rsa_private_key& privateKey)
{
    UBJ::Object res;

    res["a"] = mpzToHexStr(privateKey.a);

    res["b"] = mpzToHexStr(privateKey.b);

    res["c"] = mpzToHexStr(privateKey.c);

    res["d"] = mpzToHexStr(privateKey.d);

    res["p"] = mpzToHexStr(privateKey.p);

    res["q"] = mpzToHexStr(privateKey.q);

    res["s"] = (uint64_t)privateKey.size;

    return res;
}

/**
 * @brief ubjToPublicKey
 * @param publicKeyObj
 * @param publicKey
 */

void ubjToPublicKey(const UBJ::Object &publicKeyObj, struct rsa_public_key& publicKey)
{
    mpzFromHexStr(publicKeyObj["e"].toStr(), &publicKey.e);

    mpzFromHexStr(publicKeyObj["n"].toStr(), &publicKey.n);

    publicKey.size = publicKeyObj["s"].toLong();
}

/**
 * @brief ubjToPrivateKey
 * @param privateKeyObj
 * @param privateKey
 */

void ubjToPrivateKey(const UBJ::Object &privateKeyObj, struct rsa_private_key& privateKey)
{
    mpzFromHexStr(privateKeyObj["a"].toStr(), &privateKey.a);

    mpzFromHexStr(privateKeyObj["b"].toStr(), &privateKey.b);

    mpzFromHexStr(privateKeyObj["c"].toStr(), &privateKey.c);

    mpzFromHexStr(privateKeyObj["d"].toStr(), &privateKey.d);

    mpzFromHexStr(privateKeyObj["p"].toStr(), &privateKey.p);

    mpzFromHexStr(privateKeyObj["q"].toStr(), &privateKey.q);

    privateKey.size = privateKeyObj["s"].toLong();
}

/**
 * @brief RSA::createKeyPair
 * @param publicKeyObj
 * @param privateKeyObj
 * @param bits
 * @return
 */

bool RSA::createKeyPair(
        UBJ::Object& publicKeyObj,
        UBJ::Object& privateKeyObj,
        uint32_t bits)
{
    struct rsa_public_key publicKey;

    struct rsa_private_key privateKey;

    rsa_public_key_init(&publicKey);

    rsa_private_key_init(&privateKey);

    if (!rsa_generate_keypair(
            &publicKey,
            &privateKey,
            Random::getYarrowCtx(),
            (nettle_random_func*)yarrow256_random,
            nullptr,
            nullptr,
            bits,
            30)) {

        rsa_public_key_clear(&publicKey);

        rsa_private_key_clear(&privateKey);

        return false;
    }

    publicKeyObj = publicKeyToUbj(publicKey);

    privateKeyObj = privateKeyToUbj(privateKey);

    rsa_public_key_clear(&publicKey);

    rsa_private_key_clear(&privateKey);

    return true;
}

/**
 * @brief RSA::encrypt
 * @param publicKeyObj
 * @param buf
 * @return
 */

MemoryBuffer$ RSA::encrypt(const UBJ::Object &publicKeyObj, MemoryBuffer$ buf)
{
    struct rsa_public_key publicKey;

    ubjToPublicKey(publicKeyObj, publicKey);

    mpz_t z;

    mpz_init(z);

    if (!rsa_encrypt(
            &publicKey,
            Random::getYarrowCtx(),
            (nettle_random_func*)yarrow256_random,
            buf->size(),
            buf->data(),
            z)) {

        return nullptr;
    }

    uint32_t len = mpz_sizeinbase(z, 16) + 1;

    MemoryBuffer$ res = MemoryBuffer::create(nullptr, len);

    if (!res) {

        return nullptr;
    }

    mpz_get_str((char*)res->data(), 16, z);

    mpz_clear(z);

    rsa_public_key_clear(&publicKey);

    return res;
}

/**
 * @brief RSA::decrypt
 * @param privateKeyObj
 * @param buf
 * @return
 */

MemoryBuffer$ RSA::decrypt(const UBJ::Object &privateKeyObj, MemoryBuffer$ buf)
{
    struct rsa_private_key privateKey;

    ubjToPrivateKey(privateKeyObj, privateKey);

    mpz_t z;

    mpz_init(z);

    mpz_set_str(z, (char*)buf->data(), 16);

    MemoryBuffer$ tmp = MemoryBuffer::create(nullptr, 2048);

    size_t len = tmp->size();

    if (!rsa_decrypt(
            &privateKey,
            &len,
            tmp->data(),
            z)) {

        return nullptr;
    }

    MemoryBuffer$ res = MemoryBuffer::create(tmp->data(), len);

    if (!res) {

        return nullptr;
    }

    mpz_clear(z);

    rsa_private_key_clear(&privateKey);

    return res;
}

/**
 * @brief RSA::sign
 * @param privateKeyObj
 * @param buf
 * @return
 */

MemoryBuffer$ RSA::sign(const UBJ::Object &privateKeyObj, MemoryBuffer$ buf)
{
    struct rsa_private_key privateKey;

    ubjToPrivateKey(privateKeyObj, privateKey);

    MemoryBuffer$ digest = Digest::digest(buf, Digest::DIGEST_SHA256);

    mpz_t z;

    mpz_init(z);

    if (!rsa_sha256_sign_digest(&privateKey, digest->data(), z)) {

        mpz_clear(z);

        rsa_private_key_clear(&privateKey);

        return nullptr;
    }

    uint32_t len = mpz_sizeinbase(z, 16) + 1;

    MemoryBuffer$ sign = MemoryBuffer::create(nullptr, len);

    if (!sign) {

        mpz_clear(z);

        rsa_private_key_clear(&privateKey);

        return nullptr;
    }

    mpz_get_str((char*)sign->data(), 16, z);

    mpz_clear(z);

    rsa_private_key_clear(&privateKey);

    return sign;
}

/**
 * @brief RSA::verify
 * @param publicKeyObj
 * @param buf
 * @param sign
 * @return
 */

bool RSA::verify(const UBJ::Object &publicKeyObj, MemoryBuffer$ buf, MemoryBuffer$ sign)
{
    struct rsa_public_key publicKey;

    ubjToPublicKey(publicKeyObj, publicKey);

    MemoryBuffer$ digest = Digest::digest(buf, Digest::DIGEST_SHA256);

    mpz_t z;

    mpz_init(z);

    mpz_set_str(z, (char*)sign->data(), 16);

    if (!rsa_sha256_verify_digest(&publicKey, digest->data(), z)) {

        mpz_clear(z);

        rsa_public_key_clear(&publicKey);

        return false;
    }

    mpz_clear(z);

    rsa_public_key_clear(&publicKey);

    return true;
}

// ============================================================ //

}

}
