
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

#ifndef ZWAY_UBJ_STORE_BLOB_H_
#define ZWAY_UBJ_STORE_BLOB_H_

#include "Zway/buffer.h"
#include "Zway/ubj/store/store.h"

namespace Zway { namespace UBJ {

// ============================================================ //

/**
 * @brief The Blob class
 */

class Store::Blob : public Buffer
{
public:

    Blob(Store$ store);

    virtual ~Blob();

    bool open(const std::string &table, uint64_t id, bool readOnly, bool meta, bool mode=true, uint32_t size=0, MemoryBuffer$ salt=nullptr);

    bool read(uint8_t* data, uint32_t size, uint32_t offset=0, uint32_t *bytesRead=nullptr);

    bool read(MemoryBuffer$ data, uint32_t size, uint32_t offset=0, uint32_t *bytesRead=nullptr);

    bool write(const uint8_t *data, uint32_t size, uint32_t offset=0, uint32_t *bytesWritten=nullptr);

    bool write(MemoryBuffer$ data, uint32_t size=0, uint32_t offset=0, uint32_t *bytesWritten=nullptr);

    bool close();

    uint64_t id();

    Crypto::AES &aes();

    uint32_t size();

protected:

    Store$ m_store;

    uint64_t m_id;

    bool m_mode;

    void *m_blob;

    MemoryBuffer$ m_salt;

    Crypto::AES m_aes;
};

// ============================================================ //

}

}

#endif
