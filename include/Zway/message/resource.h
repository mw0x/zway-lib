
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

#ifndef RESOURCE_H_
#define RESOURCE_H_

#include "Zway/ubj/value.h"

#include <fstream>

namespace Zway {

USING_SHARED_PTR(Store)
USING_SHARED_PTR(Resource)

// ============================================================ //

/**
 * @brief The Resource class
 */

class Resource : public UBJ::Object
{
public:

    enum Type
    {
        FileSystem,

        LocalStore,

        RemoteStore
    };

    enum Status
    {
        Unknown,

        Incoming,

        Outgoing,

        Sent,

        Received,

        Delivered,

        Receipted,

        Error
    };

    static Resource$ create();

    virtual ~Resource();

    void setId(uint32_t id);

    void setType(Type type);

    void setName(const std::string& name);

    virtual bool open();

    virtual void close();

    virtual bool read(MemoryBuffer$ buf, uint32_t size, uint32_t offset);

    virtual bool readAll(MemoryBuffer$ buf);

    uint32_t id();

    Type type();

    std::string name();

    uint32_t size();

    uint32_t parts();

    std::string hash();

protected:

    Resource();

protected:

    uint32_t m_parts;

    MemoryBuffer$ m_data;

};

// ============================================================ //

/**
 * @brief The FileSystemResource class
 */

class FileSystemResource : public Resource
{
public:

    static Resource$ create(const std::string& path, const std::string &name = std::string());

protected:

    FileSystemResource(const std::string& path, const std::string &name);

    bool init(const std::string &path);

};

// ============================================================ //

/**
 * @brief The LocalStoreResource class
 */

class LocalStoreResource : public Resource
{
public:

    static Resource$ create(Store$ store, uint32_t nodeId, uint32_t resourceId=0);

protected:

    LocalStoreResource(Store$ store);

    bool init(uint32_t nodeId, uint32_t resourceId);

protected:

    Store$ m_store;
};

// ============================================================ //

}

#endif
