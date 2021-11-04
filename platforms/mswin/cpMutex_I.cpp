// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpMutex_I.cpp
//
//  Description:    Mutex Facility.
//
//  Platform:       mswin
//
//  History:
//  2012-12-10  asc Creation.
// ----------------------------------------------------------------------------

#include "cpPlatform.h"
#include "cpMutex.h"

namespace cp
{

// Constructor
Mutex::Mutex(String const &Name,
             MutexMode Mode,
             bool PriBoost) :
    Base(Name)
{
    (void)Mode;
    (void)PriBoost;

    m_Mutex = CreateMutex(NULL, false, Name.c_str());

    if (m_Mutex != INVALID_HANDLE_VALUE)
    {
        m_Valid = true;
    }
    else
    {
        LogErr << "Mutex::Mutex(): Error creating mutex: "
               << NameGet() << std::endl;
    }
}


// Destructor
Mutex::~Mutex()
{
    if ((m_Valid) && (m_Mutex != INVALID_HANDLE_VALUE))
    {
        if (CloseHandle(m_Mutex) == 0)
        {
            LogErr << "Mutex::~Mutex(): Error destroying mutex: "
                   << NameGet() << std::endl;
        }
    }
}


bool Mutex::Lock()
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
        LogErr << "Mutex::Lock(): Failed to lock mutex: "
               << NameGet() << std::endl;
    }

    return rv;
}


bool Mutex::TryLock()
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


bool Mutex::Unlock()
{
    bool rv = false;

    if (m_Valid)
    {
        rv = ReleaseMutex(m_Mutex);
    }

    if (!rv)
    {
        LogErr << "Mutex::Unlock(): Failed to unlock mutex: "
               << NameGet() << std::endl;
    }

    return rv;
}

}   // namespace cp
