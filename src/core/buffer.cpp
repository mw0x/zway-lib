
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

#include "Zway/core/buffer.h"

#include <cstring>

namespace Zway {

// ============================================================ //

/**
 * @brief Buffer::create
 * @param data
 * @param size
 * @return
 */

BUFFER Buffer::create(const uint8_t* data, uint32_t size)
{
    BUFFER res(new Buffer());

    if (!res->init(data, size)) {

        return nullptr;
    }

    return res;
}

/**
 * @brief Buffer::create
 * @param buffer
 * @return
 */

BUFFER Buffer::create(BUFFER buffer)
{
    if (buffer) {

        return create(buffer->data(), buffer->size());
    }

    return nullptr;
}

/**
 * @brief Buffer::Buffer
 */

Buffer::Buffer()
    : m_data(0),
      m_size(0)
{

}

/**
 * @brief Buffer::~Buffer
 */

Buffer::~Buffer()
{
    release();
}

/**
 * @brief Buffer::init
 * @param data
 * @param size
 * @return
 */

bool Buffer::init(const uint8_t *data, uint32_t size)
{
    m_data = new uint8_t[size];

    if (!m_data) {

        return false;
    }

    m_size = size;

    clear();

    if (data) {

        write(data, size, 0, nullptr);
    }

    return true;
}

/**
 * @brief Buffer::release
 */

void Buffer::release()
{
    if (!empty()) {

        clear();

        delete[] m_data;
    }

    m_data = 0;

    m_size = 0;
}

/**
 * @brief Buffer::clear
 */

void Buffer::clear()
{
    if (!empty()) {

        memset(m_data, 0, m_size);
    }
}

/**
 * @brief Buffer::read
 * @param data
 * @param size
 * @param offset
 * @param bytesRead
 * @return
 */

bool Buffer::read(uint8_t *data, uint32_t size, uint32_t offset, uint32_t *bytesRead)
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
 * @brief Buffer::write
 * @param data
 * @param size
 * @param offset
 * @param bytesWritten
 * @return
 */

bool Buffer::write(const uint8_t *data, uint32_t size, uint32_t offset, uint32_t *bytesWritten)
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
 * @brief Buffer::flush
 */

void Buffer::flush()
{

}

/**
 * @brief Buffer::copy
 * @return
 */

BUFFER Buffer::copy()
{
    if (!empty()) {

        return Buffer::create(m_data, m_size);
    }

    return nullptr;
}

/**
 * @brief Buffer::empty
 * @return
 */

bool Buffer::empty()
{
    return !(m_data && m_size);
}

/**
 * @brief Buffer::data
 * @return
 */

uint8_t* Buffer::data()
{
    return m_data;
}

/**
 * @brief Buffer::size
 * @return
 */

uint32_t Buffer::size()
{
    return m_size;
}

// ============================================================ //

}
