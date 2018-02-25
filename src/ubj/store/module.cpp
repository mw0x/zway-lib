
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

#include "Zway/memorybuffer.h"
#include "Zway/ubj/store/store.h"

namespace Zway { namespace UBJ { namespace Store {

// ============================================================ //

/**
 * @brief SqliteUbjModule::create
 * @param store
 * @param blobTable
 * @param name
 * @param type
 * @param index
 * @param columns
 * @param types
 * @return
 */

bool VirtualTableModule::create(
        Store$ store,
        const std::string &blobTable,
        const std::string &name,
        uint32_t type,
        bool index,
        const std::vector<std::string> &columns,
        const std::vector<std::string> &types)
{
    memset(&m_module, 0, sizeof(m_module));

    m_module.xConnect    = VirtualTableModule::xConnect;
    m_module.xBestIndex  = VirtualTableModule::xBestIndex;
    m_module.xDisconnect = VirtualTableModule::xDisconnect;
    m_module.xDestroy    = VirtualTableModule::xDestroy;
    m_module.xOpen       = VirtualTableModule::xOpen;
    m_module.xClose      = VirtualTableModule::xClose;
    m_module.xEof        = VirtualTableModule::xEof;
    m_module.xFilter     = VirtualTableModule::xFilter;
    m_module.xNext       = VirtualTableModule::xNext;
    m_module.xColumn     = VirtualTableModule::xColumn;
    m_module.xRowid      = VirtualTableModule::xRowid;
    m_module.xUpdate     = VirtualTableModule::xUpdate;

    int res = sqlite3_create_module(store->db(), name.c_str(), &m_module, this);

    if (res != SQLITE_OK) {

        return false;
    }


    m_store = store;

    m_type = type;

    m_blobTable = blobTable;

    m_columns = columns;


    m_sql = "CREATE TABLE vtab (";

    for (int i=0; i<m_columns.size(); ++i) {

        m_sql += "`" + m_columns[i] + "` " + types[i];

        if (i < m_columns.size() - 1) {

            m_sql += ",";
        }
    }

    m_sql += ")";


    if (index) {

        /*
        UBJ::Object obj;

        if (m_store->getIndex(name, obj)) {

            std::cerr << "found index for '" << name << "'\n";

            std::cerr << obj << "\n";

            m_ids = obj["ids"];

            m_indexes = obj["idx"];
        }
        else {

            std::cerr << "no index for '" << name << "'\n";

            std::cerr << "creating index for '" << name << "'\n";

            const UBJ::Array ids = m_store->getBlobIds(m_blobTable);

            UBJ::Object obj;

            createIndexes(obj, ids);

            std::cerr << obj << "\n";

            if (!m_store->setIndex(name, ids, obj)) {

                std::cerr << "failed to set index for '" << name << "'\n";
            }
        }
        */

        //createHitmap(m_indexes, m_hitmap);
    }
    else {

        m_ids = m_store->getBlobIds(m_blobTable);
    }

    return true;
}

/**
 * @brief SqliteUbjModule::createIndex
 * @param column
 * @param index
 * @param ids
 * @return
 */

bool VirtualTableModule::createIndex(const std::string &column, UBJ::Object &index, const UBJ::Array &ids)
{
    UBJ::Object res;

    UBJ::Array blobIds = ids.size() ? ids : m_store->getBlobIds(m_blobTable);

    for (auto &v : blobIds) {

        uint32_t id = v.toInt();

        MemoryBuffer$ nodeData = m_store->getBlobData(m_blobTable, id);

        if (nodeData) {

            UBJ::Object obj;

            if (UBJ::Value::read(obj, nodeData)) {

                if (obj.hasField(column)) {

                    addIndexItem(res, obj[column].toStr(), id);
                }
            }
        }
    }

    index = res;

    return true;
}

/**
 * @brief SqliteUbjModule::createIndexes
 * @param indexes
 * @param ids
 * @return
 */

bool VirtualTableModule::createIndexes(UBJ::Object &indexes, const UBJ::Array &ids)
{
    for (auto &column : m_columns) {

        if (!indexes.hasField(column)) {

            UBJ::Object obj;

            createIndex(column, obj, ids);

            indexes[column] = obj;
        }
    }

    return true;
}

/**
 * @brief SqliteUbjModule::createHitmap
 * @param indexes
 * @param hitmap
 * @return
 */

bool VirtualTableModule::createHitmap(UBJ::Object &indexes, UBJ::Value &hitmap)
{
    /*
    // determine the average number of rows that might be
    // returned by the specified index

    for (auto &v : indexes) {

        UBJ::Value &val = v.second;

        if (val.size()) {

            // sum up all rows for this key

            uint32_t a = 0;

            for (auto &v2 : val) {

                a += v2.second.size();
            }

            // set average row number

            hitmap[v.first.second] = a / val.size();
        }
    }
    */

    return true;
}

/**
 * @brief SqliteUbjModule::getIndex
 * @param column
 * @param index
 * @return
 */

bool VirtualTableModule::getIndex(const std::string &column, UBJ::Object &index)
{
    if (m_indexes.hasField(column)) {

        index = m_indexes[column];

        return true;
    }

    return false;
}

/**
 * @brief SqliteUbjModule::getIndexes
 * @param indexes
 */

void VirtualTableModule::getIndexes(UBJ::Object &indexes)
{
    indexes = m_indexes;
}

/**
 * @brief SqliteUbjModule::blobTableName
 * @return
 */

std::string VirtualTableModule::blobTableName()
{
    return m_blobTable;
}

/**
 * @brief SqliteUbjModule::updateIds
 */

void VirtualTableModule::updateIds()
{
    UBJ::Array ids = m_store->getBlobIds(m_blobTable);

    UBJ::Object index;

    createIndex("_$type", index, ids);

    m_ids = index[UBJ::Value(m_type).toStr()];
}

/**
 * @brief SqliteUbjModule::hasIndexItem
 * @param index
 * @param val
 * @param id
 * @return
 */

bool VirtualTableModule::hasIndexItem(UBJ::Object &index, const std::string &val, uint32_t id)
{
    if (index.hasField(val) && index[val].type() == UBJ_ARRAY) {

        for (auto &v : index[val].arr()) {

            if (v.toInt() == id) {

                return true;
            }
        }
    }

    return false;
}

/**
 * @brief SqliteUbjModule::addIndexItem
 * @param index
 * @param val
 * @param id
 */

void VirtualTableModule::addIndexItem(UBJ::Object &index, const std::string &val, uint32_t id)
{
    if (!index.hasField(val)) {

        index[val] = UBJ_ARR(id);
    }
    else
    if (!hasIndexItem(index, val, id)) {

        index[val].arr() << id;
    }
}

/**
 * @brief SqliteUbjModule::removeIndexItem
 * @param index
 * @param val
 * @param id
 */

void VirtualTableModule::removeIndexItem(UBJ::Object &index, const std::string &val, uint32_t id)
{
    if (index.hasField(val) && index[val].type() == UBJ_ARRAY) {

        UBJ::Array ids;

        for (auto &v : index[val].arr()) {

            if (v.toInt() != id) {

                ids << v;
            }
        }

        index[val] = ids;

        if (index[val].arr().empty()) {

            index.erase(val);
        }
    }
    else {

        index.erase(val);
    }
}

/**
 * @brief SqliteUbjModule::xConnect
 * @param db
 * @param pAux
 * @param argc
 * @param argv
 * @param ppVTab
 * @param pzErr
 * @return
 */

int VirtualTableModule::xConnect(sqlite3* db, void *pAux, int argc, const char* const* argv, sqlite3_vtab **ppVTab, char **pzErr)
{
    VirtualTableModule *module = (VirtualTableModule*)pAux;

    if (sqlite3_declare_vtab(db, module->m_sql.c_str()) != SQLITE_OK) {

        return SQLITE_ERROR;
    }

    VirtualTable *vtab = new VirtualTable();

    if (!vtab) {

        return SQLITE_ERROR;
    }

    vtab->pModule = &module->m_module;

    vtab->module = module;

    *ppVTab = (sqlite3_vtab*)vtab;

    return SQLITE_OK;
}

/**
 * @brief SqliteUbjModule::xBestIndex
 * @param pVTab
 * @param pInfo
 * @return
 */

int VirtualTableModule::xBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* pInfo)
{
    //std::cerr << "xBestIndex nConstraint=" <<  pInfo->nConstraint << "\n";

    VirtualTable *vtab = (VirtualTable*)pVTab;

    int numValues = 0;
    int idx = 0;
    int ix = 0;

    for (int i=0; i<pInfo->nConstraint; ++i) {

        sqlite3_index_info::sqlite3_index_constraint &constr = pInfo->aConstraint[i];

        sqlite3_index_info::sqlite3_index_constraint_usage &usage = pInfo->aConstraintUsage[i];

        if (constr.iColumn == -1) {

            idx = -1;

            ix = i;
        }
        else {

            std::string key = vtab->module->m_columns[constr.iColumn];

            uint32_t hits = vtab->module->m_hitmap[key].toInt();

            if (hits && (!numValues || hits < numValues)) {

                numValues = hits;

                idx = constr.iColumn + 1;

                ix = i;

                pInfo->estimatedRows = numValues;
            }
        }
    }

    if (idx) {

        sqlite3_index_info::sqlite3_index_constraint &constr = pInfo->aConstraint[ix];

        sqlite3_index_info::sqlite3_index_constraint_usage &usage = pInfo->aConstraintUsage[ix];

        usage.argvIndex = 1;

        pInfo->idxStr = sqlite3_mprintf("%d", constr.op);

        pInfo->needToFreeIdxStr = 1;
    }

    pInfo->idxNum = idx;

    return SQLITE_OK;
}

/**
 * @brief SqliteUbjModule::xDisconnect
 * @param pVTab
 * @return
 */

int VirtualTableModule::xDisconnect(sqlite3_vtab *pVTab)
{
    VirtualTable* vtab = (VirtualTable*)pVTab;

    delete vtab;

    return SQLITE_OK;
}

/**
 * @brief SqliteUbjModule::xDestroy
 * @param pVTab
 * @return
 */

int VirtualTableModule::xDestroy(sqlite3_vtab *pVTab)
{
    return SQLITE_OK;
}

/**
 * @brief SqliteUbjModule::xOpen
 * @param pVTab
 * @param ppCur
 * @return
 */

int VirtualTableModule::xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCur)
{
    Cursor* cursor = new Cursor();

    if (!cursor) {

        return SQLITE_ERROR;
    }

    cursor->pVtab = pVTab;

    cursor->pos = 0;

    *ppCur = (sqlite3_vtab_cursor*)cursor;

    return SQLITE_OK;
}

/**
 * @brief SqliteUbjModule::xClose
 * @param pCur
 * @return
 */

int VirtualTableModule::xClose(sqlite3_vtab_cursor* pCur)
{
    Cursor *cursor = (Cursor*)pCur;

    delete cursor;

    return SQLITE_OK;
}

/**
 * @brief SqliteUbjModule::xEof
 * @param pCur
 * @return
 */

int VirtualTableModule::xEof(sqlite3_vtab_cursor *pCur)
{
    Cursor *cursor = (Cursor*)pCur;

    if (cursor->pos < cursor->rows.size()) {

        return 0;
    }

    return 1;
}

/**
 * @brief SqliteUbjModule::xFilter
 * @param pCur
 * @param idxNum
 * @param idxStr
 * @param argc
 * @param argv
 * @return
 */

int VirtualTableModule::xFilter(sqlite3_vtab_cursor *pCur, int idxNum, const char *idxStr, int argc, sqlite3_value **argv)
{
    //std::cerr << "xFilter argc " << argc << " idxNum " << idxNum << " idxStr " << (idxStr ? idxStr : "-") << "\n";

    VirtualTable *vtab = (VirtualTable*)pCur->pVtab;

    UBJ::Array ids;

    if (idxNum == -1) {

        int op;
        sscanf(idxStr, "%d", &op);

        if (op == SQLITE_INDEX_CONSTRAINT_EQ) {

            ids << (uint64_t)sqlite3_value_int64(argv[0]);
        }
    }
    else
    if (idxNum > 0) {

        int op;
        sscanf(idxStr, "%d", &op);

        std::string key = vtab->module->m_columns[idxNum - 1];

        UBJ::Object index = vtab->module->m_indexes[key];

        int valueType = sqlite3_value_type(argv[0]);

        if (valueType == SQLITE_INTEGER) {

            uint64_t value = sqlite3_value_int64(argv[0]);

            for (auto &v : index) {

                int val = UBJ::Value(v.first).toInt();

                if ((op == SQLITE_INDEX_CONSTRAINT_EQ && val == value) ||
                    (op == SQLITE_INDEX_CONSTRAINT_GT && val > value) ||
                    (op == SQLITE_INDEX_CONSTRAINT_LE && val <= value) ||
                    (op == SQLITE_INDEX_CONSTRAINT_LT && val < value) ||
                    (op == SQLITE_INDEX_CONSTRAINT_GE && val >= value)) {

                    for (auto &it : v.second.arr()) {

                        ids << it;
                    }
                }
            }

            if (ids.empty()) {

                ids = index[UBJ::Value(value).toStr()];
            }
        }
        else
        if (valueType == SQLITE_TEXT) {

            std::string value = (char*)sqlite3_value_text(argv[0]);

            if (op != SQLITE_INDEX_CONSTRAINT_MATCH &&
                op != SQLITE_INDEX_CONSTRAINT_LIKE &&
                op != SQLITE_INDEX_CONSTRAINT_GLOB &&
                op != SQLITE_INDEX_CONSTRAINT_REGEXP) {

                for (auto &v : index) {

                    std::string val = v.first;

                    if ((op == SQLITE_INDEX_CONSTRAINT_EQ && val == value) ||
                        (op == SQLITE_INDEX_CONSTRAINT_GT && val > value) ||
                        (op == SQLITE_INDEX_CONSTRAINT_LE && val <= value) ||
                        (op == SQLITE_INDEX_CONSTRAINT_LT && val < value) ||
                        (op == SQLITE_INDEX_CONSTRAINT_GE && val >= value)) {

                        for (auto &it : v.second.arr()) {

                            ids << it;
                        }
                    }
                }

                if (ids.empty()) {

                    ids = index[value];
                }
            }
        }
    }

    if (ids.empty()) {

        ids = vtab->module->m_ids;
    }

    Cursor *cursor = (Cursor*)pCur;

    cursor->rowIds.clear();

    cursor->rows.clear();

    cursor->pos = 0;

    for (auto &it : ids) {

        uint32_t id = it.toInt();

        MemoryBuffer$ nodeData = vtab->module->m_store->getBlobData(vtab->module->m_blobTable, id);

        if (nodeData) {

            UBJ::Object obj;

            if (UBJ::Value::read(obj, nodeData)) {

                cursor->rowIds.push_back(id);

                cursor->rows.push_back(obj);
            }
        }
    }

    return SQLITE_OK;
}

/**
 * @brief SqliteUbjModule::xNext
 * @param pCur
 * @return
 */

int VirtualTableModule::xNext(sqlite3_vtab_cursor *pCur)
{
    Cursor *cursor = (Cursor*)pCur;

    cursor->pos++;

    return SQLITE_OK;
}

/**
 * @brief SqliteUbjModule::xColumn
 * @param pCur
 * @param pCtx
 * @param N
 * @return
 */

int VirtualTableModule::xColumn(sqlite3_vtab_cursor *pCur, sqlite3_context* pCtx, int N)
{
    Cursor *cursor = (Cursor*)pCur;

    VirtualTable* vtab = (VirtualTable*)cursor->pVtab;

    UBJ::Object &obj = cursor->rows[cursor->pos];

    std::string key = vtab->module->m_columns[N];

    if (obj.hasField(key)) {

        UBJ::Value &val = obj[key];

        switch (val.type()) {
        case UBJ_OBJECT:
        case UBJ_ARRAY: {

            MemoryBuffer$ buf = UBJ::Value::write(val);

            if (buf) {

                sqlite3_result_blob(pCtx, buf->data(), buf->size(), SQLITE_TRANSIENT);
            }
            else {

                sqlite3_result_null(pCtx);
            }

            break;
        }
        case UBJ_INT32:
            sqlite3_result_int(pCtx, val.toInt());
            break;
        case UBJ_INT64:
            sqlite3_result_int64(pCtx, val.toLong());
            break;
        case UBJ_FLOAT32:
        case UBJ_FLOAT64:
            sqlite3_result_double(pCtx, val.toDouble());
            break;
        case UBJ_STRING:
            sqlite3_result_text(pCtx, (char*)val.bufferData(), val.bufferSize(), nullptr);
            break;
        default:
            sqlite3_result_null(pCtx);
            break;
        }
    }

    return SQLITE_OK;
}

/**
 * @brief SqliteUbjModule::xRowid
 * @param pCur
 * @param pRowid
 * @return
 */

int VirtualTableModule::xRowid(sqlite3_vtab_cursor *pCur, sqlite_int64 *pRowid)
{
    Cursor *cursor = (Cursor*)pCur;

    if (cursor->pos < cursor->rowIds.size()) {

        *pRowid = cursor->rowIds[cursor->pos];

        return SQLITE_OK;
    }

    return SQLITE_ERROR;
}

/**
 * @brief SqliteUbjModule::xUpdate
 * @param pVTab
 * @param argc
 * @param argv
 * @param pRowid
 * @return
 */

int VirtualTableModule::xUpdate(sqlite3_vtab *pVTab, int argc, sqlite3_value **argv, sqlite3_int64 *pRowid)
{
    //std::cerr << "xUpdate argc " << argc << "\n";

    VirtualTable *vtab = (VirtualTable*)pVTab;

    if (argc == 1) {

        // delete

        /*
        uint64_t blobId = sqlite3_value_int64(argv[0]);

        if (vtab->module->m_store->removeBlob(vtab->module->m_blobTable, blobId)) {

            // remove indexes

            / *
            for (auto &v : obj) {

                std::string key = v.first;

                std::string val = v.second.toStr();

                vtab->module->removeIndexItem(vtab->module->m_indexes[key], val, blobId);
            }
            * /

            // remove id

            UBJ::Array ids;

            for (auto &it : vtab->module->m_ids) {

                if (it.toInt() != blobId) {

                    ids << it;
                }
            }

            vtab->module->m_ids = ids;

            return SQLITE_OK;
        }
        */
    }
    else
    if (argc > 1 && sqlite3_value_type(argv[0]) == SQLITE_NULL) {

        // insert

        UBJ::Object obj;

        for (int i=2; i<argc; ++i) {

            std::string key = vtab->module->m_columns[i-2];

            int type = sqlite3_value_type(argv[i]);

            if (type == SQLITE_INTEGER) {

                obj[key] = (int64_t)sqlite3_value_int64(argv[i]);
            }
            else
            if (type == SQLITE_TEXT) {

                obj[key] = (char*)sqlite3_value_text(argv[i]);
            }
            else
            if (type == SQLITE_BLOB) {

                UBJ::Value::read(obj[key], (uint8_t*)sqlite3_value_blob(argv[i]), sqlite3_value_bytes(argv[i]));
            }
        }

        obj["_$type"] = vtab->module->m_type;

        MemoryBuffer$ data = UBJ::Value::write(obj);

        if (!data) {

            return SQLITE_ERROR;
        }

        uint64_t blobId = vtab->module->m_store->createBlob(vtab->module->m_blobTable, data);

        if (!blobId) {

            return SQLITE_ERROR;
        }

        // update index

        /*
        obj.erase("_$type");

        vtab->module->m_ids << blobId;

        for (auto &it : obj) {

            std::string key = it.first;

            std::string val = it.second.toStr();

            vtab->module->addIndexItem(vtab->module->m_indexes[key], val, blobId);
        }
        */

        // return row id

        *pRowid = blobId;

        return SQLITE_OK;
    }
    else
    if (argc > 1 && sqlite3_value_type(argv[0]) != SQLITE_NULL) {

        // update

        /*
        uint64_t blobId = sqlite3_value_int64(argv[0]);

        BUFFER data = vtab->module->m_store->getBlobData(vtab->module->m_blobTable, blobId);

        if (!data) {

            return SQLITE_ERROR;
        }

        UBJ::Object obj;

        if (!UBJ::Value::read(obj, data)) {

            return SQLITE_ERROR;
        }

        //UBJ::Object tmp = obj.clone();

        for (int i=2; i<argc; ++i) {

            std::string key = vtab->module->m_columns[i-2];

            int type = sqlite3_value_type(argv[i]);

            if (type == SQLITE_INTEGER) {

                obj[key] = (int64_t)sqlite3_value_int64(argv[i]);
            }
            else
            if (type == SQLITE_TEXT) {

                obj[key] = (char*)sqlite3_value_text(argv[i]);
            }
            else
            if (type == SQLITE_BLOB) {

                UBJ::Value::read(obj[key], (uint8_t*)sqlite3_value_blob(argv[i]), sqlite3_value_bytes(argv[i]));
            }
        }

        if (!vtab->module->m_store->updateBlobData(vtab->module->m_blobTable, blobId, obj)) {

            return SQLITE_ERROR;
        }

        // remove old indexes

        / *
        for (auto &v : tmp) {

            std::string key = v.first;

            std::string val = v.second.toStr();

            vtab->module->removeIndexItem(vtab->module->m_indexes[key], val, blobId);
        }
        * /

        // add new indexes

        / *
        for (auto &it : obj) {

            std::string key = it.first;

            std::string val = it.second.toStr();

            vtab->module->addIndexItem(vtab->module->m_indexes[key], val, blobId);
        }
        * /

        return SQLITE_OK;
        */
    }

    return SQLITE_ERROR;
}

// ============================================================ //

}}}
