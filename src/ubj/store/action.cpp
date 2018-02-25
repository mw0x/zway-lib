
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
#include "Zway/ubj/store/action.h"
#include "Zway/ubj/store/store.h"

namespace Zway { namespace UBJ { namespace Store {

// ============================================================ //

/**
 * @brief Action::Action
 * @param store
 * @param table
 */

Action::Action(Store$ store, const std::string &table)
    : m_store(store),
      m_table(table),
      m_vtab(false),
      m_stmt(nullptr)
{
    m_done = false;

    m_vtab = store->m_vtabs.find(table) != store->m_vtabs.end();
}

/**
 * @brief Action::~Action
 */

Action::~Action()
{
    finish();
}

/**
 * @brief Action::step
 * @return
 */

int32_t Action::step()
{
    if (m_stmt) {

        return sqlite3_step(m_stmt);
    }

    return 0;
}

/**
 * @brief Action::execute
 * @return
 */

bool Action::execute()
{
    return true;
}

/**
 * @brief Action::reset
 */

void Action::reset()
{
    if (m_stmt) {

        sqlite3_reset(m_stmt);
    }
}

/**
 * @brief Action::finish
 */

void Action::finish()
{
    if (m_stmt) {

        sqlite3_finalize(m_stmt);

        m_stmt = nullptr;
    }
}

/**
 * @brief Action::wait
 */

void Action::wait()
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
 * @brief Action::stmt
 * @return
 */

sqlite3_stmt *Action::stmt()
{
    return m_stmt;
}

/**
 * @brief Action::table
 * @return
 */

std::string Action::table()
{
    return m_table;
}

/**
 * @brief Action::sql
 * @param expanded
 * @return
 */

std::string Action::sql(bool expanded)
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
 * @brief Action::prepareStmt
 * @param sql
 * @return
 */

bool Action::prepareStmt(const std::string &sql)
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
 * @brief Action::fieldsToReturnPart
 * @param fieldsToReturn
 * @return
 */

std::string Action::fieldsToReturnPart(const Array &fieldsToReturn)
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
 * @brief Action::wherePart
 * @param query
 * @return
 */

std::string Action::wherePart(const Object &query)
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
 * @brief Action::orderPart
 * @param order
 * @return
 */

std::string Action::orderPart(const Object &order)
{
    std::string res;

    for (auto &it : order) {

        res += it.first + (it.second.toInt() > 0 ? " ASC " : " DESC ");
    }

    return res;
}

/**
 * @brief Action::insertPart
 * @param insert
 * @return
 */

std::string Action::insertPart(const Object &insert)
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
 * @brief Action::updatePart
 * @param update
 * @return
 */

std::string Action::updatePart(const Object &update)
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
 * @brief Action::bindUbjToStmt
 * @return
 */

int32_t Action::bindUbjToStmt()
{
    int32_t i=0;

    for (auto &value : m_bound) {

        MemoryBuffer$ buf = value.buffer();

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

                MemoryBuffer$ buf = Value::write(value);

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

                MemoryBuffer$ buf = Value::write(value);

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
 * @brief Action::rowToUbj
 * @param obj
 * @return
 */

bool Action::rowToUbj(Object &obj)
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

            MemoryBuffer$ buf = MemoryBuffer::create(sqlite3_column_text(m_stmt, i), numBytes);

            if (!buf) {

                return false;
            }

            obj[name] = std::string((char*)buf->data(), buf->size());
        }
        else
        if (type == SQLITE_BLOB) {

            int32_t numBytes = sqlite3_column_bytes(m_stmt, i);

            MemoryBuffer$ buf = MemoryBuffer::create((uint8_t*)sqlite3_column_blob(m_stmt, i), numBytes);

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
 * @brief Action::notify
 */

void Action::notify()
{
    std::unique_lock<std::mutex> lock(m_waitMutex);

    m_waitCondition.notify_one();

    {
        MutexLocker lock(m_done);

        m_done = true;
    }
}

// ============================================================ //

}}}
