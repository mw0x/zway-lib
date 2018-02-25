
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

#ifndef ZWAY_THREAD_SAFE_H_
#define ZWAY_THREAD_SAFE_H_

#include <mutex>

namespace Zway {

// ============================================================ //

typedef std::lock_guard<std::mutex> MutexLocker;

// ============================================================ //

template <typename T>
class ThreadSafe : public std::mutex
{
public:

    ThreadSafe()
        : std::mutex()
    {
    }

    ThreadSafe(T t)
        : std::mutex()
    {
        m_t = t;
    }

    ThreadSafe& operator=(T t)
    {
        m_t = t;

        return *this;
    }

    operator T&() const
    {
        return (T&)m_t;
    }

    operator T&()
    {
        return m_t;
    }

    T& operator*()
    {
        return m_t;
    }

    T* operator->()
    {
        return &m_t;
    }

protected:

    T m_t;
};

// ============================================================ //

template <typename T>
class EnableLock
{
public:

    class Lock
    {
    public:

        Lock(T &value) : m_value(value), m_guard(value.__mutex) {}

        T *operator -> () { return &m_value; }

    private:

        T &m_value;

        std::lock_guard<std::mutex> m_guard;
    };

protected:

    std::mutex __mutex;
};

// ============================================================ //

class EnableVisit
{
public:

    template <typename T>
    auto visit(T action) -> decltype (action())
    {
        std::lock_guard<std::mutex> locker(__mutex);

        return action();
    }

private:

    std::mutex __mutex;
};


// ============================================================ //

}

#endif
