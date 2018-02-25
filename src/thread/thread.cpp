
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

#include "Zway/thread/thread.h"

namespace Zway {

// ============================================================ //

/**
 * @brief Thread::Thread
 */

Thread::Thread()
{
    m_running = false;

    m_suspended = false;

    m_canceled = false;
}

/**
 * @brief Thread::~Thread
 */

Thread::~Thread()
{

}

/**
 * @brief Thread::run
 */

void Thread::start(bool paused)
{
    if (!running()) {

        {
            MutexLocker lock(m_canceled);

            m_canceled = false;
        }

        if (paused) {

            MutexLocker lock(m_suspended);

            m_suspended = true;
        }

        m_thread = std::thread(&Thread::doRun, this);
    }
}

/**
 * @brief Thread::pause
 */

void Thread::suspend()
{
    if (running()) {

        MutexLocker lock(m_suspended);

        m_suspended = true;
    }
}

/**
 * @brief Thread::resume
 */

void Thread::resume()
{
    if (running() && suspended()) {

        {
            MutexLocker lock(m_suspended);

            m_suspended = false;
        }

        {
            std::unique_lock<std::mutex> lock(m_waitResumeMutex);

            m_waitResumeCondition.notify_one();
        }
    }
}

/**
 * @brief Thread::cancel
 */

void Thread::cancel()
{
    {
        MutexLocker lock(m_canceled);

        m_canceled = true;
    }

    resume();
}

/**
 * @brief Thread::join
 */

void Thread::join()
{
    if (m_thread.get_id() != std::this_thread::get_id() &&
        m_thread.joinable()) {

        m_thread.join();
    }
}

/**
 * @brief Thread::cancelAndJoin
 */

void Thread::cancelAndJoin()
{
    cancel();

    join();
}

/**
 * @brief Thread::waitResume
 */

void Thread::waitResume()
{
    std::unique_lock<std::mutex> lock(m_waitResumeMutex);

    m_waitResumeCondition.wait(lock);
}

/**
 * @brief Thread::running
 * @return
 */

bool Thread::running()
{
    MutexLocker lock(m_running);

    return m_running;
}

/**
 * @brief Thread::paused
 * @return
 */

bool Thread::suspended()
{
    MutexLocker lock(m_suspended);

    return m_suspended;
}

/**
 * @brief Thread::canceled
 * @return
 */

bool Thread::canceled()
{
    MutexLocker lock(m_canceled);

    return m_canceled;
}

/**
 * @brief Thread::threadId
 * @return
 */

std::thread::id Thread::threadId()
{
    return m_thread.get_id();
}

/**
 * @brief Thread::doRun
 */

void Thread::doRun()
{
    {
        MutexLocker lock(m_running);

        m_running = true;
    }

    run();

    {
        MutexLocker lock(m_running);

        m_running = false;
    }
}

// ============================================================ //

}
