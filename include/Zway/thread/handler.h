
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

#ifndef ZWAY_HANDLER_H_
#define ZWAY_HANDLER_H_

#include "Zway/thread/thread.h"

#include <list>

namespace Zway {

// ============================================================ //

/**
 * @brief The Handler class
 */

template <typename T>
class Handler : public Thread
{
public:

    Handler()
        : m_busy(false)
    {

    }

    void notify()
    {
        std::unique_lock<std::mutex> lock(m_waitMutex);

        m_waitCondition.notify_one();
    }

    void post(const T &element)
    {
        {
            MutexLocker lock(m_queue);

            m_queue->push_back(element);
        }

        if (!busy()) {

            notify();
        }
    }

    void clear()
    {
        MutexLocker lock(m_queue);

        m_queue->clear();
    }

    void suspend()
    {
        Thread::suspend();

        notify();
    }

    void cancel()
    {
        Thread::cancel();

        notify();
    }

    bool busy()
    {
        MutexLocker lock(m_busy);

        return m_busy;
    }

protected:

    virtual void process(T &element) = 0;

    virtual bool getElements()
    {
        return false;
    }

    void wait()
    {
        {
            MutexLocker lock(m_busy);

            m_busy = false;
        }

        std::unique_lock<std::mutex> lock(m_waitMutex);

        m_waitCondition.wait(lock);

        {
            MutexLocker lock(m_busy);

            m_busy = true;
        }
    }

    void run()
    {
        for (;;) {

            if (canceled()) {

                break;
            }

            if (suspended()) {

                waitResume();

                continue;
            }

            T *element = nullptr;

            {
                MutexLocker lock(m_queue);

                if (!m_queue->empty()) {

                    element = &m_queue->front();
                }
            }

            if (element) {

                process(*element);

                MutexLocker lock(m_queue);

                m_queue->pop_front();
            }
            else {

                if (!getElements()) {

                    wait();
                }
            }
        }
    }

protected:

    ThreadSafe<std::list<T>> m_queue;

    ThreadSafe<bool> m_busy;

    std::mutex m_waitMutex;

    std::condition_variable m_waitCondition;
};

// ============================================================ //

}

#endif
