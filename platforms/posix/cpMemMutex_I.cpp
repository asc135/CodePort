// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpMemMutex_I.cpp
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

#include "cpPlatform.h"
#include "cpMemMutex_I.h"

namespace cp
{

// constructor
MemMutex::MemMutex()
{
    bool rv = false;
    pthread_mutexattr_t attr;

    // initialize the mutex attribute object
    pthread_mutexattr_init(&attr);

    // set attribute for mutex type to normal (non-recursive)
    rv = (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL) == 0);

    if (rv)
    {
        // initialize the mutex
        // no need to check return type because it always returns 0
        pthread_mutex_init(&m_Mutex, &attr);
        m_Valid = true;
    }
    else
    {
        LogErr << "MemMutex::MemMutex(): Error creating mutex in MemManager."
               << std::endl;
    }
}


// destructor
MemMutex::~MemMutex()
{
    if (m_Valid)
    {
        if (pthread_mutex_destroy(&m_Mutex) != 0)
        {
            LogErr << "MemMutex::~MemMutex(): Error destroying mutex in MemManager."
                   << std::endl;
        }
    }
}


bool MemMutex::Lock()
{
    bool rv = false;
    int status = 0;

    // check for instance validity
    if (!m_Valid)
    {
        return false;
    }

    // attempt to lock the mutex
    status = pthread_mutex_lock(&m_Mutex);
    rv = (status == 0);

    if (!rv)
    {
        LogErr << "MemMutex::Lock(): Failed to lock mutex in MemManager.  Error = ";

        switch (status)
        {
        case EINVAL:
            LogErr << "EINVAL";
            break;

        case EDEADLK:
            LogErr << "EDEADLK";
            break;

        case EAGAIN:
            LogErr << "EAGAIN";
            break;

        default:
            LogErr << "UNKNOWN";
            break;
        }

        LogErr << std::endl;
    }

    return rv;
}


bool MemMutex::TryLock()
{
    bool rv = false;

    // check for instance validity
    if (!m_Valid)
    {
        return false;
    }

    // attempt to lock the mutex
    rv = (pthread_mutex_trylock(&m_Mutex) == 0);

    return rv;
}


bool MemMutex::Unlock()
{
    bool rv = false;

    // check for instance validity
    if (!m_Valid)
    {
        return false;
    }

    // attempt to unlock the mutex
    rv = (pthread_mutex_unlock(&m_Mutex) == 0);

    if (!rv)
    {
        LogErr << "MemMutex::Unlock(): Failed to unlock mutex in MemManager."
               << std::endl;
    }

    return rv;
}

}   // namespace cp
