
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

#include "Zway/crypto/crypto.h"
#include "Zway/crypto/digest.h"
#include "Zway/packet.h"
#include "Zway/memorybuffer.h"
#include "Zway/message/resource.h"
#include "Zway/store.h"
#include "Zway/ubj/store/blob.h"

namespace Zway {

// ============================================================ //

/**
 * @brief Resource::create
 * @return
 */

Resource$ Resource::create()
{
    return Resource$(new Resource());
}

/**
 * @brief Resource::Resource
 */

Resource::Resource()
    : UBJ::Object(),
      m_parts(0)
{

}

/**
 * @brief Resource::~Resource
 */

Resource::~Resource()
{
    close();
}

/**
 * @brief Resource::setId
 * @param id
 */

void Resource::setId(uint32_t id)
{
    setField("id", id);
}

/**
 * @brief Resource::setType
 * @param type
 */

void Resource::setType(Type type)
{
    setField("type", type);
}

/**
 * @brief Resource::setName
 * @param name
 */

void Resource::setName(const std::string& name)
{
    setField("name", name);
}

/**
 * @brief Resource::open
 * @return
 */

bool Resource::open()
{
    return true;
}

/**
 * @brief Resource::close
 */

void Resource::close()
{

}

/**
 * @brief Resource::read
 * @param buf
 * @param size
 * @param offset
 * @return
 */

bool Resource::read(MemoryBuffer$ buf, uint32_t size, uint32_t offset)
{
    if (!m_data) {

        return false;
    }

    if (!buf || buf->size() < size) {

        return false;
    }

    if (!m_data->read(buf->data(), size ? size : buf->size(), offset, nullptr)) {

        return false;
    }

    return true;
}

/**
 * @brief Resource::readAll
 * @param buf
 * @return
 */

bool Resource::readAll(MemoryBuffer$ buf)
{
    return read(buf, size(), 0);
}

/**
 * @brief Resource::id
 * @return
 */

uint32_t Resource::id()
{
    return (*this)["id"].toInt();
}

/**
 * @brief Resource::type
 * @return
 */

Resource::Type Resource::type()
{
    return (Type)(*this)["type"].toInt();
}

/**
 * @brief Resource::name
 * @return
 */

std::string Resource::name()
{
    return (*this)["name"].toStr();
}

/**
 * @brief Resource::size
 * @return
 */

uint32_t Resource::size()
{
    return m_data ? m_data->size() : 0;
}

/**
 * @brief Resource::parts
 * @return
 */

uint32_t Resource::parts()
{
    return m_parts;
}

/**
 * @brief Resource::hash
 * @return
 */

std::string Resource::hash()
{
    return (*this)["hash"].toStr();
}

// ============================================================ //

/**
 * @brief FileSystemResource::create
 * @param path
 * @param name
 * @return
 */

Resource$ FileSystemResource::create(const std::string &path, const std::string &name)
{
    std::shared_ptr<FileSystemResource> res(new FileSystemResource(path, name));

    if (!res->init(path)) {

        return nullptr;
    }

    return res;
}

/**
 * @brief FileSystemResource::FileSystemResource
 * @param path
 * @param name
 */

FileSystemResource::FileSystemResource(const std::string &path, const std::string &name)
{
    setType(FileSystem);

    setField("path", path);

    setField("name", name);
}

/**
 * @brief FileSystemResource::init
 * @param path
 * @return
 */

bool FileSystemResource::init(const std::string &path)
{
    std::ifstream fs(path);

    if (!fs) {

        return false;
    }

    fs.seekg(0, std::ios_base::end);

    uint32_t size = fs.tellg();

    fs.seekg(0, std::ios_base::beg);

    if (!size) {

        return false;
    }

    m_data = MemoryBuffer::create(nullptr, size);

    if (!m_data) {

        return false;
    }

    Crypto::Digest digest(Crypto::Digest::DIGEST_SHA256);

    uint32_t chunkSize = 4096;

    uint32_t bytesRead = 0;

    MemoryBuffer$ buf = MemoryBuffer::create(nullptr, chunkSize);

    while (bytesRead < size) {

        uint32_t bytesToRead = chunkSize;

        if (size - bytesRead < chunkSize) {

            bytesToRead = size - bytesRead;
        }

        fs.read((char*)buf->data(), bytesToRead);

        m_data->write(buf->data(), bytesToRead, bytesRead, nullptr);

        digest.update(buf->data(), bytesToRead);

        bytesRead += bytesToRead;
    }

    setField("hash", Crypto::hexStr(digest.result()));

    setField("path", path);

    m_parts = size / MAX_PACKET_BODY + (size % MAX_PACKET_BODY ? 1 : 0);

    return true;
}

// ============================================================ //

/**
 * @brief LocalStoreResource::create
 * @param store
 * @param nodeId
 * @param resourceId
 * @return
 */

Resource$ LocalStoreResource::create(Store$ store, uint32_t nodeId, uint32_t resourceId)
{
    std::shared_ptr<LocalStoreResource> res(new LocalStoreResource(store));

    if (!res->init(nodeId, resourceId)) {

        return nullptr;
    }

    return res;
}

/**
 * @brief LocalStoreResource::LocalStoreResource
 * @param store
 */

LocalStoreResource::LocalStoreResource(Store$ store)
    : m_store(store)
{
    setType(LocalStore);
}

/**
 * @brief LocalStoreResource::init
 * @param nodeId
 * @param resourceId
 * @return
 */

bool LocalStoreResource::init(uint32_t nodeId, uint32_t resourceId)
{
    UBJ::Object data;

    if (resourceId) {

        if (!m_store->query("resources", UBJ_OBJ("rowid" << resourceId), &data)) {

            return false;
        }
    }
    else {

        if (!m_store->query("vfs", UBJ_OBJ("rowid" << nodeId), &data)) {

            return false;
        }
    }

    UBJ::Object info;

    uint64_t blobId = data["data"].toInt();

    if (!m_store->getBlobInfo("blob3", blobId, info)) {

        return false;
    }

    uint32_t blobSize = data["size"].toInt();

    m_store->readBlob("blob3", blobId, [this, &blobSize] (bool error, UBJ::Store::Blob$ blob) {

        if (!error) {

            MemoryBuffer$ data = MemoryBuffer::create(nullptr, blobSize);

            if (!data) {

                return;
            }

            if (blob->read(data, blobSize)) {

                m_data = data;
            }
        }
    });

    if (!m_data) {

        return false;
    }

    setField("name", data["name"]);

    setField("hash", data["hash"]);

    m_parts = blobSize / MAX_PACKET_BODY + (blobSize % MAX_PACKET_BODY ? 1 : 0);

    return true;
}

// ============================================================ //

}
