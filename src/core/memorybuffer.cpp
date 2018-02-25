
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

#include "Zway/core/memorybuffer.h"
#include "Zway/core/crypto/erase_from_memory.h"

namespace Zway {

// ============================================================ //

/**
 * @brief MemoryBuffer::create
 * @param data
 * @param size
 * @return
 */

MemoryBuffer$ MemoryBuffer::create(const uint8_t* data, uint32_t size)
{
    MemoryBuffer$ res(new MemoryBuffer());

    if (!res->init(data, size)) {

        return nullptr;
    }

    return res;
}

/**
 * @brief MemoryBuffer::create
 * @param buffer
 * @return
 */

MemoryBuffer$ MemoryBuffer::create(MemoryBuffer$ buffer)
{
    if (buffer) {

        return create(buffer->data(), buffer->size());
    }

    return nullptr;
}

/**
 * @brief MemoryBuffer::MemoryBuffer
 */

MemoryBuffer::MemoryBuffer()
{

}

/**
 * @brief MemoryBuffer::~MemoryBuffer
 */

MemoryBuffer::~MemoryBuffer()
{
    release();
}

/**
 * @brief MemoryBuffer::init
 * @param data
 * @param size
 * @return
 */

bool MemoryBuffer::init(const uint8_t *data, uint32_t size)
{
    m_data = (uint8_t*)malloc(size);

    if (!m_data) {

        return false;
    }

    m_size = size;

    memset(m_data, 0, m_size);

    if (data) {

        write(data, size, 0, nullptr);
    }

    return true;
}

/**
 * @brief MemoryBuffer::release
 */

void MemoryBuffer::release()
{
    if (!empty()) {

        clear();

        free(m_data);
    }

    m_data = 0;

    m_size = 0;
}

/**
 * @brief MemoryBuffer::clear
 */

void MemoryBuffer::clear()
{
    if (!empty()) {
        erase_from_memory(m_data, m_size, m_size);
    }
}

/**
 * @brief MemoryBuffer::read
 * @param data
 * @param size
 * @param offset
 * @param bytesRead
 * @return
 */

bool MemoryBuffer::read(uint8_t *data, uint32_t size, uint32_t offset, uint32_t *bytesRead)
{
    if (empty() || !data) {

        return false;
    }

    uint32_t bytesToRead = 0;

    if (offset < m_size) {

        bytesToRead = m_size - offset > size ? size : m_size - offset;
    }

    if (bytesToRead) {

        memcpy(data, m_data + offset, bytesToRead);
    }

    if (bytesRead) {

        *bytesRead = bytesToRead;
    }

    return true;
}

/**
 * @brief MemoryBuffer::write
 * @param data
 * @param size
 * @param offset
 * @param bytesWritten
 * @return
 */

bool MemoryBuffer::write(const uint8_t *data, uint32_t size, uint32_t offset, uint32_t *bytesWritten)
{
    if (empty() || !data) {

        return false;
    }

    uint32_t bytesToWrite = 0;

    if (offset < m_size) {

        bytesToWrite = m_size - offset > size ? size : m_size - offset;
    }

    if (bytesToWrite) {

        memcpy(m_data + offset, data, bytesToWrite);
    }

    if (bytesWritten) {

        *bytesWritten = bytesToWrite;
    }

    return true;
}

/**
 * @brief MemoryBuffer::copy
 * @return
 */

MemoryBuffer$ MemoryBuffer::copy()
{
    if (!empty()) {

        return MemoryBuffer::create(m_data, m_size);
    }

    return nullptr;
}

/**
 * @brief MemoryBuffer::empty
 * @return
 */

bool MemoryBuffer::empty()
{
    return !(m_data && m_size);
}

/**
 * @brief MemoryBuffer::data
 * @return
 */

uint8_t* MemoryBuffer::data()
{
    return m_data;
}

// ============================================================ //

}
