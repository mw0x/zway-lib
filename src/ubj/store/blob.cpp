
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
//   Copyright (C) 2018 Marc Weiler
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

#include "Zway/memorybuffer.h"
#include "Zway/ubj/store/blob.h"
#include "Zway/ubj/store/store.h"

namespace Zway { namespace UBJ {

// ============================================================ //

/**
 * @brief Store::Blob::Blob
 * @param store
 */

Store::Blob::Blob(Store$ store)
    : m_store(store),
      m_id(0),
      m_mode(false),
      m_blob(nullptr)
{

}

/**
 * @brief Store::Blob::~Blob
 */

Store::Blob::~Blob()
{
    close();
}

/**
 * @brief Store::Blob::open
 * @param table
 * @param id
 * @param readOnly
 * @param meta
 * @param mode
 * @param size
 * @param salt
 * @return
 */

bool Store::Blob::open(const std::string &table, uint64_t id, bool readOnly, bool meta, bool mode, uint32_t size, MemoryBuffer$ salt)
{
    Object info;

    if (meta) {

        if (!m_store->getBlobInfo(table, id, info)) {

            return false;
        }
    }

    sqlite3_blob* blob;

    if (sqlite3_blob_open(m_store->m_db, "main", table.c_str(), "data", id, !readOnly, &blob) != SQLITE_OK) {

        return false;
    }

    m_id = id;

    if (meta) {

        m_mode = info["mode"].toInt();

        m_size = info["size"].toInt();

        m_salt = info["salt"].buffer();
    }
    else {

        m_mode = mode;

        m_size = size;

        m_salt = salt;
    }

    m_blob = blob;

    m_aes.setCtr(m_salt);

    m_aes.setKey(m_store->m_key);

    return true;
}

/**
 * @brief Store::Blob::read
 * @param data
 * @param size
 * @param offset
 * @param bytesRead
 * @return
 */

bool Store::Blob::read(uint8_t *data, uint32_t size, uint32_t offset, uint32_t *bytesRead)
{
    if (!m_blob) {

        return false;
    }

    if (sqlite3_blob_read((sqlite3_blob*)m_blob, data, size, offset) != SQLITE_OK) {

        return false;
    }

    if (m_mode) {

        if (!m_aes.decrypt(data, data, size)) {

            return false;
        }
    }

    return true;
}

/**
 * @brief Store::Blob::read
 * @param data
 * @param size
 * @param offset
 * @param bytesRead
 * @return
 */

bool Store::Blob::read(MemoryBuffer$ data, uint32_t size, uint32_t offset, uint32_t *bytesRead)
{
    if (!data || !data->size() || (data->size() < size)) {

        return false;
    }

    if (!read(data->data(), size ? size : data->size(), offset, bytesRead)) {

        return false;
    }

    return true;
}

/**
 * @brief Store::Blob::write
 * @param data
 * @param size
 * @param offset
 * @param bytesWritten
 * @return
 */

bool Store::Blob::write(const uint8_t *data, uint32_t size, uint32_t offset, uint32_t *bytesWritten)
{
    if (!m_blob) {

        return false;
    }

    if (m_mode) {

        return m_aes.encrypt((uint8_t*)data, nullptr, size,
            [this, offset] (
                uint8_t* encryptedData,
                uint32_t bytesEncrypted,
                uint32_t bytesEncryptedTotal,
                uint32_t bytesToEncrypt) -> bool {

                    if (sqlite3_blob_write(
                                (sqlite3_blob*)m_blob,
                                encryptedData,
                                bytesEncrypted,
                                offset + bytesEncryptedTotal - bytesEncrypted) != SQLITE_OK) {

                        return false;
                    }

                    return true;
                });
    }
    else {

        if (sqlite3_blob_write((sqlite3_blob*)m_blob, data, size, offset) != SQLITE_OK) {

            return false;
        }
    }

    return true;
}

/**
 * @brief Store::Blob::write
 * @param data
 * @param size
 * @param offset
 * @param bytesWritten
 * @return
 */

bool Store::Blob::write(MemoryBuffer$ data, uint32_t size, uint32_t offset, uint32_t *bytesWritten)
{
    if (!data || !data->size() || (size > data->size())) {

        return false;
    }

    if (!write(data->data(), size ? size : data->size(), offset, bytesWritten)) {

        return false;
    }

    return true;
}

/**
 * @brief Store::Blob::close
 * @return
 */

bool Store::Blob::close()
{
    if (!m_blob) {

        return false;
    }

    if (sqlite3_blob_close((sqlite3_blob*)m_blob) != SQLITE_OK) {

        return false;
    }

    m_blob = nullptr;

    return true;
}

/**
 * @brief Store::Blob::id
 * @return
 */

uint64_t Store::Blob::id()
{
    return m_id;
}

/**
 * @brief Store::Blob::aes
 * @return
 */

Crypto::AES &Store::Blob::aes()
{
    return m_aes;
}

/**
 * @brief Store::Blob::size
 * @return
 */

uint32_t Store::Blob::size()
{
    return m_size;
}

// ============================================================ //

}

}
