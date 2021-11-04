// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpItcQueue.h
//
//  Description:    Inter-Thread Communications Queue
//
//  Platform:       common
//
//  History:
//  2012-10-24  asc Creation.
//  2013-08-26  asc Added Capacity() accessor.
// ----------------------------------------------------------------------------

#ifndef CP_ITCQUEUE_H
#define CP_ITCQUEUE_H

#include <queue>

#include "cpMutex.h"
#include "cpSemLite.h"
#include "cpAlloc.h"

namespace cp
{

class ItcQueue : public Base
{
public:
    // local types
    typedef std::deque<void *, Alloc<void *> > Queue_t;

    // constructor
    ItcQueue(String const &Name, size_t MaxEntries = 16);

    // destructor
    ~ItcQueue();

    // accessors
    bool Put(void *Element,
             uint32_t Timeout = k_InfiniteTimeout);         // put an element into the queue

    bool Get(void *&Element,
             uint32_t Timeout = k_InfiniteTimeout);         // get an element from the queue

    size_t Capacity() const { return m_Depth; }             // return maximum queue capacity

private:
    // copy constructor
    ItcQueue(ItcQueue const &rhs);

    // assignment operator
    ItcQueue &operator=(ItcQueue const &rhs);

    size_t              m_Depth;                            // depth of queue (max entries)
    SemLite             m_SemPut;                           // semaphore to control put operations
    SemLite             m_SemGet;                           // semaphore to control get operations
    Mutex               m_Mutex;                            // mutex to protect container
    Queue_t             m_Queue;
};

}   // namespace cp

#endif  // CP_ITCQUEUE_H
