
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

#include "Zway/store/ubjstore.h"

#include <cstring>
#include <sstream>

namespace Zway { namespace UBJ {

// ============================================================ //

/**
 * @brief UBJStore::create
 * @param filename
 * @param password
 * @return
 */

Store::Pointer Store::create(const std::string &filename, const std::string &password, bool handler)
{
    Pointer p(new Store());

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

Store::Pointer Store::unlock(const std::string &filename, const std::string &password, bool handler)
{
    Pointer p(new Store());

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

        m_handler = StoreHandler::create(shared_from_this());

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

    BUFFER salt = Buffer::create(nullptr, 16);

    if (!Crypto::Random::random(salt->data(), salt->size(), Crypto::Random::Strong)) {

        close();

        return false;
    }

    // create key

    BUFFER pwd = Buffer::create(nullptr, 32);

    pbkdf2_hmac_sha256(password.size(), (uint8_t*)&password[0], 10000, salt->size(), salt->data(), pwd->size(), pwd->data());

    //memset((void*)&password[0], 0, password.size());

    // create random store key and encrypt with password key

    m_key = Buffer::create(nullptr, 32);

    if (!Crypto::Random::random(m_key->data(), m_key->size(), Crypto::Random::VeryStrong)) {

        close();

        return false;
    }

    BUFFER key = Buffer::create(nullptr, 32);

    Crypto::AES aes;

    aes.setCtr(Buffer::create(nullptr, 16));

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

        m_handler = StoreHandler::create(shared_from_this());

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

    BUFFER salt = rootData["salt"].buffer();

    BUFFER pwd = Buffer::create(nullptr, 32);

    pbkdf2_hmac_sha256(password.size(), (uint8_t*)&password[0], 10000, salt->size(), salt->data(), pwd->size(), pwd->data());

    //memset((void*)&password[0], 0, password.size());

    // decrypt store key

    BUFFER key = Buffer::create(rootData["key"].buffer());

    Crypto::AES aes;

    aes.setCtr(Buffer::create(nullptr, 16));

    aes.setKey(pwd);

    aes.decrypt(key, key, 32);

    // decrypt store password

    BUFFER tmp = Buffer::create(rootData["pwd"].buffer());

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
                this,
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

Store::ACTION Store::beginTransaction(
        bool exclusive,
        const BeginTransaction::Callback &callback,
        bool async)
{
    BeginTransaction::Pointer action(new BeginTransaction(shared_from_this(), exclusive, callback));

    processAction(action);

    return action;
}

/**
 * @brief UBJStore::endTransaction
 * @param callback
 * @return
 */

Store::ACTION Store::endTransaction(
        const Store::EndTransaction::Callback &callback)
{
    ACTION action(new EndTransaction(shared_from_this(), callback));

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

    ACTION action(new QueryAction(
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
        const Store::QueryAction::Callback &callback)
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
              [&] (bool error, CURSOR cursor) {

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
          [&result, &res] (bool error, CURSOR cursor) {

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

    ACTION action(new CountAction(
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

    ACTION action(new InsertAction(
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

        query(table, where, {}, {"rowid"}, 0, 0, [&res,&rowIds] (bool error, CURSOR cursor) {

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

        ACTION action(new UpdateAction(
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

        query(table, where, {}, {"rowid"}, 0, 0, [&res,&rowIds] (bool error, CURSOR cursor) {

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

        ACTION action(
                    new RemoveAction(
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

bool Store::readBlob(const std::string &table,
        uint64_t id,
        const Store::ReadBlobAction::Callback &callback)
{
    ACTION action(
                new ReadBlobAction(
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
        const Store::WriteBlobAction::Callback &callback)
{
    ACTION action(
                new WriteBlobAction(
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

    BUFFER salt;

    if (encrypt) {

        salt = Buffer::create(nullptr, 16);

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

uint64_t Store::createBlob(const std::string &table, BUFFER data, bool encrypt)
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
    BUFFER buf = Value::write(data);

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

Store::BLOB Store::openBlob(const std::string &table, uint64_t id, bool readOnly, bool meta, bool mode, uint32_t size, BUFFER salt)
{
    BLOB blob(new Blob(this));

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

uint32_t Store::updateBlobData(const std::string &table, uint64_t id, BUFFER data, bool encrypt)
{
    if (!data) {

        return 0;
    }

    Object rec;

    rec["mode"] = encrypt;
    rec["size"] = data->size();

    BUFFER salt;

    if (encrypt) {

        salt = Buffer::create(nullptr, 16);

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

        writeBlob(table, id, [&data] (bool error, BLOB blob) {

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
    BUFFER buf = Value::write(data);

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

BUFFER Store::getBlobData(const std::string &table, uint64_t id)
{
    BUFFER buf;

    readBlob(table, id, [&buf] (bool error, BLOB blob) {

        if (!error) {

            buf = Buffer::create(nullptr, blob->size());

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
    BUFFER buf = getBlobData(table, id);

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
          [&res, &info] (bool error, CURSOR cursor) {

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
          [&ids] (bool error, CURSOR cursor) {

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

bool Store::processAction(Store::ACTION action)
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
// Action
// ============================================================ //

/**
 * @brief UBJStore::Action::Action
 * @param store
 * @param table
 */

Store::Action::Action(const Store::Pointer &store, const std::string &table)
    : m_store(store),
      m_table(table),
      m_vtab(false),
      m_stmt(nullptr)
{
    m_done = false;

    m_vtab = store->m_vtabs.find(table) != store->m_vtabs.end();
}

/**
 * @brief UBJStore::Action::~Action
 */

Store::Action::~Action()
{
    finish();
}

/**
 * @brief UBJStore::Action::step
 * @return
 */

int32_t Store::Action::step()
{
    if (m_stmt) {

        return sqlite3_step(m_stmt);
    }

    return 0;
}

/**
 * @brief UBJStore::Action::execute
 * @return
 */

bool Store::Action::execute()
{
    return true;
}

/**
 * @brief UBJStore::Action::reset
 */

void Store::Action::reset()
{
    if (m_stmt) {

        sqlite3_reset(m_stmt);
    }
}

/**
 * @brief UBJStore::Action::finish
 */

void Store::Action::finish()
{
    if (m_stmt) {

        sqlite3_finalize(m_stmt);

        m_stmt = nullptr;
    }
}

/**
 * @brief UBJStore::Action::wait
 */

void Store::Action::wait()
{
    {
        MutexLocker lock(m_done);

        if (m_done) {

            return;
        }
    }

    std::unique_lock<std::mutex>lock(m_waitMutex);

    m_waitCondition.wait(lock);
}

/**
 * @brief UBJStore::Action::stmt
 * @return
 */

sqlite3_stmt *Store::Action::stmt()
{
    return m_stmt;
}

/**
 * @brief UBJStore::Action::table
 * @return
 */

std::string Store::Action::table()
{
    return m_table;
}

/**
 * @brief UBJStore::Action::sql
 * @param values
 * @return
 */

std::string Store::Action::sql(bool expanded)
{
    if (expanded) {

        char* ps = sqlite3_expanded_sql(m_stmt);

        std::string s = ps;

        sqlite3_free(ps);

        return s;
    }
    else {

        return m_sql;
    }
}

/**
 * @brief UBJStore::Action::prepareStmt
 * @param sql
 * @return
 */

bool Store::Action::prepareStmt(const std::string &sql)
{
    if (m_stmt) {

        return false;
    }

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(m_store->db(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {

        m_store->m_err = sqlite3_errmsg(m_store->db());

        return false;
    }

    m_stmt = stmt;

    m_sql = sql;

    bindUbjToStmt();

    return true;
}

/**
 * @brief UBJStore::Action::fieldsToReturnPart
 * @param fieldsToReturn
 * @return
 */

std::string Store::Action::fieldsToReturnPart(const Array &fieldsToReturn)
{
    std::string res;

    for (auto it = fieldsToReturn.begin(); it != fieldsToReturn.end(); ) {

        res += it->toStr();

        if (++it != fieldsToReturn.end()) {

            res += ",";
        }
    }

    return res;
}

/**
 * @brief UBJStore::Action::wherePart
 * @param query
 * @return
 */

std::string Store::Action::wherePart(const Object &query)
{
    std::string res;

    std::map<std::string, std::string> ops = {
        {"$ne", "<>?"},
        {"$gt", ">?"},
        {"$le", "<=?"},
        {"$lt", "<?"},
        {"$ge", ">=?"},
        {"$like", " like ?"}
    };

    for (auto it = query.begin(); it != query.end();) {

        const Value &v = it->second;

        std::string key = it->first;

        if (v.type() == UBJ_ARRAY) {

            if (key == "$or") {

                if (v.numItems() > 1) {

                    res += "(";

                    for (uint32_t i=0; i<v.numItems(); ++i) {

                        res += wherePart(v[i]);

                        if (i < v.numItems() - 1) {

                            res += " OR ";
                        }
                    }

                    res += ")";
                }
                else
                if (v.numItems() > 0) {

                    res += wherePart(v[0]);
                }
            }
        }
        else
        if (v.type() == UBJ_OBJECT && v.numItems()) {

            for (auto &it : ops) {

                if (v.hasField(it.first)) {

                    res += key + it.second;

                    m_bound << v[it.first];

                    break;
                }
            }
        }
        else {

            res += key + "=?";

            m_bound << v;
        }

        if (++it != query.end()) {

            res += " AND ";
        }
    }

    return res;
}

/**
 * @brief UBJStore::Action::orderPart
 * @param order
 * @return
 */

std::string Store::Action::orderPart(const Object &order)
{
    std::string res;

    for (auto &it : order) {

        res += it.first + (it.second.toInt() > 0 ? " ASC " : " DESC ");
    }

    return res;
}

/**
 * @brief UBJStore::Action::insertPart
 * @param insert
 * @return
 */

std::string Store::Action::insertPart(const Object &insert)
{
    std::string part1;
    std::string part2;

    for (auto it = insert.begin(); it != insert.end();) {

        m_bound.push_back(it->second);

        part1 += it->first;
        part2 += "?";

        if (++it != insert.end()) {

            part1 += ",";
            part2 += ",";
        }
    }

    std::string res;

    if (!part1.empty()) {

        res = "(" + part1 + ") VALUES(" + part2 + ")";
    }

    return res;
}

/**
 * @brief UBJStore::Action::updatePart
 * @param update
 * @return
 */

std::string Store::Action::updatePart(const Object &update)
{
    std::string res;

    for (auto it = update.begin(); it != update.end();) {

        m_bound.push_back(it->second);

        res += it->first + "=?";

        if (++it != update.end()) {

            res += ",";
        }
    }

    return res;
}

/**
 * @brief UBJStore::Action::bindUbjToStmt
 * @return
 */

int32_t Store::Action::bindUbjToStmt()
{
    int32_t i=0;

    for (auto &value : m_bound) {

        BUFFER buf = value.buffer();

        switch (value.type()) {
        case UBJ_OBJECT:

            if (value.hasField("$zeroBlob")) {

                uint32_t size = value["$zeroBlob"].toInt();

                sqlite3_bind_zeroblob(m_stmt, ++i, value["$zeroBlob"].toLong());
            }
            else
            if (value.hasField("$bindNull")) {

                sqlite3_bind_null(m_stmt, ++i);
            }
            else
            if (m_vtab) {

                // serialize object and store it as blob

                BUFFER buf = Value::write(value);

                if (buf) {

                    sqlite3_bind_blob(m_stmt, ++i, buf->data(), buf->size(), SQLITE_TRANSIENT);
                }
            }

            break;

        case UBJ_ARRAY:

            if (value.bufferSize()) {

                sqlite3_bind_blob(m_stmt, ++i, buf->data(), buf->size(), nullptr);
            }
            else
            if (m_vtab) {

                // serialize array and store it as blob

                BUFFER buf = Value::write(value);

                if (buf) {

                    sqlite3_bind_blob(m_stmt, ++i, buf->data(), buf->size(), SQLITE_TRANSIENT);
                }
            }

            break;

        case UBJ_BOOL_TRUE:
            sqlite3_bind_int(m_stmt, ++i, 1);
            break;
        case UBJ_BOOL_FALSE:
            sqlite3_bind_int(m_stmt, ++i, 0);
            break;
        case UBJ_INT32:
            sqlite3_bind_int(m_stmt, ++i, *((int32_t*)buf->data()));
            break;
        case UBJ_INT64:
            sqlite3_bind_int64(m_stmt, ++i, *((int64_t*)buf->data()));
            break;
        case UBJ_FLOAT32:
        case UBJ_FLOAT64:
            sqlite3_bind_double(m_stmt, ++i, *((double*)buf->data()));
            break;
        case UBJ_STRING:
            sqlite3_bind_text(m_stmt, ++i, (char*)buf->data(), buf->size(), nullptr);
            break;
        default:
            sqlite3_bind_null(m_stmt, ++i);
            break;
        }
    }

    return i;
}

/**
 * @brief UBJStore::Action::rowToUbj
 * @param obj
 * @return
 */

bool Store::Action::rowToUbj(Object &obj)
{
    obj.clear();

    int32_t numCols = sqlite3_column_count(m_stmt);

    for (int32_t i=0; i<numCols; ++i) {

        int32_t type = sqlite3_column_type(m_stmt, i);

        std::string name = sqlite3_column_name(m_stmt, i);

        if (type == SQLITE_INTEGER) {

            obj[name] = (int64_t)sqlite3_column_int64(m_stmt, i);
        }
        else
        if (type == SQLITE_TEXT) {

            int32_t numBytes = sqlite3_column_bytes(m_stmt, i);

            BUFFER buf = Buffer::create(sqlite3_column_text(m_stmt, i), numBytes);

            if (!buf) {

                return false;
            }

            obj[name] = std::string((char*)buf->data(), buf->size());
        }
        else
        if (type == SQLITE_BLOB) {

            int32_t numBytes = sqlite3_column_bytes(m_stmt, i);

            BUFFER buf = Buffer::create((uint8_t*)sqlite3_column_blob(m_stmt, i), numBytes);

            if (!buf) {

                return false;
            }

            if (!m_vtab) {

                obj[name] = buf;
            }
            else {

                Value::read(obj[name], buf);
            }
        }
    }

    return true;
}

/**
 * @brief UBJStore::Action::notify
 */

void Store::Action::notify()
{
    std::unique_lock<std::mutex> lock(m_waitMutex);

    m_waitCondition.notify_one();

    {
        MutexLocker lock(m_done);

        m_done = true;
    }
}

// ============================================================ //
// Cursor
// ============================================================ //

/**
 * @brief UBJStore::Cursor::create
 * @param action
 * @return
 */

Store::CURSOR Store::Cursor::create(ACTION action)
{
    CURSOR cursor = CURSOR(new Cursor(action));

    return cursor;
}

/**
 * @brief UBJStore::Cursor::Cursor
 * @param action
 */

Store::Cursor::Cursor(ACTION action)
    : m_action(action)
{

}

/**
 * @brief UBJStore::Cursor::~Cursor
 */

Store::Cursor::~Cursor()
{

}

/**
 * @brief UBJStore::Cursor::forEach
 * @param fn
 */

void Store::Cursor::forEach(const std::function<void (Object&)> &fn)
{
    reset();

    for (;;) {

        Object item;

        if (!next(item)) {

            return;
        }

        fn(item);
    }
}

/**
 * @brief UBJStore::Cursor::forEach
 * @param fn
 * @return
 */

bool Store::Cursor::forEach(const std::function<bool (Object&)> &fn)
{
    reset();

    for (;;) {

        Object item;

        if (!next(item)) {

            return false;
        }

        if (!fn(item)) {

            return false;
        }
    }

    return true;
}

/**
 * @brief UBJStore::Cursor::next
 * @param item
 * @return
 */

bool Store::Cursor::next(Object &item)
{
    if (!m_action) {

        return false;
    }

    if (m_action->step() == SQLITE_ROW) {

        if (m_action->rowToUbj(item)) {

            return true;
        }
    }

    return false;
}

/**
 * @brief UBJStore::Cursor::reset
 */

void Store::Cursor::reset()
{
    if (m_action) {

        m_action->reset();
    }
}

/**
 * @brief UBJStore::Cursor::action
 * @return
 */

Store::ACTION Store::Cursor::action()
{
    return m_action;
}

/**
 * @brief UBJStore::Cursor::getInt
 * @param index
 * @return
 */

int32_t Store::Cursor::getInt(int32_t index)
{
    if (m_action) {

        return sqlite3_column_int(m_action->stmt(), index);
    }

    return 0;
}

/**
 * @brief UBJStore::Cursor::getLong
 * @param index
 * @return
 */

int64_t Store::Cursor::getLong(int32_t index)
{
    if (m_action) {

        return sqlite3_column_int64(m_action->stmt(), index);
    }

    return 0;
}

/**
 * @brief UBJStore::BeginTransaction::BeginTransaction
 * @param exclusive
 */

Store::BeginTransaction::BeginTransaction(
        const Store::Pointer &store,
        bool exclusive,
        const Callback &callback)
    : Action(store),
      m_exclusive(exclusive),
      m_callback(callback)
{
    std::stringstream sql;

    sql << "BEGIN " << (exclusive ? "EXCLUSIVE" : "IMMEDIATE") << " TRANSACTION";

    m_sql = sql.str();
}

/**
 * @brief UBJStore::BeginTransaction::execute
 * @return
 */

bool Store::BeginTransaction::execute()
{
    if (!prepareStmt(m_sql)) {

        if (m_callback) {

            m_callback(true);

            notify();
        }

        return false;
    }

    if (step() == SQLITE_DONE) {

        //std::cerr << m_sql << " - SUCCESS\n";

        finish();

        if (m_callback) {

            m_callback(false);

            m_store->endTransaction();
        }

        notify();

        return true;
    }
    else {

        //std::cerr << m_sql << " - FAILURE - " << sqlite3_errmsg(m_store->db()) << "\n";

        finish();

        if (m_callback) {

            m_callback(true);
        }

        notify();

        return false;
    }
}

/**
 * @brief UBJStore::BeginTransaction::exclusive
 * @return
 */

bool Store::BeginTransaction::exclusive()
{
    return m_exclusive;
}

/**
 * @brief UBJStore::EndTransaction::EndTransaction
 * @param store
 * @param callback
 */

Store::EndTransaction::EndTransaction(
        const Store::Pointer &store,
        const Store::EndTransaction::Callback &callback)
    : Action(store),
      m_callback(callback)
{
    m_sql = "END TRANSACTION";
}

/**
 * @brief UBJStore::EndTransaction::execute
 * @return
 */

bool Store::EndTransaction::execute()
{
    if (!prepareStmt(m_sql)) {

        if (m_callback) {

            m_callback(true);

            notify();
        }

        return false;
    }

    if (step() == SQLITE_DONE) {

        //std::cerr << m_sql << " - SUCCESS\n";

        finish();

        if (m_callback) {

            m_callback(false);
        }

        notify();

        return true;
    }
    else {

        //std::cerr << m_sql << " - FAILURE - " << sqlite3_errmsg(m_store->db()) << "\n";

        finish();

        if (m_callback) {

            m_callback(true);
        }

        notify();

        return false;
    }
}

/**
 * @brief UBJStore::QueryAction::QueryAction
 * @param store
 * @param table
 * @param query
 * @param order
 * @param fieldsToReturn
 * @param limit
 * @param offset
 * @param callback
 */

Store::QueryAction::QueryAction(
        const Store::Pointer &store,
        const std::string &table,
        const Object &query,
        const Object &order,
        const Array &fieldsToReturn,
        int32_t limit,
        int32_t offset,
        const Store::QueryAction::Callback &callback)
    : Action(store, table),
      m_callback(callback)
{
    std::stringstream sql;

    sql << "SELECT ";

    std::string fieldsToReturnStr = fieldsToReturnPart(fieldsToReturn);

    if (!fieldsToReturnStr.empty()) {

        sql << fieldsToReturnStr << " FROM " << table;
    }
    else {

        sql << "* FROM " << table;
    }

    std::string whereStr = wherePart(query);

    if (!whereStr.empty()) {

        sql << " WHERE " << whereStr;
    }

    std::string orderStr = orderPart(order);

    if (!orderStr.empty()) {

        sql << " ORDER BY " << orderStr;
    }

    if (limit > 0) {

        sql << " LIMIT " << limit;
    }

    if (offset > 0) {

        sql << " OFFSET " << offset;
    }

    m_sql = sql.str();
}

/**
 * @brief UBJStore::QueryAction::execute
 * @return
 */

bool Store::QueryAction::execute()
{
    if (!prepareStmt(m_sql)) {

        if (m_callback) {

            m_callback(true, nullptr);

            notify();
        }

        return false;
    }

    //std::cerr << sql(true) << "\n";

    if (m_callback) {

        m_callback(false, Store::Cursor::create(shared_from_this()));
    }

    finish();

    notify();

    return true;
}

/**
 * @brief UBJStore::CountAction::CountAction
 * @param store
 * @param table
 * @param query
 * @param callback
 */

Store::CountAction::CountAction(
        const Store::Pointer &store,
        const std::string &table,
        const Object &query,
        const Callback &callback)
    : Action(store, table),
      m_callback(callback)
{
    std::stringstream sql;

    sql << "SELECT COUNT(1) FROM " << table;

    std::string whereStr = wherePart(query);

    if (!whereStr.empty()) {

        sql << " WHERE " << whereStr;
    }

    m_sql = sql.str();
}

/**
 * @brief UBJStore::CountAction::execute
 * @return
 */

bool Store::CountAction::execute()
{
    if (!prepareStmt(m_sql)) {

        if (m_callback) {

            m_callback(true, 0);

            notify();
        }

        return false;
    }

    //std::cerr << sql(true) << "\n";

    if (step() == SQLITE_ROW) {

        uint64_t count = sqlite3_column_int64(m_stmt, 0);

        finish();

        if (m_callback) {

            m_callback(false, count);
        }

        notify();

        return true;
    }
    else {

        finish();

        if (m_callback) {

            m_callback(true, 0);
        }

        notify();

        return false;
    }
}

/**
 * @brief UBJStore::InsertAction::InsertAction
 * @param store
 * @param table
 * @param insert
 * @param callback
 */

Store::InsertAction::InsertAction(
        const Store::Pointer &store,
        const std::string &table,
        const Object &insert,
        const Callback &callback)
    : Action(store, table),
      m_callback(callback)
{
    std::stringstream sql;

    sql << "INSERT INTO " << table << " " << insertPart(insert);

    m_sql = sql.str();
}

/**
 * @brief UBJStore::InsertAction::InsertAction
 * @param store
 * @param table
 * @param insert
 * @param callback
 */

Store::InsertAction::InsertAction(
        const Store::Pointer &store,
        const std::string &table,
        const Array &insert,
        const Callback &callback)
    : Action(store, table),
      m_callback(callback)
{

}

/**
 * @brief UBJStore::InsertAction::execute
 * @return
 */

bool Store::InsertAction::execute()
{
    if (!prepareStmt(m_sql)) {

        if (m_callback) {

            m_callback(true, {});

            notify();
        }

        return false;
    }

    //std::cerr << sql(true) << "\n";

    if (step() == SQLITE_DONE) {

        uint64_t rowId = sqlite3_last_insert_rowid(m_store->db());

        finish();

        if (m_callback) {

            m_callback(false, {rowId});
        }

        notify();

        return true;
    }
    else {

        finish();

        if (m_callback) {

            m_callback(true, {});
        }

        notify();

        return false;
    }
}

/**
 * @brief UBJStore::UpdateAction::UpdateAction
 * @param store
 * @param table
 * @param update
 * @param where
 * @param callback
 */

Store::UpdateAction::UpdateAction(
        const Store::Pointer &store,
        const std::string &table,
        const Object &update,
        const Object &where,
        const Store::UpdateAction::Callback &callback)
    : Action(store, table),
      m_callback(callback)
{
    std::stringstream sql;

    sql << "UPDATE " << table << " SET " << updatePart(update);

    std::string whereStr = wherePart(where);

    if (!whereStr.empty()) {

        sql << " WHERE " << whereStr;
    }

    m_sql = sql.str();
}

/**
 * @brief UBJStore::UpdateAction::execute
 * @return
 */

bool Store::UpdateAction::execute()
{
    if (!prepareStmt(m_sql)) {

        if (m_callback) {

            m_callback(true, 0);

            notify();
        }

        return false;
    }

    //std::cerr << sql(true) << "\n";

    if (step() == SQLITE_DONE) {

        uint32_t numRowsAffected = sqlite3_changes(m_store->db());

        finish();

        if (m_callback) {

            m_callback(false, numRowsAffected);
        }

        notify();

        return true;
    }
    else {

        finish();

        if (m_callback) {

            m_callback(true, 0);
        }

        notify();

        return false;
    }
}

/**
 * @brief UBJStore::RemoveAction::RemoveAction
 * @param store
 * @param table
 * @param where
 * @param callback
 */

Store::RemoveAction::RemoveAction(
        const Store::Pointer &store,
        const std::string &table,
        const Object &where,
        const Store::RemoveAction::Callback &callback)
    : Action(store, table),
      m_callback(callback)
{
    std::stringstream sql;

    sql << "DELETE FROM " << table;

    std::string whereStr = wherePart(where);

    if (!whereStr.empty()) {

        sql << " WHERE " << whereStr;
    }

    m_sql = sql.str();
}

/**
 * @brief UBJStore::RemoveAction::execute
 * @return
 */

bool Store::RemoveAction::execute()
{
    if (!prepareStmt(m_sql)) {

        if (m_callback) {

            m_callback(true, 0);

            notify();
        }

        return false;
    }

    //std::cerr << sql(true) << "\n";

    if (step() == SQLITE_DONE) {

        uint32_t numRowsAffected = sqlite3_changes(m_store->db());

        finish();

        if (m_callback) {

            m_callback(false, numRowsAffected);
        }

        notify();

        return true;
    }
    else {

        finish();

        if (m_callback) {

            m_callback(true, 0);
        }

        notify();

        return false;
    }
}

/**
 * @brief Store::ReadBlobAction::ReadBlobAction
 * @param store
 * @param table
 * @param id
 * @param callback
 */

Store::ReadBlobAction::ReadBlobAction(
        const Store::Pointer &store,
        const std::string &table,
        uint64_t id,
        const Callback &callback)
    : Action(store, table),
      m_id(id),
      m_callback(callback)
{

}

/**
 * @brief Store::ReadBlobAction::execute
 * @return
 */

bool Store::ReadBlobAction::execute()
{
    BLOB blob = m_store->openBlob(m_table, m_id, true, true);

    if (blob) {

        if (m_callback) {

            m_callback(false, blob);
        }

        blob->close();

        notify();

        return true;
    }
    else {

        if (m_callback) {

            m_callback(true, 0);
        }

        notify();

        return false;
    }
}

/**
 * @brief Store::WriteBlobAction::WriteBlobAction
 * @param store
 * @param table
 * @param id
 * @param callback
 */

Store::WriteBlobAction::WriteBlobAction(
        const Store::Pointer &store,
        const std::string &table,
        uint64_t id,
        const Callback &callback)
    : Action(store, table),
      m_id(id),
      m_callback(callback)
{

}

/**
 * @brief Store::WriteBlobAction::execute
 * @return
 */

bool Store::WriteBlobAction::execute()
{
    BLOB blob = m_store->openBlob(m_table, m_id, false, true);

    if (blob) {

        if (m_callback) {

            m_callback(false, blob);
        }

        blob->close();

        notify();

        return true;
    }
    else {

        if (m_callback) {

            m_callback(true, 0);
        }

        notify();

        return false;
    }
}


// ============================================================ //
// Blob
// ============================================================ //

/**
 * @brief UBJStore::Blob::Blob
 * @param store
 */

Store::Blob::Blob(Store *store)
    : m_store(store),
      m_id(0),
      m_mode(false),
      m_blob(nullptr)
{

}

/**
 * @brief UBJStore::Blob::~Blob
 */

Store::Blob::~Blob()
{
    close();
}

/**
 * @brief UBJStore::Blob::open
 * @param table
 * @param id
 * @return
 */

bool Store::Blob::open(const std::string &table, uint64_t id, bool readOnly, bool meta, bool mode, uint32_t size, BUFFER salt)
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
 * @brief UBJStore::Blob::read
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

bool Store::Blob::read(BUFFER data, uint32_t size, uint32_t offset, uint32_t *bytesRead)
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
 * @brief UBJStore::Blob::write
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

bool Store::Blob::write(BUFFER data, uint32_t size, uint32_t offset, uint32_t *bytesWritten)
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
 * @brief UBJStore::Blob::close
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
 * @brief UBJStore::Blob::id
 * @return
 */

uint64_t Store::Blob::id()
{
    return m_id;
}

/**
 * @brief UBJStore::Blob::aes
 * @return
 */

Crypto::AES &Store::Blob::aes()
{
    return m_aes;
}

// ============================================================ //

/**
 * @brief StoreHandler::create
 * @param store
 * @return
 */

STORE_HANDLER StoreHandler::create(const STORE &store)
{
    return STORE_HANDLER(new StoreHandler(store));
}

/**
 * @brief StoreHandler::StoreHandler
 * @param store
 */

StoreHandler::StoreHandler(const STORE &store)
    : m_store(store)
{
    sqlite3_busy_handler(m_store->db(), &StoreHandler::busyHandler, this);
}

/**
 * @brief StoreHandler::process
 * @param action
 */

void StoreHandler::process(Store::ACTION &action)
{
    action->execute();
}

/**
 * @brief StoreHandler::busyHandler
 * @param data
 * @param arg
 * @return
 */

int StoreHandler::busyHandler(void *data, int arg)
{
    Handler *handler = (Handler*)data;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (arg == 10) {

        // handle to long busy time

        return 0;
    }

    if (handler->canceled()) {

        return 0;
    }

    // go on waiting

    return 1;
}

// ============================================================ //

}

}
