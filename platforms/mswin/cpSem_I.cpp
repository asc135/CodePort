// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSem_I.cpp
//
//  Description:    Semaphore facility using native implementation.
//
//  Platform:       mswin
//
//  History:
//  2012-12-10  asc Creation.
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
    if (InitCount > MaxCount)
    {
        InitCount = MaxCount;
    }

    // correct for MS sloppiness in choosing parameter data types
    if (static_cast<int32_t>(InitCount) < 0)
    {
        InitCount = 0x7fffffff;
    }

    if (static_cast<int32_t>(MaxCount) < 0)
    {
        MaxCount = 0x7fffffff;
    }

    m_Semaphore = CreateSemaphore(NULL, InitCount, MaxCount, NULL);

    if (m_Semaphore != INVALID_HANDLE_VALUE)
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
    // check for instance validity
    if (m_Valid)
    {
        if (CloseHandle(m_Semaphore) == false)
        {
            LogErr << "Sem::~Sem(): Failed to destroy semaphore: "
                   << NameGet() << std::endl;
        }
    }
}


bool Sem::Take(uint32_t Timeout)
{
    bool rv = false;

    DWORD result = 0;
    DWORD localTimeout = (Timeout == k_InfiniteTimeout) ? INFINITE : Timeout;

    // check for instance validity
    if (m_Valid)
    {
        result = WaitForSingleObject(m_Semaphore, localTimeout);

        if (result == WAIT_OBJECT_0)
        {
            rv = true;
        }

        // Display timeout failure message only if timeout was infinite.
        if ((Timeout == k_InfiniteTimeout) && (result == WAIT_TIMEOUT))
        {
            LogErr << "Sem::Take(): Failed to take semaphore due to timeout: "
                   << NameGet() << std::endl;
        }

        if (result == WAIT_ABANDONED)
        {
            LogErr << "Sem::Take(): Failed to take semaphore due to thread closing: "
                   << NameGet() << std::endl;
        }
    }

    return rv;
}


bool Sem::TryTake()
{
    bool rv = false;
    DWORD result = 0;

    // check for instance validity
    if (m_Valid)
    {
        result = WaitForSingleObject(m_Semaphore, 0);

        if (result == WAIT_OBJECT_0)
        {
            rv = true;
        }
    }

    return rv;
}


bool Sem::Give()
{
    bool rv = false;

    // check for instance validity
    if (m_Valid)
    {
        rv = ReleaseSemaphore(m_Semaphore, 1, NULL);
    }
    else
    {
        LogErr << "Sem::Release(): Failed to give semaphore: "
               << NameGet() << std::endl;
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
    LONG curCount = m_MaxCount;

    // check for instance validity
    if (m_Valid)
    {
        // Determine count.
        if (ReleaseSemaphore(m_Semaphore, 1, &curCount))
        {
            // If semaphore count was bumped up, put it back to where it was.
            TryTake();
        }
    }

    return curCount;
}


uint32_t Sem::MaxCountGet() const
{
    return m_MaxCount;
}

}   // namespace cp
