
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

#ifndef ZWAY_STORE_H_
#define ZWAY_STORE_H_

#include "Zway/ubj/store/store.h"

namespace Zway {

USING_SHARED_PTR(Store)
USING_SHARED_PTR(Message)

using MessageList = std::deque<Message$>;

// ============================================================ //

class Store : public UBJ::Store::Store
{
public:

    enum NodeId{

        DataNodeId = 1,

        ConfigNodeId
    };

    enum NodeType {

        Request = 1100,

        ContactRequest = 2100,

        AddCode = 3100,

        Contact = 4100,

        History = 5100,

        Message = 6100,

        Resource = 7100,

        Vfs = 8100,

        Thumbnail = 9100
    };

    enum VfsNodeType {

        Directory = 1,

        File
    };


    static Store$ create(const std::string &filename, const std::string &password, const UBJ::Object &data = UBJ::Object(), bool handler=false);

    static Store$ unlock(const std::string &filename, const std::string &password, bool handler=false);


    uint32_t accountId();

    std::string accountName();

    MemoryBuffer$ accountPassword();

    UBJ::Object &privateKey();

    UBJ::Object &publicKey();


    bool getConfig(UBJ::Object &config, bool contacts=false);

    bool setConfig(const UBJ::Object &config = UBJ::Object());


    uint64_t createVfsNode(VfsNodeType type, const std::string &name, uint64_t parent=0, const UBJ::Object &data={});

    MemoryBuffer$ getVfsNodeData(uint64_t id);

    bool deleteVfsNode(uint64_t id, bool recursive=true);


    uint64_t ioDir(uint32_t contactId, const std::string &name);


    uint64_t addContactRequest(const UBJ::Object &request);

    uint64_t addContact(const UBJ::Object &contact);


    uint64_t latestHistory(uint32_t contactId);

    uint64_t createHistory(uint32_t contactId);

    bool deleteHistory(uint64_t history);


    MessageList getMessages(uint32_t history);


    uint32_t updateInbox(uint32_t contactId, const UBJ::Array &inbox);

    uint32_t resetInbox(uint32_t contactId);

    uint32_t numInboxMessages(uint32_t contactId);


    bool deleteMessage(uint32_t id);

    bool deleteResource(uint32_t id);


    std::string randomColor();


private:

    Store();

    bool init(const std::string &filename, const std::string &password, const UBJ::Object &data, bool handler);

    bool open(const std::string &filename, const std::string &password, bool handler);


    bool createBlobTables();

    bool createVTables();

private:

    uint32_t m_accountId;

    std::string m_accountName;

    MemoryBuffer$ m_accountPassword;

    UBJ::Object m_privateKey;

    UBJ::Object m_publicKey;
};

// ============================================================ //

}

#endif
