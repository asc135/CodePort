// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpPooledBase.h
//
//  Description:    Pooled Memory Base Class.
//
//                  Deriving from this base class causes the derived class
//                  to be instantiated out of pooled memory.  This is done
//                  by overriding the various new and delete operators.
//
//  Platform:       common
//
//  History:
//  2012-02-29  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_POOLEDBASE_H
#define CP_POOLEDBASE_H

#include "cpPlatform.h"

namespace cp
{

class PooledBase
{
public:
    // constructor
    PooledBase()
    {
    }

    // destructor
    virtual ~PooledBase()
    {
    }

    // overridden new and delete operators to create all derived instances from the memory block pool

    static void *operator new(size_t Size) throw(std::bad_alloc);
    static void *operator new(size_t Size, const std::nothrow_t &) throw();
    static void *operator new(size_t Size, void *Ptr) throw();

    static void *operator new[](size_t Size) throw(std::bad_alloc);
    static void *operator new[](size_t Size, const std::nothrow_t &) throw();
    static void *operator new[](size_t Size, void *Ptr) throw();

    static void  operator delete(void *Ptr) throw();
    static void  operator delete(void *Ptr, const std::nothrow_t &) throw();
    static void  operator delete(void *Ptr, void *pSys) throw();

    static void  operator delete[](void *Ptr) throw();
    static void  operator delete[](void *Ptr, const std::nothrow_t &) throw();
    static void  operator delete[](void *Ptr, void *pSys) throw();

protected:

private:
};

}   // namespace cp

#endif  // CP_POOLEDBASE_H
