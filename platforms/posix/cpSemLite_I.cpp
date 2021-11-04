// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSemLite_I.cpp
//
//  Description:    Semaphore facility using lightweight implementation.
//
//  Platform:       posix
//
//  History:
//  2012-08-03  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-11-28  asc Added thread release during destruction.
//  2012-12-19  asc Removed safety delay in destructor.
// ----------------------------------------------------------------------------

#include <sys/time.h>

#include "cpPlatform.h"
#include "cpSemLite.h"
#include "cpUtil.h"

namespace cp
{

// thread cleanup handler
static void TakeOperationCleanup(void *pData)
{
        SemLite_t *pSem = reinterpret_cast<SemLite_t *>(pData);
        pthread_mutex_unlock(&pSem->lock);
}


// constructor
SemLite::SemLite(String const &Name, uint32_t InitCount, uint32_t MaxCount) :
    Base(Name),
    m_MaxCount(MaxCount)
{
    m_Semaphore.l_ok = true;
    m_Semaphore.c_ok = true;

    if (InitCount > MaxCount)
    {
        InitCount = MaxCount;
    }

    m_Semaphore.count = InitCount;

    if (pthread_mutex_init(&m_Semaphore.lock, NULL) != 0)
    {
        LogErr << "SemLite::~SemLite(): Failed to create mutex: "
               << NameGet() << std::endl;
        m_Semaphore.l_ok = false;
    }

    if (pthread_cond_init(&m_Semaphore.cond, NULL) != 0)
    {
        LogErr << "SemLite::~SemLite(): Failed to create condition variable: "
               << NameGet() << std::endl;
        m_Semaphore.c_ok = false;
    }

    m_Valid = m_Semaphore.l_ok && m_Semaphore.c_ok;
    m_Semaphore.enabled = m_Valid;
}


// destructor
SemLite::~SemLite()
{
    // prevent anyone from blocking on the semaphore
    m_Semaphore.enabled = false;

    // release anyone blocked on the semaphore
    GiveAll();
    //cp::MilliSleep(10);

    // destroy the underlying mutex
    if (m_Semaphore.l_ok)
    {
        if (pthread_mutex_destroy(&m_Semaphore.lock) != 0)
        {
            LogErr << "SemLite::~SemLite(): Failed to destroy mutex: "
                   << NameGet() << std::endl;
        }
    }

    // destroy the underlying condition variable
    if (m_Semaphore.c_ok)
    {
        if (pthread_cond_destroy(&m_Semaphore.cond) != 0)
        {
            LogErr << "SemLite::~SemLite(): Failed to destroy condition variable: "
                   << NameGet() << std::endl;
        }
    }
}


bool SemLite::Take(uint32_t Timeout)
{
    bool rv = true;
    timespec now;
    timespec then;

    // check for instance validity
    if (!IsValid("SemLite::Take()"))
    {
        return false;
    }

    // check for take operation enabled state
    if (!m_Semaphore.enabled)
    {
        return false;
    }

    pthread_mutex_lock(&m_Semaphore.lock);
    pthread_cleanup_push(TakeOperationCleanup, &m_Semaphore);

    while (rv && (m_Semaphore.count == 0))
    {
        if (Timeout == k_InfiniteTimeout)
        {
            rv = (pthread_cond_wait(&m_Semaphore.cond, &m_Semaphore.lock) == 0);
        }
        else
        {
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

                rv = (pthread_cond_timedwait(&m_Semaphore.cond, &m_Semaphore.lock, &then) != ETIMEDOUT);
            }
        }
    }

    if (rv)
    {
        m_Semaphore.count--;
    }

    pthread_cleanup_pop(0);
    pthread_mutex_unlock(&m_Semaphore.lock);

    return rv;
}


bool SemLite::TryTake()
{
    bool rv = false;

    // check for instance validity
    if (!IsValid("SemLite::TryTake()"))
    {
        return false;
    }

    // call Take() with a timeout of 0 milliseconds
    rv = Take(0);

    return rv;
}


bool SemLite::Give()
{
    bool rv = false;

    // check for instance validity
    if (!IsValid("SemLite::Give()"))
    {
        return false;
    }

    pthread_mutex_lock(&m_Semaphore.lock);

    if (m_Semaphore.count < m_MaxCount)
    {
        m_Semaphore.count++;
        pthread_cond_signal(&m_Semaphore.cond);
        rv = true;
    }

    pthread_mutex_unlock(&m_Semaphore.lock);

    return rv;
}


bool SemLite::GiveAll()
{
    // check for instance validity
    if (!IsValid("SemLite::GiveAll()"))
    {
        return false;
    }

    pthread_mutex_lock(&m_Semaphore.lock);

    m_Semaphore.count = m_MaxCount;
    pthread_cond_broadcast(&m_Semaphore.cond);

    pthread_mutex_unlock(&m_Semaphore.lock);

    return true;
}


uint32_t SemLite::CountGet() const
{
    return m_Semaphore.count;
}


uint32_t SemLite::MaxCountGet() const
{
    return m_MaxCount;
}

}   // namespace cp
