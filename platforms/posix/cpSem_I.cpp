// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSem_I.cpp
//
//  Description:    Semaphore facility using native implementation.
//
//  Platform:       posix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Added GiveAll() method.
// ----------------------------------------------------------------------------

#include "cpPlatform.h"
#include "cpSem.h"

namespace cp
{

// constructor
Sem::Sem(String const &Name, uint32_t InitCount, uint32_t MaxCount) :
    Base(Name),
    m_MaxCount(MaxCount)
{
    bool rv = false;

    if (InitCount > MaxCount)
    {
        InitCount = MaxCount;
    }

    rv = (sem_init(&m_Semaphore, 0, InitCount) == 0);

    if (rv)
    {
        m_Valid = true;
    }
    else
    {
        m_MaxCount = 0;
        LogErr << "Sem::Sem(): Failed to initialize semaphore: "
               << NameGet() << std::endl;
    }
}


// destructor
Sem::~Sem()
{
    if (m_Valid)
    {
        if (sem_destroy(&m_Semaphore) != 0)
        {
            LogErr << "Sem::~Sem(): Failed to destroy semaphore: "
                   << NameGet() << std::endl;
        }
    }
}


bool Sem::Take(uint32_t Timeout)
{
    bool rv = false;

    // check for instance validity
    if (!IsValid("Sem::Take()"))
    {
        return false;
    }

    if (Timeout == k_InfiniteTimeout)
    {
        rv = (sem_wait(&m_Semaphore) == 0);

        if (!rv)
        {
            LogErr << "Sem::Take(): Failed to take semaphore: "
                   << NameGet() << std::endl;
        }
    }
    else
    {
        timespec now;
        timespec then;

        // get the current time
        if (clock_gettime(CLOCK_REALTIME, &now) == 0)
        {
            // translate milliseconds into seconds + nanoseconds
            then.tv_sec  = now.tv_sec  + (Timeout / 1000);
            then.tv_nsec = now.tv_nsec + (Timeout % 1000) * 1000000;

            // normalize the nanoseconds field
            while (then.tv_nsec >= 1000000000)
            {
                then.tv_sec++;
                then.tv_nsec -= 1000000000;
            }

            rv = (sem_timedwait(&m_Semaphore, &then) == 0);
        }
        else
        {
            LogErr << "Sem::Take(): clock_gettime() failure: "
                   << NameGet() << std::endl;
        }
    }

    return rv;
}


bool Sem::TryTake()
{
    bool rv = false;

    // check for instance validity
    if (!IsValid("Sem::TryTake()"))
    {
        return false;
    }

    // returns -1 if failed to take semaphore
    rv = (sem_trywait(&m_Semaphore) == 0);

    return rv;
}


bool Sem::Give()
{
    bool rv = false;

    // check for instance validity
    if (!IsValid("Sem::Give()"))
    {
        return false;
    }

    if (sem_post(&m_Semaphore) == 0)
    {
        rv = true;
    }

    return rv;
}


bool Sem::GiveAll()
{
    bool rv = true;

    for (uint32_t i=0; i<MaxCountGet(); ++i)
    {
        rv = rv && Give();
    }

    return rv;
}


uint32_t Sem::CountGet()
{
    int curCount = 0;

    // check for instance validity
    if (!IsValid("Sem::CountGet()"))
    {
        return 0;
    }

    if (sem_getvalue(&m_Semaphore, &curCount) != 0)
    {
        curCount = 0;
    }

    return curCount;
}


uint32_t Sem::MaxCountGet() const
{
    return m_MaxCount;
}

}   // namespace cp
