
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

#include "Zway/crypto/random.h"
#include "Zway/memorybuffer.h"
#include "Zway/ubj/store/blob.h"
#include "Zway/ubj/store/cursor.h"
#include "Zway/ubj/store/handler.h"
#include "Zway/ubj/store/store.h"

#include <sstream>

#include "nettle/pbkdf2.h"

namespace Zway { namespace UBJ { namespace Store {

// ============================================================ //

/**
 * @brief UBJStore::create
 * @param filename
 * @param password
 * @return
 */

Store$ Store::create(const std::string &filename, const std::string &password, bool handler)
{
    Store$ p(new Store());

    if (!p->init(filename, password, handler)) {

        return nullptr;
    }

    return p;
}

/**
 * @brief UBJStore::unlock
 * @param filename
 * @param password
 * @return
 */

Store$ Store::unlock(const std::string &filename, const std::string &password, bool handler)
{
    Store$ p(new Store());

    if (!p->open(filename, password, handler)) {

        return nullptr;
    }

    return p;
}

/**
 * @brief UBJStore::UBJStore
 */

Store::Store()
    : m_db(nullptr)
{

}

/**
 * @brief UBJStore::~UBJStore
 */

Store::~Store()
{
    cleanup();

    close();
}

/**
 * @brief UBJStore::init
 * @param filename
 * @param password
 * @return
 */

bool Store::init(const std::string &filename, const std::string &password, bool handler)
{
    FILE* pf = fopen(filename.c_str(), "r");

    if (pf) {

        fclose(pf);

        return false;
    }

    if (sqlite3_open_v2(
            filename.c_str(),
            &m_db,
            SQLITE_OPEN_CREATE |
            SQLITE_OPEN_READWRITE |
            (!m_handler ? SQLITE_OPEN_FULLMUTEX : SQLITE_OPEN_NOMUTEX),
            nullptr) != SQLITE_OK) {

        return false;
    }

    if (handler) {

        m_handler = Handler::create(shared_from_this());

        if (!m_handler) {

            close();

            return false;
        }

        m_handler->start();
    }

    if (!createInternalBlobTables()) {

        close();

        return false;
    }

    if (!createInternalVTables()) {

        close();

        return false;
    }

    // create main blob

    Object main;

    main["version"] = 1;

    if (!createBlob("__main", main, false)) {

        close();

        return false;
    }

    // key generation from password

    // create salt

    MemoryBuffer$ salt = MemoryBuffer::create(nullptr, 16);

    if (!Crypto::Random::random(salt->data(), salt->size(), Crypto::Random::Strong)) {

        close();

        return false;
    }

    // create key

    MemoryBuffer$ pwd = MemoryBuffer::create(nullptr, 32);

    pbkdf2_hmac_sha256(password.size(), (uint8_t*)&password[0], 10000, salt->size(), salt->data(), pwd->size(), pwd->data());

    //memset((void*)&password[0], 0, password.size());

    // create random store key and encrypt with password key

    m_key = MemoryBuffer::create(nullptr, 32);

    if (!Crypto::Random::random(m_key->data(), m_key->size(), Crypto::Random::VeryStrong)) {

        close();

        return false;
    }

    MemoryBuffer$ key = MemoryBuffer::create(nullptr, 32);

    Crypto::AES aes;

    aes.setCtr(MemoryBuffer::create(nullptr, 16));

    aes.setKey(pwd);

    aes.encrypt(m_key, key, 32);

    // encrypt store password

    aes.encrypt(pwd, pwd, 32);

    // create root blob

    if (!createBlob("__main", UBJ_OBJ("salt" << salt << "key" << key << "pwd" << pwd), false)) {

        close();

        return false;
    }

    // create custom blob tables

    if (!createBlobTables()) {

        close();

        return false;
    }

    // create custom virtual tables

    if (!createVTables()) {

        close();

        return false;
    }

    return true;
}

/**
 * @brief UBJStore::open
 * @param filename
 * @param password
 * @return
 */

bool Store::open(const std::string &filename, const std::string &password, bool handler)
{
    if (sqlite3_open_v2(
            filename.c_str(),
            &m_db,
            SQLITE_OPEN_READWRITE |
            (!m_handler ? SQLITE_OPEN_FULLMUTEX : SQLITE_OPEN_NOMUTEX),
            nullptr) != SQLITE_OK) {

        return false;
    }

    if (handler) {

        m_handler = Handler::create(shared_from_this());

        if (!m_handler) {

            close();

            return false;
        }

        m_handler->start();
    }

    if (!createInternalBlobTables()) {

        close();

        return false;
    }

    if (!createInternalVTables()) {

        close();

        return false;
    }

    // load root data

    Object rootData;

    if (!getBlobData("__main", RootNodeId, rootData)) {

        close();

        return false;
    }

    if (!(rootData.hasField("key") && rootData["key"].bufferSize()) ||
        !(rootData.hasField("pwd") && rootData["pwd"].bufferSize())) {

        close();

        return false;
    }

    // key generation from password

    MemoryBuffer$ salt = rootData["salt"].buffer();

    MemoryBuffer$ pwd = MemoryBuffer::create(nullptr, 32);

    pbkdf2_hmac_sha256(password.size(), (uint8_t*)&password[0], 10000, salt->size(), salt->data(), pwd->size(), pwd->data());

    //memset((void*)&password[0], 0, password.size());

    // decrypt store key

    MemoryBuffer$ key = MemoryBuffer::create(rootData["key"].buffer());

    Crypto::AES aes;

    aes.setCtr(MemoryBuffer::create(nullptr, 16));

    aes.setKey(pwd);

    aes.decrypt(key, key, 32);

    // decrypt store password

    MemoryBuffer$ tmp = MemoryBuffer::create(rootData["pwd"].buffer());

    aes.decrypt(tmp, tmp, 32);

    // verify password

    if (memcmp(tmp->data(), pwd->data(), 32)) {

        close();

        return false;
    }

    // set store key

    m_key = key;

    // create custom blob tables

    if (!createBlobTables()) {

        close();

        return false;
    }

    // create custom virtual tables

    if (!createVTables()) {

        close();

        return false;
    }

    // process index

    // ...

    return true;
}

void Store::cleanup()
{
    if (m_db) {

        /*
        Object mainData;

        if (getBlobData("__main", MainNodeId, mainData)) {

            mainData["cleanExit"] = true;

            mainData["acquired"] = false;

            updateBlobData("__main", MainNodeId, mainData, false);
        }
        */
    }
}

/**
 * @brief UBJStore::close
 */

void Store::close()
{
    if (m_handler) {

        m_handler->cancelAndJoin();

        m_handler.reset();
    }

    if (m_db) {

        sqlite3_close(m_db);

        m_db = nullptr;
    }

    m_key.reset();
}

/**
 * @brief UBJStore::vtab
 * @param name
 * @return
 */

VirtualTableModule *Store::vtab(const std::string &name)
{
    if (m_vtabs.find(name) != m_vtabs.end()) {

        return &m_vtabs[name];
    }

    return nullptr;
}

/**
 * @brief UBJStore::createBlobTables
 * @return
 */

bool Store::createBlobTables()
{
    return true;
}

/**
 * @brief UBJStore::createVTables
 * @return
 */

bool Store::createVTables()
{
    return true;
}

/**
 * @brief UBJStore::createBlobTable
 * @param name
 * @return
 */

bool Store::createBlobTable(const std::string &name)
{
    std::string sql =
            "CREATE TABLE IF NOT EXISTS " + name + " ("\
                "id INTEGER PRIMARY KEY," \
                "mode INTEGER,"\
                "size INTEGER,"\
                "salt BLOB,"\
                "data BLOB"\
            ");";

    char* errmsg;

    sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errmsg);

    if (errmsg) {

        m_err = errmsg;

        sqlite3_free(errmsg);

        return false;
    }

    return true;
}

/**
 * @brief UBJStore::createInternalBlobTables
 * @return
 */

bool Store::createInternalBlobTables()
{
    // create main blob table

    if (!createBlobTable("__main")) {

        return false;
    }

    // create index blob and virtual table

    if (!createBlobTable("__indexes")) {

        return false;
    }

    return true;
}

/**
 * @brief UBJStore::createInternalVTables
 * @return
 */

bool Store::createInternalVTables()
{
    if (!m_vtabs["_$indexes"].create(
                shared_from_this(),
                "__indexes",
                "_$indexes",
                0,
                false,
                {"name",
                 "ids",
                 "idx",
                 "status"},
                {"STRING",
                 "BLOB",
                 "BLOB",
                 "INTEGER"})) {

        return false;
    }

    return true;
}

/**
 * @brief UBJStore::beginTransaction
 * @param exclusive
 * @param callback
 * @return
 */

Action$ Store::beginTransaction(
        bool exclusive,
        const BeginTransaction::Callback &callback,
        bool async)
{
    Action$ action(new BeginTransaction(shared_from_this(), exclusive, callback));

    processAction(action);

    return action;
}

/**
 * @brief UBJStore::endTransaction
 * @param callback
 * @return
 */

Action$ Store::endTransaction(
        const EndTransaction::Callback &callback)
{
    Action$ action(new EndTransaction(shared_from_this(), callback));

    processAction(action);

    return action;
}

/**
 * @brief UBJStore::query
 * @param table
 * @param query
 * @param order
 * @param fieldsToReturn
 * @param limit
 * @param offset
 * @return
 */

void Store::query(
        const std::string &table,
        const Object &query,
        const Object &order,
        const Array &fieldsToReturn,
        int32_t limit,
        int32_t offset,
        const QueryAction::Callback &callback)
{
    if (m_vtabs.find(table) != m_vtabs.end()) {

        m_vtabs[table].updateIds();
    }

    Action$ action(new QueryAction(
                       shared_from_this(),
                       table,
                       query,
                       order,
                       fieldsToReturn,
                       limit,
                       offset,
                       callback));

    processAction(action);
}

/**
 * @brief Store::query
 * @param table
 * @param query
 * @param callback
 */

void Store::query(
        const std::string &table,
        const Object &query,
        const QueryAction::Callback &callback)
{
    Store::query(table, query, {}, {}, 0, 0, callback);
}

/**
 * @brief UBJStore::query
 * @param table
 * @param query
 * @param result
 * @param order
 * @param fieldsToReturn
 * @param limit
 * @param offset
 * @return
 */

bool Store::query(
        const std::string &table,
        const Object &query,
        Object *result,
        const Object &order,
        const Array &fieldsToReturn,
        int32_t offset)
{
    bool res = false;

    if (result) {

        Store::query(table, query, order, fieldsToReturn, 1, offset,
              [&] (bool error, Cursor$ cursor) {

            if (!error) {

                if (cursor->next(*result)) {

                    res = true;
                }
            }
        });
    }

    return res;
}

/**
 * @brief Store::query
 * @param table
 * @param query
 * @param result
 * @param order
 * @param fieldsToReturn
 * @param limit
 * @param offset
 * @return
 */

bool Store::query(
        const std::string &table,
        const Object &query,
        std::list<Object> &result,
        const Object &order,
        const Array &fieldsToReturn,
        int32_t limit,
        int32_t offset)
{
    bool res = true;

    Store::query(table, query, order, fieldsToReturn, limit, offset,
          [&result, &res] (bool error, Cursor$ cursor) {

        if (!error) {

            cursor->forEach([&] (Object &item) {

                result.push_back(item);
            });
        }
        else {

            res = false;
        }
    });

    return res;
}

/**
 * @brief UBJStore::count
 * @param table
 * @param where
 * @return
 */

uint32_t Store::count(
        const std::string &table,
        const Object &where)
{
    uint32_t res = -1;

    if (m_vtabs.find(table) != m_vtabs.end()) {

        m_vtabs[table].updateIds();
    }

    auto callback = [&res] (bool error, uint32_t count) {

        if (!error) {

            res = count;
        }
    };

    Action$ action(new CountAction(
                       shared_from_this(),
                       table,
                       where,
                       callback));

    processAction(action);

    return res;
}

/**
 * @brief UBJStore::insert
 * @param table
 * @param insert
 * @return
 */

uint64_t Store::insert(
        const std::string &table,
        const Object &insert)
{
    uint64_t res = 0;

    auto callback = [&res] (bool error, const std::deque<uint64_t> &rowIds) {

        if (!error) {

            res = rowIds[0];
        }
    };

    Action$ action(new InsertAction(
                       shared_from_this(),
                       table,
                       insert,
                       callback));

    processAction(action);

    return res;
}

/**
 * @brief UBJStore::update
 * @param table
 * @param update
 * @param where
 * @return
 */

uint32_t Store::update(
        const std::string &table,
        const Object &update,
        const Object &where)
{
    uint32_t res = 0;

    if (m_vtabs.find(table) != m_vtabs.end()) {

        std::list<uint64_t> rowIds;

        query(table, where, {}, {"rowid"}, 0, 0, [&res,&rowIds] (bool error, Cursor$ cursor) {

            if (!error) {

                cursor->forEach([&] (Object &item) {

                    rowIds.push_back(item["rowid"].toLong());
                });
            }
            else {

                res = -1;
            }
        });

        if (!rowIds.empty()) {

            beginTransaction(true, [&] (bool error) {

                if (!error) {

                    std::string blobTable = m_vtabs[table].blobTableName();

                    for (auto &id : rowIds) {

                        Object data;

                        if (getBlobData(blobTable, id, data)) {

                            for (auto &it : update) {

                                data[it.first] = it.second;
                            }

                            if (!updateBlobData(blobTable, id, data)) {


                            }
                        }
                    }

                    res = rowIds.size();
                }
                else {

                    res = -1;
                }
            });
        }
    }
    else {

        auto callback = [&res] (bool error, uint32_t numRows) {

            if (!error) {

                res = numRows;
            }
            else {

                res = -1;
            }
        };

        Action$ action(new UpdateAction(
                           shared_from_this(),
                           table,
                           update,
                           where,
                           callback));

        processAction(action);
    }

    return res;
}

/**
 * @brief UBJStore::remove
 * @param table
 * @param where
 * @return
 */

uint32_t Store::remove(
        const std::string &table,
        const Object &where)
{
    uint32_t res = 0;

    if (m_vtabs.find(table) != m_vtabs.end()) {

        std::list<uint64_t> rowIds;

        query(table, where, {}, {"rowid"}, 0, 0, [&res,&rowIds] (bool error, Cursor$ cursor) {

            if (!error) {

                cursor->forEach([&] (Object &item) {

                    rowIds.push_back(item["rowid"].toLong());
                });
            }
            else {

                res = -1;
            }
        });

        if (!rowIds.empty()) {

            beginTransaction(true, [&] (bool error) {

                if (!error) {

                    std::string blobTable = m_vtabs[table].blobTableName();

                    for (auto &id : rowIds) {

                        if (!removeBlob(blobTable, id)) {

                            // ...
                        }
                    }

                    res = rowIds.size();
                }
                else {

                    res = -1;
                }
            });
        }
    }
    else {

        auto callback = [&] (bool error, uint32_t numRows) {

            if (!error) {

                res = numRows;
            }
        };

        Action$ action(new RemoveAction(
                           shared_from_this(),
                           table,
                           where,
                           callback));

        processAction(action);
    }

    return res;
}

/**
 * @brief UBJStore::truncate
 * @param table
 * @return
 */

uint32_t Store::truncate(const std::string &table)
{
    return remove(table, {});
}

/**
 * @brief UBJStore::queryInt
 * @param table
 * @param field
 * @param query
 * @param order
 * @param offset
 * @return
 */

int64_t Store::queryInt(
        const std::string &table,
        const std::string &field,
        const Object &query,
        const Object &order,
        int32_t offset)
{
    Object rec;

    if (!Store::query(table, query, &rec, order, UBJ_ARR(field), offset)) {

        return 0;
    }

    return rec[field].toLong();
}

/**
 * @brief Store::readBlob
 * @param table
 * @param id
 * @param callback
 * @return
 */

bool Store::readBlob(
        const std::string &table,
        uint64_t id,
        const ReadBlobAction::Callback &callback)
{
    Action$ action(new ReadBlobAction(
                       shared_from_this(),
                       table,
                       id,
                       callback));

    return processAction(action);
}

/**
 * @brief Store::writeBlob
 * @param table
 * @param id
 * @param callback
 * @return
 */

bool Store::writeBlob(
        const std::string &table,
        uint64_t id,
        const WriteBlobAction::Callback &callback)
{
    Action$ action(new WriteBlobAction(
                       shared_from_this(),
                       table,
                       id,
                       callback));

    return processAction(action);
}

/**
 * @brief UBJStore::createBlob
 * @param table
 * @param size
 * @param encrypt
 * @return
 */

uint64_t Store::createBlob(const std::string &table, uint32_t size, bool encrypt)
{
    Object rec;

    rec["mode"] = encrypt;
    rec["size"] = size;
    rec["data"] = UBJ_OBJ("$zeroBlob" << size);

    MemoryBuffer$ salt;

    if (encrypt) {

        salt = MemoryBuffer::create(nullptr, 16);

        if (!Crypto::Random::random(salt->data(), 12, Crypto::Random::Strong)) {

            return 0;
        }

        rec["salt"] = salt;
    }

    uint64_t blobId = insert(table, rec);

    if (!blobId) {

        return 0;
    }

    return blobId;
}

/**
 * @brief UBJStore::createBlob
 * @param table
 * @param data
 * @param encrypt
 * @return
 */

uint64_t Store::createBlob(const std::string &table, MemoryBuffer$ data, bool encrypt)
{
    Object rec;

    rec["mode"] = encrypt;

    if (data) {

        rec["size"] = data->size();
    }

    if (data && !encrypt) {

        rec["data"] = data;
    }

    uint64_t blobId = insert(table, rec);

    if (!blobId) {

        return 0;
    }

    if (data && encrypt && !updateBlobData(table, blobId, data, encrypt)) {

        removeBlob(table, blobId);

        return 0;
    }

    return blobId;
}

/**
 * @brief UBJStore::createBlob
 * @param table
 * @param data
 * @param encrypt
 * @return
 */

uint64_t Store::createBlob(const std::string &table, const Object &data, bool encrypt)
{
    MemoryBuffer$ buf = Value::write(data);

    if (!buf) {

        return 0;
    }

    uint64_t blobId = createBlob(table, buf, encrypt);

    if (!blobId) {

        return 0;
    }

    return blobId;
}

/**
 * @brief UBJStore::openBlob
 * @param table
 * @param id
 * @return
 */

Blob$ Store::openBlob(const std::string &table, uint64_t id, bool readOnly, bool meta, bool mode, uint32_t size, MemoryBuffer$ salt)
{
    Blob$ blob(new Blob(shared_from_this()));

    if (blob->open(table, id, readOnly, meta, mode, size, salt)) {

        return blob;
    }

    return nullptr;
}

/**
 * @brief UBJStore::updateBlobData
 * @param table
 * @param id
 * @param data
 * @param encrypt
 * @return
 */

uint32_t Store::updateBlobData(const std::string &table, uint64_t id, MemoryBuffer$ data, bool encrypt)
{
    if (!data) {

        return 0;
    }

    Object rec;

    rec["mode"] = encrypt;
    rec["size"] = data->size();

    MemoryBuffer$ salt;

    if (encrypt) {

        salt = MemoryBuffer::create(nullptr, 16);

        if (!Crypto::Random::random(salt->data(), 12, Crypto::Random::Strong)) {

            return 0;
        }

        rec["data"] = UBJ_OBJ("$zeroBlob" << data->size());
        rec["salt"] = salt;
    }
    else {

        rec["data"] = data;
        rec["salt"] = UBJ_OBJ("$bindNull" << 1);
    }

    uint64_t res = update(table, rec, UBJ_OBJ("rowid" << id));

    // ...

    if (encrypt) {

        writeBlob(table, id, [&data] (bool error, Blob$ blob) {

            if (!error) {

                if (!blob->write(data->data(), data->size())) {

                }
            }
        });
    }

    return id;
}

/**
 * @brief UBJStore::updateBlobData
 * @param table
 * @param id
 * @param data
 * @param encrypt
 * @return
 */

uint32_t Store::updateBlobData(const std::string &table, uint64_t id, const Object &data, bool encrypt)
{
    MemoryBuffer$ buf = Value::write(data);

    if (!buf) {

        return 0;
    }

    if (!updateBlobData(table, id, buf, encrypt)) {

        return 0;
    }

    return id;
}

/**
 * @brief UBJStore::removeBlob
 * @param table
 * @param id
 * @return
 */

bool Store::removeBlob(const std::string &table, uint64_t id)
{
    // TODO zero blob

    uint64_t res = remove(table, UBJ_OBJ("rowid" << id));

    if (res <= 0) {

        return false;
    }

    return true;
}

/**
 * @brief UBJStore::getBlobData
 * @param table
 * @param id
 * @return
 */

MemoryBuffer$ Store::getBlobData(const std::string &table, uint64_t id)
{
    MemoryBuffer$ buf;

    readBlob(table, id, [&buf] (bool error, Blob$ blob) {

        if (!error) {

            buf = MemoryBuffer::create(nullptr, blob->size());

            if (buf) {

                if (!blob->read(buf->data(), buf->size())) {


                }
            }
        }
    });

    return buf;
}

/**
 * @brief UBJStore::getBlobData
 * @param table
 * @param id
 * @param data
 * @return
 */

bool Store::getBlobData(const std::string &table, uint64_t id, Object &data)
{
    MemoryBuffer$ buf = getBlobData(table, id);

    if (!buf) {

        return false;
    }

    if (!Value::read(data, buf)) {

        return false;
    }

    return true;
}

/**
 * @brief UBJStore::getBlobInfo
 * @param table
 * @param id
 * @param info
 * @return
 */

bool Store::getBlobInfo(const std::string &table, uint64_t id, Object &info)
{
    bool res = false;

    Array fieldsToReturn = UBJ_ARR("mode" << "size" << "salt");

    query(table, UBJ_OBJ("rowid" << id), Object(), fieldsToReturn, 0, 0,
          [&res, &info] (bool error, Cursor$ cursor) {

        if (!error) {

            if (cursor->next(info)) {

                res = true;
            }
        }
    });

    return res;
}

/**
 * @brief UBJStore::getBlobIds
 * @param table
 * @return
 */

Array Store::getBlobIds(const std::string &table)
{
    Array ids;

    query(table, Object(), Object(), UBJ_ARR("id"), 0, 0,
          [&ids] (bool error, Cursor$ cursor) {

        if (!error) {

            cursor->forEach([&] (Object &item) {

                ids << item["id"];
            });
        }
    });

    return ids;
}

/**
 * @brief UBJStore::db
 * @return
 */

sqlite3 *Store::db()
{
    return m_db;
}

/**
 * @brief UBJStore::err
 * @return
 */

std::string Store::err()
{
    return m_err;
}

/**
 * @brief UBJStore::processAction
 * @param action
 * @return
 */

bool Store::processAction(Action$ action)
{
    if (m_handler && m_handler->threadId() != std::this_thread::get_id()) {

        m_handler->post(action);

        action->wait();
    }
    else {

        action->execute();
    }

    return true;
}

/**
 * @brief UBJStore::getIndex
 * @param table
 * @param index
 * @return
 */

bool Store::getIndex(const std::string &table, Object &index)
{
    if (!query("_$indexes", UBJ_OBJ("name" << table), &index)) {

        return false;
    }

    return true;
}

/**
 * @brief UBJStore::setIndex
 * @param table
 * @param index
 * @return
 */

bool Store::setIndex(const std::string &table, const Array &ids, const Object &index)
{
    if (count("_$indexes", UBJ_OBJ("name" << table))) {

        if (!update("_$indexes", UBJ_OBJ("ids" << ids << "idx" << index), UBJ_OBJ("name" << table))) {

            return false;
        }
    }
    else {

        if (!insert("_$indexes", UBJ_OBJ("name" << table << "ids" << ids << "idx" << index))) {

            return false;
        }
    }

    return true;
}

/**
 * @brief Store::vacuum
 * @return
 */

bool Store::vacuum()
{
    std::string sql = "VACUUM";

    char* errmsg;

    sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errmsg);

    if (errmsg) {

        m_err = errmsg;

        sqlite3_free(errmsg);

        return false;
    }

    return true;
}

// ============================================================ //

}}}
