// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpMemMutex_I.cpp
//
//  Description:    Memory Manager Mutex Facility.  This is a
//                  low dependency mutex just for MemManager.
//
//  Platform:       mswin
//
//  History:
//  2012-12-10  asc Creation.
// ----------------------------------------------------------------------------

#include "cpPlatform.h"
#include "cpMemMutex_I.h"

namespace cp
{

// Constructor
MemMutex::MemMutex()
{
    m_Mutex = CreateMutex(NULL, false, NULL);

    if (m_Mutex != INVALID_HANDLE_VALUE)
    {
        m_Valid = true;
    }
    else
    {
        LogErr << "MemMutex::MemMutex(): Error creating mutex in MemManager." << std::endl;
    }
}


// Destructor
MemMutex::~MemMutex()
{
    if ((m_Valid) && (m_Mutex != INVALID_HANDLE_VALUE))
    {
        if (CloseHandle(m_Mutex) == 0)
        {
            LogErr << "MemMutex::~MemMutex(): Error destroying mutex in MemManager." << std::endl;
        }
    }
}


bool MemMutex::Lock()
{
    bool rv = false;

    if (m_Valid)
    {
        if (WaitForSingleObject(m_Mutex, INFINITE) == WAIT_OBJECT_0)
        {
            rv = true;
        }
    }

    if (!rv)
    {
        LogErr << "MemMutex::Lock(): Failed to lock mutex in MemManager." << std::endl;
    }

    return rv;
}


bool MemMutex::TryLock()
{
    bool rv = false;

    if (m_Valid)
    {
        if (WaitForSingleObject(m_Mutex, 0) == WAIT_OBJECT_0)
        {
            rv = true;
        }
    }

    return rv;
}


bool MemMutex::Unlock()
{
    bool rv = false;

    if (m_Valid)
    {
        rv = ReleaseMutex(m_Mutex);
    }

    if (!rv)
    {
        LogErr << "MemMutex::Unlock(): Error unlocking mutex in MemManager." << std::endl;
    }

    return rv;
}

}   // namespace cp
