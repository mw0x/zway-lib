
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

#include "Zway/core/ubj/value.h"
#include "Zway/core/ubj/reader.h"
#include "Zway/core/ubj/writer.h"
#include "Zway/core/ubj/dumper.h"

#include <cstring>
#include <sstream>

namespace Zway { namespace UBJ {

// ============================================================ //

/**
 * @brief Value::read
 * @param val
 * @param data
 * @return
 */

bool Value::read(Value &val, const BUFFER &data)
{
    return UBJ::Reader().read(val, data);
}

/**
 * @brief Value::read
 * @param val
 * @param data
 * @param size
 * @return
 */

bool Value::read(Value &val, const uint8_t *data, uint32_t size)
{
    return UBJ::Reader().read(val, data, size);
}

/**
 * @brief Value::read
 * @param obj
 * @param data
 * @return
 */

bool Value::read(Object &obj, const BUFFER &data)
{
    if (!data) {

        return false;
    }

    return read(obj, data->data(), data->size());
}

/**
 * @brief Value::read
 * @param obj
 * @param data
 * @param size
 * @return
 */

bool Value::read(Object &obj, const uint8_t *data, uint32_t size)
{
    Value val;

    if (!read(val, data, size)) {

        return false;
    }

    obj = val;

    return true;
}

/**
 * @brief Value::read
 * @param arr
 * @param data
 * @return
 */

bool Value::read(Array &arr, const BUFFER &data)
{
    if (!data) {

        return false;
    }

    return read(arr, data->data(), data->size());
}

/**
 * @brief Value::read
 * @param arr
 * @param data
 * @param size
 * @return
 */

bool Value::read(Array &arr, const uint8_t *data, uint32_t size)
{
    Value val;

    if (!read(val, data, size)) {

        return false;
    }

    arr = val;

    return true;
}

/**
 * @brief Value::write
 * @param val
 * @return
 */

BUFFER Value::write(const Value &val)
{
    return UBJ::Writer().write(val);
}

/**
 * @brief Value::dump
 * @param val
 * @param indent
 * @return
 */

std::string Value::dump(const Value &val, int indent)
{
    return UBJ::Dumper().dump(val, indent);
}

// ============================================================ //

/**
 * @brief Value::Value
 */

Value::Value()
    : m_type(UBJ_NULLTYPE)
{

}

/**
 * @brief Value::Value
 * @param obj
 */

Value::Value(const Object &obj)
    : m_type(UBJ_OBJECT)
{
    m_obj = std::make_shared<Object>(obj);
}

/**
 * @brief Value::Value
 * @param arr
 */

Value::Value(const Array &arr)
    : m_type(UBJ_ARRAY)
{
    m_arr = std::make_shared<Array>(arr);
}

/**
 * @brief Value::Value
 * @param str
 */

Value::Value(const std::string& str)
    : m_type(UBJ_NULLTYPE)
{
    setString(str.c_str(), str.size());
}

/**
 * @brief Value::Value
 * @param str
 * @param len
 */

Value::Value(const char* str, uint32_t len)
    : m_type(UBJ_NULLTYPE)
{
    setString(str, len);
}

/**
 * @brief Value::Value
 * @param val
 */

Value::Value(int32_t val)
{
    setBuffer((uint8_t*)&val, sizeof(int32_t), UBJ_INT32);
}

/**
 * @brief Value::Value
 * @param val
 */

Value::Value(int64_t val)
{
    setBuffer((uint8_t*)&val, sizeof(int64_t), UBJ_INT64);
}

/**
 * @brief Value::Value
 * @param val
 */

Value::Value(uint32_t val)
{
    setBuffer((uint8_t*)&val, sizeof(uint32_t), UBJ_INT32);
}

/**
 * @brief Value::Value
 * @param val
 */

Value::Value(uint64_t val)
{
    setBuffer((uint8_t*)&val, sizeof(uint64_t), UBJ_INT64);
}

/**
 * @brief Value::Value
 * @param val
 */

Value::Value(float val)
{
    setBuffer((uint8_t*)&val, sizeof(float), UBJ_FLOAT32);
}

/**
 * @brief Value::Value
 * @param val
 */

Value::Value(double val)
{
    setBuffer((uint8_t*)&val, sizeof(double), UBJ_FLOAT64);
}

/**
 * @brief Value::Value
 * @param val
 */

Value::Value(bool val)
    : m_type(val ? UBJ_BOOL_TRUE : UBJ_BOOL_FALSE)
{

}

/**
 * @brief Value::Value
 * @param buf
 */

Value::Value(BUFFER buf)
    : m_type(buf ? UBJ_ARRAY : UBJ_NULLTYPE),
      m_buffer(buf)
{

}

/**
 * @brief Value::~Value
 */

Value::~Value()
{
    clear();
}

/**
 * @brief Value::clear
 */

void Value::clear()
{
    m_type = UBJ_NULLTYPE;

    m_buffer.reset();

    m_obj.reset();

    m_arr.reset();
}

/**
 * @brief Value::type
 * @return
 */

UBJ_TYPE Value::type() const
{
    return m_type;
}

/**
 * @brief Value::isNull
 * @return
 */

bool Value::isNull() const
{
    return m_type == UBJ_NULLTYPE;
}

/**
 * @brief Value::isObject
 * @return
 */

bool Value::isObject() const
{
    return m_type == UBJ_OBJECT && m_obj;
}

/**
 * @brief Value::isArray
 * @return
 */

bool Value::isArray() const
{
    return m_type == UBJ_ARRAY && m_arr;
}

/**
 * @brief Value::toObject
 * @return
 */

Object Value::toObject() const
{
    return Object(*this);
}

/**
 * @brief Value::toArray
 * @return
 */

Array Value::toArray() const
{
    return Array(*this);
}

/**
 * @brief Value::toStr
 * @return
 */

std::string Value::toStr() const
{
    std::stringstream ss;

    if (m_buffer) {

        switch (m_type) {
        case UBJ_STRING:
            return std::string((char*)m_buffer->data(), m_buffer->size());
        case UBJ_INT32:
            ss << toInt();
            break;
        case UBJ_INT64:
            ss << toLong();
            break;
        case UBJ_FLOAT32:
            ss << toFloat();
            break;
        case UBJ_FLOAT64:
            ss << toDouble();
            break;
        default:
            break;
        }
    }

    return ss.str();
}

/**
 * @brief Value::toInt
 * @return
 */

int32_t Value::toInt() const
{
    if (m_buffer) {

        switch (m_type) {
        case UBJ_INT32:
            return *((int32_t*)m_buffer->data());
        case UBJ_INT64:
            return *((int64_t*)m_buffer->data());
        case UBJ_FLOAT32:
            return *((float*)m_buffer->data());
        case UBJ_FLOAT64:
            return *((double*)m_buffer->data());
        case UBJ_BOOL_TRUE:
            return 1;
        case UBJ_BOOL_FALSE:
            return 0;
        default:
            return 0;
        }
    }

    return 0;
}

/**
 * @brief Value::toUInt
 * @return
 */

uint32_t Value::toUInt() const
{
    return toInt();
}

/**
 * @brief Value::toLong
 * @return
 */

int64_t Value::toLong() const
{
    if (m_buffer) {

        switch (m_type) {
        case UBJ_INT32:
            return *((int32_t*)m_buffer->data());
        case UBJ_INT64:
            return *((int64_t*)m_buffer->data());
        case UBJ_FLOAT32:
            return *((float*)m_buffer->data());
        case UBJ_FLOAT64:
            return *((double*)m_buffer->data());
        case UBJ_BOOL_TRUE:
            return 1;
        case UBJ_BOOL_FALSE:
            return 0;
        default:
            return 0;
        }
    }

    return 0;
}

/**
 * @brief Value::toULong
 * @return
 */

uint64_t Value::toULong() const
{
    return toLong();
}

/**
 * @brief Value::toFloat
 * @return
 */

float Value::toFloat() const
{
    if (m_buffer) {

        switch (m_type) {
        case UBJ_INT32:
            return *((int32_t*)m_buffer->data());
        case UBJ_INT64:
            return *((int64_t*)m_buffer->data());
        case UBJ_FLOAT32:
            return *((float*)m_buffer->data());
        case UBJ_FLOAT64:
            return *((double*)m_buffer->data());
        case UBJ_BOOL_TRUE:
            return 1;
        case UBJ_BOOL_FALSE:
            return 0;
        default:
            return 0;
        }
    }

    return 0;
}

/**
 * @brief Value::toDouble
 * @return
 */

double Value::toDouble() const
{
    if (m_buffer) {

        switch (m_type) {
        case UBJ_INT32:
            return *((int32_t*)m_buffer->data());
        case UBJ_INT64:
            return *((int64_t*)m_buffer->data());
        case UBJ_FLOAT32:
            return *((float*)m_buffer->data());
        case UBJ_FLOAT64:
            return *((double*)m_buffer->data());
        case UBJ_BOOL_TRUE:
            return 1;
        case UBJ_BOOL_FALSE:
            return 0;
        default:
            return 0;
        }
    }

    return 0;
}

/**
 * @brief Value::toBool
 * @return
 */

bool Value::toBool() const
{
    if (m_type == UBJ_BOOL_TRUE) {

        return true;
    }
    else
    if (m_type == UBJ_BOOL_FALSE) {

        return false;
    }
    else
    if (m_buffer) {

        switch (m_type) {
        case UBJ_INT32:
            return *((int32_t*)m_buffer->data());
        case UBJ_INT64:
            return *((int64_t*)m_buffer->data());
        case UBJ_FLOAT32:
            return *((float*)m_buffer->data()) != 0;
        case UBJ_FLOAT64:
            return *((double*)m_buffer->data()) != 0;
        default:
            return false;
        }
    }

    return false;
}

/**
 * @brief Value::obj
 * @return
 */

Object &Value::obj()
{
    return *m_obj;
}

/**
 * @brief Value::obj
 * @return
 */

const Object &Value::obj() const
{
    return *m_obj;
}

/**
 * @brief Value::arr
 * @return
 */

Array &Value::arr()
{
    return *m_arr;
}

/**
 * @brief Value::arr
 * @return
 */

const Array &Value::arr() const
{
    return *m_arr;
}

/**
 * @brief Value::copy
 * @return
 */

Value Value::copy() const
{
    if (m_type == UBJ_OBJECT && m_obj) {

        return m_obj->copy();
    }
    else
    if (m_type == UBJ_ARRAY && m_arr) {

        return m_arr->copy();
    }
    else {

        Value val;

        val.m_type = m_type;

        val.m_buffer = m_buffer ? m_buffer->copy() : nullptr;

        return val;
    }
}

/**
 * @brief Value::buffer
 * @return
 */

BUFFER Value::buffer() const
{
    return m_buffer;
}

/**
 * @brief Value::bufferSize
 * @return
 */

uint32_t Value::bufferSize() const
{
    if (m_buffer) {

        return m_buffer->size();
    }

    return 0;
}

/**
 * @brief Value::bufferData
 * @return
 */

uint8_t *Value::bufferData() const
{
    if (m_buffer) {

        return m_buffer->data();
    }

    return nullptr;
}

/**
 * @brief Value::numItems
 * @return
 */

uint32_t Value::numItems() const
{
    if (m_type == UBJ_OBJECT && m_obj) {

        return m_obj->size();
    }
    else
    if (m_type == UBJ_ARRAY && m_arr) {

        return m_arr->size();
    }

    return 0;
}

/**
 * @brief Value::hasField
 * @param key
 * @return
 */

bool Value::hasField(const std::string &key) const
{
    if (m_type == UBJ_OBJECT && m_obj) {

        return m_obj->hasField(key);
    }

    return false;
}

/**
 * @brief Value::operator []
 * @param key
 * @return
 */

Value const Value::operator[](const std::string &key) const
{
    if (m_type == UBJ_OBJECT && m_obj && m_obj->find(key) != m_obj->end()) {

        return m_obj->at(key);
    }

    return Value();
}

/**
 * @brief Value::operator []
 * @param index
 * @return
 */

Value const Value::operator[](uint32_t index) const
{
    if (m_type == UBJ_ARRAY && m_arr && index < m_arr->size()) {

        return m_arr->at(index);
    }

    return Value();
}

/**
 * @brief operator <<
 * @param os
 * @param val
 * @return
 */

std::ostream &operator<<(std::ostream &os, const Value &val)
{
    os << UBJ_DUMP(val);

    return os;
}

/**
 * @brief Value::setString
 * @param str
 * @param len
 * @return
 */

bool Value::setString(const char* str, uint32_t len)
{
    if (!str) {

        return false;
    }

    uint32_t l = len > 0 ? len : strlen(str);

    if (!l) {

        return false;
    }

    m_buffer = Buffer::create(nullptr, l);

    if (!m_buffer) {

        return false;
    }

    if (!m_buffer->write((uint8_t*)str, l, 0, nullptr)) {

        return false;
    }

    m_type = UBJ_STRING;

    return true;
}

/**
 * @brief Value::setBuffer
 * @param data
 * @param size
 * @param type
 * @return
 */

bool Value::setBuffer(const uint8_t* data, uint32_t size, UBJ_TYPE type)
{
    if (type == UBJ_STRING) {

        return setString((char*)data, size);
    }
    else {

        m_buffer = Buffer::create(data, size);

        if (!m_buffer) {

            return false;
        }

        m_type = type;
    }

    return true;
}

// ============================================================ //

/**
 * @brief Object::ValueInit::ValueInit
 * @param obj
 * @param key
 */

Object::ValueInit::ValueInit(Object &obj, const key_type &key)
    : m_obj(obj),
      m_key(key)
{

}

/**
 * @brief Object::ValueInit::operator <<
 * @param val
 * @return
 */

Object &Object::ValueInit::operator<<(const Value &val)
{
    m_obj[m_key] = val;

    return m_obj;
}

/**
 * @brief Object::Object
 */

Object::Object()
{

}

/**
 * @brief Object::Object
 * @param val
 */

Object::Object(const Value &val)
{
    if (val.m_obj) {

        *this = *val.m_obj;
    }
}

/**
 * @brief Object::Object
 * @param args
 */

Object::Object(std::initializer_list<value_type> args)
    : std::map<key_type, mapped_type>(args)
{

}

/**
 * @brief Object::copy
 * @return
 */

Object Object::copy() const
{
    Object obj;

    for (auto &it : *this) {

        obj[it.first] = it.second.copy();
    }

    return obj;
}

/**
 * @brief Object::operator <<
 * @param key
 * @return
 */

Object::ValueInit Object::operator<<(const key_type &key)
{
    return ValueInit(*this, key);
}

/**
 * @brief Object::operator <<
 * @param obj
 * @return
 */

Object &Object::operator <<(const Object &obj)
{
    for (auto &it : obj) {

        (*this)[it.first] = it.second;
    }

    return *this;
}

/**
 * @brief Object::operator []
 * @param key
 * @return
 */

Object::mapped_type &Object::operator[](const key_type &key)
{
    return std::map<key_type, mapped_type>::operator[](key);
}

/**
 * @brief Object::operator []
 * @param key
 * @return
 */

Object::mapped_type const Object::operator[](const key_type &key) const
{
    if (find(key ) != cend()) {

        return at(key);
    }

    return mapped_type();
}

/**
 * @brief operator <<
 * @param os
 * @param obj
 * @return
 */

std::ostream &operator<<(std::ostream &os, const Object &obj)
{
    os << UBJ_DUMP(obj);

    return os;
}

/**
 * @brief Object::setField
 * @param key
 * @param val
 */

void Object::setField(const key_type &key, const Value &val)
{
    (*this)[key] = val;
}

/**
 * @brief Object::hasField
 * @param key
 * @return
 */

bool Object::hasField(const key_type &key) const
{
    return find(key) != end();
}

// ============================================================ //

/**
 * @brief Array::Array
 */

Array::Array()
{

}

/**
 * @brief Array::Array
 * @param val
 */

Array::Array(const Value &val)
{
    if (val.m_arr) {

        *this = *val.m_arr;
    }
}

/**
 * @brief Array::Array
 * @param args
 */

Array::Array(std::initializer_list<value_type> args)
    : std::deque<value_type>(args)
{

}

/**
 * @brief Array::copy
 * @return
 */

Array Array::copy() const
{
    Array arr;

    for (auto &it : *this) {

        arr << it.copy();
    }

    return arr;
}

/**
 * @brief Array::operator <<
 * @param val
 * @return
 */

Array &Array::operator<<(const Value &val)
{
    push_back(val);

    return *this;
}

/**
 * @brief operator <<
 * @param os
 * @param arr
 * @return
 */

std::ostream &operator<<(std::ostream &os, const Array &arr)
{
    os << UBJ_DUMP(arr);

    return os;
}

// ============================================================ //

}}
