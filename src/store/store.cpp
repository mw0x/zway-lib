
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

#include "Zway/core/crypto/random.h"
#include "Zway/message/message.h"
#include "Zway/store/store.h"

namespace Zway {

// ============================================================ //

/**
 * @brief Store::Store
 */

Store::Store()
    : UBJ::Store(),
      m_accountId(0)
{

}

/**
 * @brief Store::create
 * @param filename
 * @param password
 * @param data
 * @param handler
 * @return
 */

Store$ Store::create(const std::string &filename, const std::string &password, const UBJ::Object &data, bool handler)
{
    Store$ store(new Store());

    if (store->init(filename, password, data, handler)) {

        return store;
    }

    return nullptr;
}

/**
 * @brief Store::unlock
 * @param filename
 * @param password
 * @param handler
 * @return
 */

Store$ Store::unlock(const std::string &filename, const std::string &password, bool handler)
{
    Store$ store(new Store());

    if (store->open(filename, password, handler)) {

        return store;
    }

    return nullptr;
}

/**
 * @brief Store::accountId
 * @return
 */

uint32_t Store::accountId()
{
    return m_accountId;
}

/**
 * @brief Store::accountName
 * @return
 */

std::string Store::accountName()
{
    return m_accountName;
}

/**
 * @brief Store::accountPassword
 * @return
 */

MemoryBuffer$ Store::accountPassword()
{
    return m_accountPassword;
}

/**
 * @brief Store::privateKey
 * @return
 */

UBJ::Object &Store::privateKey()
{
    return m_privateKey;
}

/**
 * @brief Store::publicKey
 * @return
 */

UBJ::Object &Store::publicKey()
{
    return m_publicKey;
}

/**
 * @brief Store::getConfig
 * @param config
 * @param contacts
 * @return
 */

bool Store::getConfig(UBJ::Object &config, bool contacts)
{
    if (!getBlobData("blob1", Store::ConfigNodeId, config)) {

        return false;
    }

    if (contacts) {

        UBJ::Array contacts;

        query("contacts", {}, [&contacts] (bool error, UBJ::Store::CURSOR cursor) {

            cursor->forEach([&] (UBJ::Object &contact) {

                contacts << UBJ_OBJ("contactId" << contact["id"] << "notifyStatus" << 1);
            });
        });

        config["contacts"] = contacts;
    }

    return true;
}

/**
 * @brief Store::setConfig
 * @param config
 * @return
 */

bool Store::setConfig(const UBJ::Object &config)
{
    UBJ::Object obj;

    if (!getConfig(obj)) {

        return false;
    }

    // override props

    for (auto &it : config) {

        obj[it.first] = it.second;
    }

    if (!updateBlobData("blob1", Store::ConfigNodeId, obj)) {

        return false;
    }

    return true;
}

/**
 * @brief Store::createVfsNode
 * @param type
 * @param name
 * @param parent
 * @return
 */

uint64_t Store::createVfsNode(VfsNodeType type, const std::string &name, uint64_t parent, const UBJ::Object &data)
{
    UBJ::Object node = data;

    node << UBJ_OBJ(
                "type"   << type <<
                "parent" << parent <<
                "status" << 0 <<
                "time"   << (uint64_t)time(nullptr) <<
                "name"   << name);

    if (count("vfs", node) > 0) {

        return 0;
    }

    uint64_t nodeId = insert("vfs", node);

    if (!nodeId) {

        return 0;
    }

    return nodeId;
}

/**
 * @brief Store::getVfsNodeData
 * @param id
 * @return
 */

MemoryBuffer$ Store::getVfsNodeData(uint64_t id)
{
    UBJ::Object node;

    if (!query("vfs", UBJ_OBJ("rowid" << id), &node, {}, UBJ_ARR("data"))) {

        return nullptr;
    }

    MemoryBuffer$ buf = getBlobData("blob3", node["data"].toInt());

    if (!buf) {

        return nullptr;
    }

    return buf;
}

/**
 * @brief Store::deleteVfsNode
 * @param id
 * @param recursive
 * @return
 */

bool Store::deleteVfsNode(uint64_t id, bool recursive)
{
    UBJ::Object node;

    if (!query("vfs", UBJ_OBJ("rowid" << id), &node)) {

        return false;
    }

    uint32_t type = node["type"].toInt();

    if (type == Directory) {

        if (recursive) {

            // delete child nodes if any

            std::list<UBJ::Object> nodes;

            if (!query("vfs", UBJ_OBJ("parent" << id), nodes, UBJ::Object(), UBJ_ARR("rowid"))) {

                return false;
            }

            for (auto &node : nodes) {

                if (!deleteVfsNode(node["rowid"].toInt())) {

                    return false;
                }
            }
        }

        if (remove("vfs", UBJ_OBJ("rowid" << id))) {

            return true;
        }
    }
    else
    if (type == File) {

        uint32_t blobId = node["data"].toInt();

        if (!remove("vfs", UBJ_OBJ("rowid" << id))) {

            return false;
        }

        if (blobId &&
            !count("vfs", UBJ_OBJ("data" << blobId)) &&
            !count("resources", UBJ_OBJ("data" << blobId))) {

            if (!removeBlob("blob3", blobId)) {

                return false;
            }
        }

        return true;
    }

    return false;
}

/**
 * @brief Store::ioDir
 * @param contactId
 * @param name
 * @return
 */

uint64_t Store::ioDir(uint32_t contactId, const std::string &name)
{
    UBJ::Object contact;

    if (!query("contacts", UBJ_OBJ("id" << contactId), &contact, {}, UBJ::Array("name"))) {

        return 0;
    }

    uint64_t outDir = queryInt("vfs", "rowid", UBJ_OBJ("type" << Directory << "parent" << 0 << "name" << name));

    if (!outDir) {

        outDir = createVfsNode(Directory, name);

        if (!outDir) {

            return 0;
        }
    }

    std::string contactName = contact["name"].toStr();

    uint64_t contactOutDir = queryInt("vfs", "rowid", UBJ_OBJ("type" << Directory << "parent" << outDir << "name" << contactName));

    if (!contactOutDir) {

        contactOutDir = createVfsNode(Directory, contactName, outDir);

        if (!contactOutDir) {

            return 0;
        }
    }

    return contactOutDir;
}

/**
 * @brief Store::addContactRequest
 * @param request
 * @return
 */

uint64_t Store::addContactRequest(const UBJ::Object &request)
{
    UBJ::Object tmp = request;

    if (!tmp.hasField("color")) {

        tmp["color"] = randomColor();
    }

    uint64_t res = insert("contact_requests", tmp);

    return res;
}

/**
 * @brief Store::addContact
 * @param contact
 * @return
 */

uint64_t Store::addContact(const UBJ::Object &contact)
{
    UBJ::Object tmp = contact;

    if (!tmp.hasField("color")) {

        tmp["color"] = randomColor();
    }

    uint64_t res = insert("contacts", tmp);

    return res;
}

/**
 * @brief Store::latestHistory
 * @param contactId
 * @return
 */

uint64_t Store::latestHistory(uint32_t contactId)
{
    UBJ::Object rec;

    uint32_t historyId = 0;

    if (query("histories", UBJ_OBJ("dst" << contactId), &rec, UBJ_OBJ("time" << -1), UBJ_ARR("rowid"))) {

        historyId = rec["rowid"].toInt();
    }
    else {

        historyId = createHistory(contactId);
    }

    return historyId;

}

/**
 * @brief Store::createHistory
 * @param contactId
 * @return
 */

uint64_t Store::createHistory(uint32_t contactId)
{
    uint64_t historyId = 0;

    historyId = insert("histories", UBJ_OBJ("dst" << contactId << "time" << (uint64_t)time(nullptr)));

    return historyId;
}

/**
 * @brief Store::deleteHistory
 * @param history
 * @return
 */

bool Store::deleteHistory(uint64_t history)
{
    UBJ::Object rec;

    if (!query("histories", UBJ_OBJ("rowid" << history), &rec)) {

        return false;
    }

    // delete messages

    std::list<uint32_t> messages;

    query("messages", UBJ_OBJ("history" << history), UBJ::Object(), UBJ_ARR("id"), 0, 0,
          [&messages] (bool error, CURSOR cursor) {

        cursor->forEach([&] (UBJ::Object &message) {

            messages.push_back(message["id"].toInt());
        });
    });

    if (!messages.empty()) {

        for (auto &it : messages) {

            if (!deleteMessage(it)) {

                //return false;
            }
        }
    }

    // delete history

    if (!remove("histories", UBJ_OBJ("rowid" << history))) {

        return false;
    }

    return true;
}

/**
 * @brief Store::getMessages
 * @param history
 * @return
 */

MessageList Store::getMessages(uint32_t history)
{
    MessageList messages;

    query("messages", UBJ_OBJ("history" << history), [&messages] (bool error, CURSOR cursor) {

        cursor->forEach([&] (UBJ::Object &message) {

            messages.push_back(Message::create(message));
        });
    });


    return messages;
}

/**
 * @brief Store::numInboxMessages
 * @param contactId
 * @return
 */

uint32_t Store::updateInbox(uint32_t contactId, const UBJ::Array &inbox)
{
    UBJ::Object contact;

    query("contacts", UBJ_OBJ("id" << contactId), &contact, {}, {"inbox"});

    UBJ::Array currentInbox(contact["inbox"]);

    std::map<uint32_t, bool> map;

    for (auto &it : currentInbox) {

        map[it.toInt()] = true;
    }

    uint32_t numInserted = 0;

    for (auto &it : inbox) {

        uint32_t messageId = it.toInt();

        if (map.find(messageId) == map.end()) {

            currentInbox << messageId;

            numInserted++;
        }
    }

    update("contacts", UBJ_OBJ("inbox" << currentInbox), UBJ_OBJ("id" << contactId));

    return numInserted;
}

/**
 * @brief Store::setNumInboxMessages
 * @param contactId
 * @param numMessages
 * @return
 */

uint32_t Store::resetInbox(uint32_t contactId)
{
    update("contacts", UBJ_OBJ("inbox" << UBJ::Array()), UBJ_OBJ("id" << contactId));

    return 0;
}

/**
 * @brief Store::numInboxMessages
 * @param contactId
 * @return
 */

uint32_t Store::numInboxMessages(uint32_t contactId)
{
    UBJ::Object contact;

    query("contacts", UBJ_OBJ("id" << contactId), &contact, {}, {"inbox"});

    return contact["inbox"].numItems();
}

/**
 * @brief Store::deleteMessage
 * @param id
 * @return
 */

bool Store::deleteMessage(uint32_t id)
{
    UBJ::Object rec;

    if (!query("messages", UBJ_OBJ("id" << id), &rec, UBJ::Object(), UBJ::Array("data"))) {

        return false;
    }

    // delete resources

    std::list<uint32_t> resources;

    query("resources", UBJ_OBJ("request" << id), UBJ::Object(), UBJ_ARR("id"), 0, 0,
          [&resources] (bool error, CURSOR cursor) {

        cursor->forEach([&] (UBJ::Object &resource) {

            resources.push_back(resource["id"].toInt());
        });
    });

    if (!resources.empty()) {

        for (auto &it : resources) {

            if (!deleteResource(it)) {

                //return false;
            }
        }
    }

    // delete message

    if (!remove("messages", UBJ_OBJ("id" << id))) {

        return false;
    }

    return true;
}

/**
 * @brief Store::deleteResource
 * @param id
 * @return
 */

bool Store::deleteResource(uint32_t id)
{
    UBJ::Object rec;

    if (!query("resources", UBJ_OBJ("id" << id), &rec, {}, UBJ::Array("data"))) {

        return false;
    }

    // delete data blob

    uint32_t blobId = rec["data"].toInt();

    if (blobId) {

        if (!count("vfs", UBJ_OBJ("data" << blobId))) {

            if (!removeBlob("blob3", blobId)) {

                return false;
            }
        }
    }

    // delete resource

    if (!remove("resources", UBJ_OBJ("id" << id))) {

        return false;
    }

    return true;
}

/**
 * @brief Store::randomColor
 * @return
 */

std::string Store::randomColor()
{
    /*
    std::vector<std::string> colors = {
        "#cd6155",
        "#ec7063",
        "#af7ac5",
        "#a569bd",
        "#5499c7",
        "#5dade2",
        "#48c9b0",
        "#45b39d",
        "#52be80",
        "#58d68d",
        "#f4d03f",
        "#f5b041",
        "#eb984e",
        "#dc7633"
    };
    */

    std::vector<std::string> colors = {
        "#c0392b",
        "#e74c3c",
        "#9b59b6",
        "#8e44ad",
        "#2980b9",
        "#3498db",
        "#1abc9c",
        "#16a085",
        "#27ae60",
        "#2ecc71",
        "#f1c40f",
        "#f39c12",
        "#e67e22",
        "#d35400"
    };

    uint8_t i;

    Crypto::Random::random(&i, 1);

    return colors[i % colors.size()];
}

/**
 * @brief Store::init
 * @param filename
 * @param password
 * @param data
 * @param handler
 * @return
 */

bool Store::init(const std::string &filename, const std::string &password, const UBJ::Object &data, bool handler)
{
    if (!UBJ::Store::init(filename, password, handler)) {

        return false;
    }

    // data blob

    if (!createBlob("blob1", data)) {

        close();

        return false;
    }

    // config blob

    UBJ::Object config;

    if (!createBlob("blob1", config)) {

        close();

        return false;
    }

    // set account data

    m_accountId = data["accountId"].toInt();

    m_accountName = data["accountName"].toStr();

    m_accountPassword = data["accountPassword"].buffer();

    m_publicKey = data["publicKey"];

    m_privateKey = data["privateKey"];

    return true;
}

/**
 * @brief Store::open
 * @param filename
 * @param password
 * @param handler
 * @return
 */

bool Store::open(const std::string &filename, const std::string &password, bool handler)
{
    if (!UBJ::Store::open(filename, password, handler)) {

        return false;
    }

    // load data

    UBJ::Object data;

    if (getBlobData("blob1", DataNodeId, data)) {

        // set account data

        m_accountId = data["accountId"].toInt();

        m_accountName = data["accountName"].toStr();

        m_accountPassword = data["accountPassword"].buffer();

        m_publicKey = data["publicKey"];

        m_privateKey = data["privateKey"];
    }

    return true;
}

/**
 * @brief Store::createBlobTables
 * @return
 */

bool Store::createBlobTables()
{
    if (!createBlobTable("blob1") ||
        !createBlobTable("blob2") ||
        !createBlobTable("blob3")) {

        return false;
    }

    return true;
}

/**
 * @brief Store::createVTables
 * @return
 */

bool Store::createVTables()
{
    if (!UBJ::Store::createVTables()) {

        return false;
    }

    if (!m_vtabs["requests"].create(
                this,
                "blob2",
                "requests",
                Request,
                true,
                {"id",
                 "type",
                 "time",
                 "status",
                 "result",
                 "data"},
                {"INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "BLOB"})) {

        return false;
    }

    if (!m_vtabs["contact_requests"].create(
                this,
                "blob2",
                "contact_requests",
                ContactRequest,
                true,
                {"id",
                 "time",
                 "src",
                 "dst",
                 "name",
                 "phone",
                 "addCode",
                 "color",
                 "result"},
                {"INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "TEXT",
                 "TEXT",
                 "TEXT",
                 "TEXT",
                 "INTEGER"})) {

        return false;
    }

    if (!m_vtabs["add_codes"].create(
                this,
                "blob2",
                "add_codes",
                AddCode,
                true,
                {"time",
                 "addCode"},
                {"INTEGER",
                 "TEXT"})) {

        return false;
    }

    if (!m_vtabs["contacts"].create(
                this,
                "blob2",
                "contacts",
                Contact,
                true,
                {"id",
                 "name",
                 "phone",
                 "inbox",
                 "color",
                 "publicKey"},
                {"INTEGER",
                 "TEXT",
                 "TEXT",
                 "BLOB",
                 "TEXT",
                 "BLOB"})) {

        return false;
    }

    if (!m_vtabs["histories"].create(
                this,
                "blob2",
                "histories",
                History,
                true,
                {"dst",
                 "time"},
                {"INTEGER",
                 "INTEGER"})) {

        return false;
    }

    if (!m_vtabs["messages"].create(
                this,
                "blob2",
                "messages",
                Message,
                true,
                {"id",
                 "src",
                 "dst",
                 "history",
                 "status",
                 "time",
                 "text"},
                {"INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "TEXT",
                 "TEXT"})) {

        return false;
    }

    if (!m_vtabs["resources"].create(
                this,
                "blob2",
                "resources",
                Resource,
                true,
                {"id",
                 "type",
                 "request",
                 "status",
                 "time",
                 "name",
                 "size",
                 "hash",
                 "data"},
                {"INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "TEXT",
                 "INTEGER",
                 "TEXT",
                 "INTEGER"})) {

        return false;
    }

    if (!m_vtabs["vfs"].create(
                this,
                "blob2",
                "vfs",
                Vfs,
                true,
                {"id",
                 "type",
                 "parent",
                 "status",
                 "time",
                 "name",
                 "size",
                 "hash",
                 "data"},
                {"INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "TEXT",
                 "INTEGER",
                 "TEXT",
                 "INTEGER"})) {

        return false;
    }

    if (!m_vtabs["thumbnails"].create(
                this,
                "blob2",
                "thumbnails",
                Thumbnail,
                true,
                {"id",
                 "time",
                 "width",
                 "height",
                 "format",
                 "data"},
                {"TEXT",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER",
                 "INTEGER"})) {

        return false;
    }

    return true;
}

// ============================================================ //

}
