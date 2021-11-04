// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpMutex.h
//
//  Description:    Mutex Facility.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_MUTEX_H
#define CP_MUTEX_H

#include "cpBase.h"
#include "cpMutex_I.h"

namespace cp
{

class Mutex : public Base
{
public:
    enum MutexMode { MtxNonRecursive, MtxRecursive };

    Mutex(String const &Name = "Anonymous Mutex",
          MutexMode Mode = MtxNonRecursive,
          bool PriBoost = false);

    virtual ~Mutex();

    bool Lock();
    bool TryLock();
    bool Unlock();

private:
    Mutex_t             m_Mutex;                            // native data storage
};

}   // namespace cp

#endif  // CP_MUTEX_H
