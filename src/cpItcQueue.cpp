// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpItcQueue.cpp
//
//  Description:    Inter-Thread Communications Queue
//
//  Platform:       common
//
//  History:
//  2012-10-24  asc Creation.
// ----------------------------------------------------------------------------

#include "cpItcQueue.h"

namespace cp
{

// constructor
ItcQueue::ItcQueue(String const &Name, size_t MaxEntries) :
    Base(Name),
    m_SemPut("Put Semaphore", MaxEntries, MaxEntries),
    m_SemGet("Get Semaphore", 0, MaxEntries),
    m_Mutex("ItcQueue Mutex")
{
    m_Valid = true;
}


// copy constructor
ItcQueue::ItcQueue(ItcQueue const &rhs) :
    Base(rhs.NameGet()),
    m_SemPut("Put Semaphore", rhs.m_Depth, rhs.m_Depth),
    m_SemGet("Get Semaphore", 0, rhs.m_Depth),
    m_Mutex("ItcQueue Mutex")
{
    // invoke assignment operator
    *this = rhs;
}


// destructor
ItcQueue::~ItcQueue()
{
}


// assignment operator
ItcQueue &ItcQueue::operator=(ItcQueue const &rhs)
{
    (void)rhs;
    return *this;
}


// put an element into the queue
bool ItcQueue::Put(void *Element, uint32_t Timeout)
{
    bool rv = m_SemPut.Take(Timeout);

    if (rv)
    {
        m_Mutex.Lock();
        m_Queue.push_back(Element);
        m_SemGet.Give();
        m_Mutex.Unlock();
    }

    return rv;
}


// get an element from the queue
bool ItcQueue::Get(void *&Element, uint32_t Timeout)
{
    bool rv = m_SemGet.Take(Timeout);

    if (rv)
    {
        m_Mutex.Lock();
        Element = m_Queue.front();
        m_Queue.pop_front();
        m_SemPut.Give();
        m_Mutex.Unlock();
    }

    return rv;
}

}   // namespace cp
