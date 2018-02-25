
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

#ifndef ZWAY_UBJ_STORE_H_
#define ZWAY_UBJ_STORE_H_

#include "Zway/crypto/aes.h"
#include "Zway/thread/safe.h"
#include "Zway/ubj/store/module.h"

#include "Zway/ubj/store/action/begintransaction.h"
#include "Zway/ubj/store/action/endtransaction.h"
#include "Zway/ubj/store/action/query.h"
#include "Zway/ubj/store/action/count.h"
#include "Zway/ubj/store/action/insert.h"
#include "Zway/ubj/store/action/update.h"
#include "Zway/ubj/store/action/remove.h"
#include "Zway/ubj/store/action/readblob.h"
#include "Zway/ubj/store/action/writeblob.h"

namespace Zway { namespace UBJ { namespace Store {

USING_SHARED_PTR(Handler)
USING_SHARED_PTR(Store)

// ============================================================ //

/**
 * @brief The Store class
 */

class Store : public std::enable_shared_from_this<Store>
{
public:

    enum {
        MainNodeId=1,
        RootNodeId
    };


    static Store$ create(const std::string &filename, const std::string &password, bool handler=false);

    static Store$ unlock(const std::string &filename, const std::string &password, bool handler=false);


    Store();

    virtual ~Store();


    bool init(const std::string &filename, const std::string &password, bool handler=false);

    bool open(const std::string &filename, const std::string &password, bool handler=false);


    void cleanup();

    void close();


    VirtualTableModule* vtab(const std::string& name);


    Action$ beginTransaction(
            bool exclusive = false,
            const BeginTransaction::Callback &callback = nullptr,
            bool async=true);

    Action$ endTransaction(const EndTransaction::Callback &callback = nullptr);


    void query(
            const std::string &table,
            const Object &query={},
            const Object &order={},
            const Array &fieldsToReturn={},
            int32_t limit=0,
            int32_t offset=0,
            const QueryAction::Callback &callback=nullptr);

    void query(
            const std::string &table,
            const Object &query,
            const QueryAction::Callback &callback=nullptr);

    bool query(
            const std::string &table,
            const Object &query,
            Object *result,
            const Object &order={},
            const Array &fieldsToReturn={},
            int32_t offset=0);

    bool query(
            const std::string &table,
            const Object &query,
            std::list<Object> &result,
            const Object &order={},
            const Array &fieldsToReturn={},
            int32_t limit=0,
            int32_t offset=0);

    uint32_t count(
            const std::string &table,
            const Object &where={});

    uint64_t insert(
            const std::string &table,
            const Object &insert);

    uint32_t update(
            const std::string &table,
            const Object &update,
            const Object &where);

    uint32_t remove(
            const std::string &table,
            const Object &where);

    uint32_t truncate(
            const std::string &table);


    int64_t queryInt(
            const std::string &table,
            const std::string &field,
            const Object &query,
            const Object &order=Object(),
            int32_t offset=0);


    bool readBlob(const std::string &table, uint64_t id, const ReadBlobAction::Callback &callback);

    bool writeBlob(const std::string &table, uint64_t id, const WriteBlobAction::Callback &callback);


    uint64_t createBlob(const std::string &table, uint32_t size, bool encrypt=true);

    uint64_t createBlob(const std::string &table, MemoryBuffer$ data, bool encrypt=true);

    uint64_t createBlob(const std::string &table, const Object &data, bool encrypt=true);

    Blob$ openBlob(const std::string &table, uint64_t id, bool readOnly=true, bool meta=true, bool mode=true, uint32_t size=0, MemoryBuffer$ salt=nullptr);

    uint32_t updateBlobData(const std::string &table, uint64_t id, MemoryBuffer$ data, bool encrypt=true);

    uint32_t updateBlobData(const std::string &table, uint64_t id, const Object &data, bool encrypt=true);

    bool removeBlob(const std::string &table, uint64_t id);

    MemoryBuffer$ getBlobData(const std::string &table, uint64_t id);

    bool getBlobData(const std::string &table, uint64_t id, Object &data);

    bool getBlobInfo(const std::string &table, uint64_t id, Object &info);

    Array getBlobIds(const std::string &table);


    sqlite3 *db();


    std::string err();


    bool processAction(Action$ action);


    bool getIndex(const std::string &table, Object &index);

    bool setIndex(const std::string &table, const Array &ids, const Object &index);


    bool vacuum();


protected:

    virtual bool createBlobTables();

    virtual bool createVTables();


    bool createBlobTable(const std::string &name);

private:

    bool createInternalBlobTables();

    bool createInternalVTables();

protected:

    sqlite3 *m_db;

    MemoryBuffer$ m_key;

    Handler$ m_handler;

    std::map<std::string, uint32_t> m_indexBlobs;

    std::map<std::string, VirtualTableModule> m_vtabs;

    std::string m_err;



    friend class Action;

    friend class Blob;
};

// ============================================================ //

}}}

#endif
