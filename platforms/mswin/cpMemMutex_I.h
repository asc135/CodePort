// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpMemMutex_I.h
//
//  Description:    Memory Manager Mutex Facility.  This is a
//                  low dependency mutex just for MemManager.
//
//  Platform:       mswin
//
//  History:
//  2012-12-10  asc Creation.
// ----------------------------------------------------------------------------

#ifndef CP_MEMMUTEX_I_H
#define CP_MEMMUTEX_I_H

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
    HANDLE              m_Mutex;                            // native data storage
};

}   // namespace cp

#endif  // CP_MEMMUTEX_I_H
