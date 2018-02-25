
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

#include "Zway/core/buffer.h"
#include "Zway/core/crypto/aes.h"
#include "Zway/core/thread/handler.h"
#include "Zway/store/ubj/module.h"

namespace Zway { namespace UBJ {

USING_SHARED_PTR(Store)
USING_SHARED_PTR(StoreHandler)

// ============================================================ //

/**
 * @brief The Store class
 */

class Store : public std::enable_shared_from_this<Store>
{
public:

    typedef std::shared_ptr<Store> Pointer;


    class Cursor;

    class Handler;


    class Action : public std::enable_shared_from_this<Action>
    {
    public:

        typedef std::shared_ptr<Action> Pointer;

        virtual ~Action();

        int32_t step();

        virtual bool execute();

        virtual void reset();

        virtual void finish();


        void wait();


        sqlite3_stmt *stmt();

        std::string table();

        std::string sql(bool expanded=false);


    protected:

        Action(const Store::Pointer &store, const std::string &table=std::string());

        bool prepareStmt(const std::string &sql);

        std::string fieldsToReturnPart(const Array &fieldsToReturn);

        std::string wherePart(const Object &query);

        std::string orderPart(const Object &order);

        std::string insertPart(const Object &insert);

        std::string updatePart(const Object &update);


        int32_t bindUbjToStmt();


        bool rowToUbj(Object &obj);


        void notify();


    protected:

        Store::Pointer m_store;

        std::string m_table;

        bool m_vtab;

        sqlite3_stmt *m_stmt;

        std::string m_sql;

        Array m_bound;

        std::mutex m_waitMutex;

        std::condition_variable m_waitCondition;

        ThreadSafe<bool> m_done;

        friend class Cursor;

        friend class Handler;
    };

    typedef Action::Pointer ACTION;


    class Cursor
    {
    public:

        typedef std::shared_ptr<Cursor> Pointer;

        static Pointer create(ACTION action);

        ~Cursor();

        void forEach(const std::function<void (Object&)> &fn);

        bool forEach(const std::function<bool (Object&)> &fn);

        bool next(Object &item);

        void reset();


        ACTION action();


        int32_t getInt(int32_t index);

        int64_t getLong(int32_t index);


    protected:

        Cursor(ACTION action);

    protected:

        ACTION m_action;
    };

    typedef Cursor::Pointer CURSOR;


    class Blob : public Buffer
    {
    public:

        typedef std::shared_ptr<Blob> Pointer;

        Blob(Store *storage);

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

        Store *m_store;

        uint64_t m_id;

        bool m_mode;

        void *m_blob;

        MemoryBuffer$ m_salt;

        Crypto::AES m_aes;
    };

    typedef Blob::Pointer BLOB;


    class BeginTransaction : public Action
    {
    public:

        typedef std::shared_ptr<BeginTransaction> Pointer;

        typedef std::function<void (bool)> Callback;

        BeginTransaction(
                const Store::Pointer &store,
                bool exclusive=false,
                const Callback &callback = nullptr);

        bool execute();

        bool exclusive();

    protected:

        bool m_exclusive;

        Callback m_callback;
    };


    class EndTransaction : public Action
    {
    public:

        typedef std::function<void (bool)> Callback;

        EndTransaction(
                const Store::Pointer &store,
                const Callback &callback = nullptr);

        bool execute();

    protected:

        Callback m_callback;
    };


    class QueryAction : public Action
    {
    public:

        typedef std::function<void (bool, CURSOR)> Callback;

        QueryAction(
                const Store::Pointer &store,
                const std::string &table,
                const Object &query={},
                const Object &order={},
                const Array &fieldsToReturn={},
                int32_t limit=0,
                int32_t offset=0,
                const Callback &callback = nullptr);

        bool execute();

    protected:

        Callback m_callback;
    };


    class CountAction : public Action
    {
    public:

        typedef std::function<void (bool, uint32_t)> Callback;

        CountAction(
                const Store::Pointer &store,
                const std::string &table,
                const Object &query={},
                const Callback &callback = nullptr);

        bool execute();

    protected:

        Callback m_callback;
    };


    class InsertAction : public Action
    {
    public:

        typedef std::function<void (bool, const std::deque<uint64_t>&)> Callback;

        InsertAction(
                const Store::Pointer &store,
                const std::string &table,
                const Object &insert,
                const Callback &callback = nullptr);

        InsertAction(
                const Store::Pointer &store,
                const std::string &table,
                const Array &insert,
                const Callback &callback = nullptr);

        bool execute();

    protected:

        Callback m_callback;
    };


    class UpdateAction : public Action
    {
    public:

        typedef std::function<void (bool, uint32_t)> Callback;

        UpdateAction(
                const Store::Pointer &store,
                const std::string &table,
                const Object &update,
                const Object &where,
                const Callback &callback = nullptr);

        bool execute();

    protected:

        Callback m_callback;
    };


    class RemoveAction : public Action
    {
    public:

        typedef std::function<void (bool, uint32_t)> Callback;

        RemoveAction(
                const Store::Pointer &store,
                const std::string &table,
                const Object &where,
                const Callback &callback = nullptr);

        bool execute();

    protected:

        Callback m_callback;
    };


    class ReadBlobAction : public Action
    {
    public:

        typedef std::function<void (bool, BLOB)> Callback;

        ReadBlobAction(
                const Store::Pointer &store,
                const std::string &table,
                uint64_t id,
                const Callback &callback = nullptr);

        bool execute();

    protected:

        uint64_t m_id;

        Callback m_callback;
    };


    class WriteBlobAction : public Action
    {
    public:

        typedef std::function<void (bool, BLOB)> Callback;

        WriteBlobAction(
                const Store::Pointer &store,
                const std::string &table,
                uint64_t id,
                const Callback &callback = nullptr);

        bool execute();

    protected:

        uint64_t m_id;

        Callback m_callback;
    };


    enum {
        MainNodeId=1,
        RootNodeId
    };


    static Pointer create(const std::string &filename, const std::string &password, bool handler=false);

    static Pointer unlock(const std::string &filename, const std::string &password, bool handler=false);


    Store();

    virtual ~Store();


    bool init(const std::string &filename, const std::string &password, bool handler=false);

    bool open(const std::string &filename, const std::string &password, bool handler=false);


    void cleanup();

    void close();


    VirtualTableModule* vtab(const std::string& name);


    ACTION beginTransaction(
            bool exclusive = false,
            const BeginTransaction::Callback &callback = nullptr,
            bool async=true);

    ACTION endTransaction(const EndTransaction::Callback &callback = nullptr);


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

    BLOB openBlob(const std::string &table, uint64_t id, bool readOnly=true, bool meta=true, bool mode=true, uint32_t size=0, MemoryBuffer$ salt=nullptr);

    uint32_t updateBlobData(const std::string &table, uint64_t id, MemoryBuffer$ data, bool encrypt=true);

    uint32_t updateBlobData(const std::string &table, uint64_t id, const Object &data, bool encrypt=true);

    bool removeBlob(const std::string &table, uint64_t id);

    MemoryBuffer$ getBlobData(const std::string &table, uint64_t id);

    bool getBlobData(const std::string &table, uint64_t id, Object &data);

    bool getBlobInfo(const std::string &table, uint64_t id, Object &info);

    Array getBlobIds(const std::string &table);


    sqlite3 *db();


    std::string err();


    bool processAction(ACTION action);


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

    StoreHandler$ m_handler;

    std::map<std::string, uint32_t> m_indexBlobs;

    std::map<std::string, VirtualTableModule> m_vtabs;

    std::string m_err;
};

// ============================================================ //

/**
 * @brief The StoreHandler class
 */

class StoreHandler : public Handler<Store::ACTION>
{
public:

    static StoreHandler$ create(const Store$ &store);

protected:

    StoreHandler(const Store$ &store);

    void process(Store::ACTION &action);

    static int busyHandler(void* data, int arg);

protected:

    Store$ m_store;
};

// ============================================================ //

}

}

#endif
