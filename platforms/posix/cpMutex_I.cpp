// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpMutex_I.cpp
//
//  Description:    Mutex Facility.
//
//  Platform:       posix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#include "cpPlatform.h"
#include "cpMutex.h"

namespace cp
{

// constructor
Mutex::Mutex(String const &Name, MutexMode Mode, bool PriBoost) :
    Base(Name)
{
    bool rv = false;
    pthread_mutexattr_t attr;

    // initialize the mutex attribute object
    pthread_mutexattr_init(&attr);

    // no priority boost in posix
    (void)PriBoost;

    // set attribute for mutex type
    switch (Mode)
    {
    case MtxRecursive:
        rv = (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0);
        break;

    case MtxNonRecursive: // fall through case

    default:
        rv = (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL) == 0);
        break;
    }

    if (rv)
    {
        // initialize the mutex
        // no need to check return type because it always returns 0
        pthread_mutex_init(&m_Mutex, &attr);
        m_Valid = true;
    }
    else
    {
        LogErr << "Mutex::Mutex(): Error creating mutex: "
               << NameGet() << std::endl;
    }
}


// destructor
Mutex::~Mutex()
{
    if (m_Valid)
    {
        if (pthread_mutex_destroy(&m_Mutex) != 0)
        {
            LogErr << "Mutex::~Mutex(): Error destroying mutex: "
                   << NameGet() << std::endl;
        }
    }
}


bool Mutex::Lock()
{
    bool rv = false;
    int status = 0;

    // check for instance validity
    if (!IsValid("Mutex::Lock()"))
    {
        return false;
    }

    // attempt to lock the mutex
    status = pthread_mutex_lock(&m_Mutex);
    rv = (status == 0);

    if (!rv)
    {
        LogErr << "Mutex::Lock(): Failed to lock mutex: "
               << NameGet() << ". Error = ";

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


bool Mutex::TryLock()
{
    bool rv = false;

    // check for instance validity
    if (!IsValid("Mutex::TryLock()"))
    {
        return false;
    }

    // attempt to lock the mutex
    rv = (pthread_mutex_trylock(&m_Mutex) == 0);

    return rv;
}


bool Mutex::Unlock()
{
    bool rv = false;

    // check for instance validity
    if (!IsValid("Mutex::Unlock()"))
    {
        return false;
    }

    // attempt to unlock the mutex
    rv = (pthread_mutex_unlock(&m_Mutex) == 0);

    if (!rv)
    {
        LogErr << "Mutex::Unlock(): Failed to unlock mutex: "
               << NameGet() << std::endl;
    }

    return rv;
}

}   // namespace cp
