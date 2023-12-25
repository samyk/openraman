/*
 *	2020 (C) The Pulsar Engineering
 *	http://www.thepulsar.be
 *
 *	This document is licensed under the CERN OHL-W v2 (http://ohwr.org/cernohl).
 *
 *	You may redistribute and modify this document under the terms of the
 *	CERN OHL-W v2 only. This document is distributed WITHOUT ANY EXPRESS OR
 *	IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND
 *	FITNESS FOR APARTICULAR PURPOSE. Please refer to the CERN OHL-W v2 for
 *	applicable conditions.
 */
#pragma once

#include <map>

#include <Windows.h>

/* RecursiveLock class */
class RecursiveLock
{
public:

    RecursiveLock(void) {}

    // lock object
    void lock(void)
    {
        if (has())
            set(true);
        else
            this->m_list.emplace(std::make_pair(id(), true));
    }

    // unlock object
    void unlock(void)
    {
        if (has())
            set(false);
    }

    // test if locked
    bool isLocked(void) const
    {
        return has() && get();
    }

private:
    bool has(void) const
    {
        return this->m_list.find(id()) != this->m_list.end();
    }

    const bool get(void) const
    {
        if (!has())
            return false;

        return this->m_list.find(id())->second;
    }

    void set(bool val)
    {
        if (!has())
            return;

        this->m_list.find(id())->second = val;
    }

    size_t id(void) const
    {
        return (size_t)GetCurrentThreadId();
    }

    std::map<size_t,bool> m_list;
};

/* automatic lock RecursiveLock object */
class AutoRecursiveLock
{
public:

    // lock on ctor
    AutoRecursiveLock(RecursiveLock& lock) : m_lock(lock)
    {
        this->m_lock.lock();
    }

    // unlock on dtor
    ~AutoRecursiveLock(void)
    {
        this->m_lock.unlock();
    }

private:
    RecursiveLock& m_lock;
};

// autolock rlock object
#define AUTORLOCK(obj)      AutoRecursiveLock rlockobj##__COUNTER__(obj);