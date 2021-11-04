// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpMemMutex_I.h
//
//  Description:    Memory Manager Mutex Facility.  This is a
//                  low dependency mutex just for MemManager.
//
//  Platform:       posix
//
//  History:
//  2011-10-13  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_MEMMUTEX_I_H
#define CP_MEMMUTEX_I_H

#include <pthread.h>

namespace cp
{

class MemMutex
{
public:
    MemMutex();

    ~MemMutex();

    bool Lock();
    bool TryLock();
    bool Unlock();

private:
    bool                m_Valid;                            // flag to indicate successful initialization
    pthread_mutex_t     m_Mutex;                            // native data storage
};

}   // namespace cp

#endif  // CP_MEMMUTEX_I_H
