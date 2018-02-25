
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

#ifndef ZWAY_UBJ_MODULE_H_
#define ZWAY_UBJ_MODULE_H_

#include "Zway/core/ubj/value.h"

#include <cstring>
#include <vector>

#include <sqlite3.h>

namespace Zway { namespace UBJ {

using UbjObjectList = std::deque<UBJ::Object>;

// ============================================================ //

class Store;

class VirtualTableModule
{
public:

    bool create(
            Store *storage,
            const std::string &blobTable,
            const std::string &name,
            uint32_t type,
            bool index,
            const std::vector<std::string> &columns,
            const std::vector<std::string> &types);

    bool createIndex(const std::string& column, UBJ::Object &index, const UBJ::Array &ids = UBJ::Array());

    bool createIndexes(UBJ::Object &indexes, const UBJ::Array &ids = UBJ::Array());

    bool createHitmap(UBJ::Object &indexes, UBJ::Value &hitmap);

    bool getIndex(const std::string& column, UBJ::Object &index);

    void getIndexes(UBJ::Object &indexes);


    std::string blobTableName();


    void updateIds();


private:

    class VirtualTable : public sqlite3_vtab
    {
    public:

        VirtualTableModule *module;
    };

    class Cursor : public sqlite3_vtab_cursor
    {
    public:

        VirtualTable *table;

        std::vector<uint32_t> rowIds;

        UbjObjectList rows;

        int pos;
    };

private:

    bool hasIndexItem(UBJ::Object &index, const std::string &val, uint32_t id);

    void addIndexItem(UBJ::Object &index, const std::string &val, uint32_t id);

    void removeIndexItem(UBJ::Object &index, const std::string &val, uint32_t id);


    static int xConnect(sqlite3* db, void *pAux, int argc, const char* const* argv, sqlite3_vtab **ppVTab, char **pzErr);

    static int xBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* pInfo);

    static int xDisconnect(sqlite3_vtab *pVTab);

    static int xDestroy(sqlite3_vtab *pVTab);

    static int xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCur);

    static int xClose(sqlite3_vtab_cursor* pCur);

    static int xEof(sqlite3_vtab_cursor *pCur);

    static int xFilter(sqlite3_vtab_cursor *pCur, int idxNum, const char *idxStr, int argc, sqlite3_value **argv);

    static int xNext(sqlite3_vtab_cursor *pCur);

    static int xColumn(sqlite3_vtab_cursor *pCur, sqlite3_context* pCtx, int N);

    static int xRowid(sqlite3_vtab_cursor *pCur, sqlite_int64 *pRowid);

    static int xUpdate(sqlite3_vtab *pVTab, int argc, sqlite3_value **argv, sqlite3_int64 *pRowid);

private:

    Store *m_store;

    uint32_t m_type;

    std::string m_blobTable;

    std::vector<std::string> m_columns;

    Zway::UBJ::Array m_ids;

    Zway::UBJ::Object m_indexes;

    Zway::UBJ::Object m_hitmap;

    sqlite3_module m_module;

    std::string m_sql;
};

// ============================================================ //

}

}

#endif
