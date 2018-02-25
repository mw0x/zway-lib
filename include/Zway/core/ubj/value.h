
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

#ifndef ZWAY_CORE_UBJ_VALUE_H_
#define ZWAY_CORE_UBJ_VALUE_H_

#include "Zway/core/ubj/ubj.h"
#include "Zway/types.h"

#include <deque>
#include <list>
#include <map>

namespace Zway {

USING_SHARED_PTR(MemoryBuffer)

namespace UBJ {

class Value;

class Object;

class Array;

// ============================================================ //

/**
 * @brief The Value class
 */

class Value
{
public:

    static bool read(Value &val, const MemoryBuffer$ &data);

    static bool read(Value &val, const uint8_t *data, uint32_t size);


    static bool read(Object &obj, const MemoryBuffer$ &data);

    static bool read(Object &obj, const uint8_t *data, uint32_t size);


    static bool read(Array &arr, const MemoryBuffer$ &data);

    static bool read(Array &arr, const uint8_t *data, uint32_t size);


    static MemoryBuffer$ write(const Value &val);


    static std::string dump(const Value& val, int indent = 2);


    Value();

    Value(const Object &obj);

    Value(const Array &arr);


    Value(const std::string& str);

    Value(const char* str, uint32_t len = 0);


    Value(int32_t val);

    Value(int64_t val);


    Value(uint32_t val);

    Value(uint64_t val);


    Value(float val);

    Value(double val);


    Value(bool val);


    Value(MemoryBuffer$ buf);


    ~Value();


    void clear();


    UBJ_TYPE type() const;


    bool isNull() const;

    bool isObject() const;

    bool isArray() const;


    UBJ::Object toObject() const;

    UBJ::Array toArray() const;

    std::string toStr() const;

    int32_t toInt() const;

    uint32_t toUInt() const;

    int64_t toLong() const;

    uint64_t toULong() const;

    float toFloat() const;

    double toDouble() const;

    bool toBool() const;


    Object &obj();

    const Object &obj() const;


    Array &arr();

    const Array &arr() const;


    Value copy() const;


    MemoryBuffer$ buffer() const;

    uint32_t bufferSize() const;

    uint8_t *bufferData() const;


    uint32_t numItems() const;


    bool hasField(const std::string &key) const;


    Value const operator[](const std::string &key) const;

    Value const operator[](uint32_t index) const;


    friend std::ostream &operator<<(std::ostream& os, const Value &val);

protected:

    bool setString(const char *str, uint32_t len);

    bool setBuffer(const uint8_t* data, uint32_t size, UBJ_TYPE type);

protected:

    UBJ_TYPE m_type;

    MemoryBuffer$ m_buffer;

    std::shared_ptr<Object> m_obj;

    std::shared_ptr<Array> m_arr;

    friend class Object;

    friend class Array;

    friend class Dumper;

    friend class Reader;

    friend class Writer;
};

#define UBJ_DUMP(x) Zway::UBJ::Value::dump(x)

// ============================================================ //

/**
 * @brief The Object class
 */

class Object : public std::map<std::string, Value>
{
protected:

    class ValueInit
    {
    public:

        ValueInit(Object &obj, const key_type &key);

        Object &operator<<(const Value &val);

    protected:

        Object &m_obj;

        key_type m_key;
    };

public:

    Object();

    Object(const Value &val);

    Object(std::initializer_list<value_type> args);


    Object copy() const;


    ValueInit operator<<(const key_type &key);

    Object &operator <<(const Object &obj);


    mapped_type &operator[](const key_type &key);

    mapped_type const operator[](const key_type &key) const;


    friend std::ostream &operator<<(std::ostream& os, const Object &obj);


    void setField(const key_type &key, const UBJ::Value &val);

    bool hasField(const key_type &key) const;
};

#define UBJ_OBJ(x) (Zway::UBJ::Object() << x)

// ============================================================ //

/**
 * @brief The Array class
 */

class Array : public std::deque<UBJ::Value>
{
public:

    Array();

    Array(const Value &val);

    Array(std::initializer_list<value_type> args);


    Array copy() const;


    Array &operator<<(const Value &val);


    friend std::ostream &operator<<(std::ostream& os, const Array &arr);
};

#define UBJ_ARR(x) (Zway::UBJ::Array() << x)

// ============================================================ //

}}

#endif
