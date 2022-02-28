// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpPooledBase.cpp
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
//  2022-02-28  asc Updated exception handling for C++11 and newer.
// ----------------------------------------------------------------------------

#include <new>

#include "cpPooledBase.h"
#include "cpMemMgr.h"

namespace cp
{

// overridden new operator (throws exceptions)
void *PooledBase::operator new(size_t Size) noexcept(false)
{
    MemBlock *pMem = NULL;

    if (MemManager::InstanceGet()->MemBlockGet(pMem, Size) == false)
    {
        LogErr << "PooledBase::operator new(): Failed to acquire memory block of size: "
               << Size << std::endl;
        throw std::bad_alloc();
    }

    return pMem->BuffGet();
}


// overridden new operator (doesn't throw exceptions)
void *PooledBase::operator new(size_t Size, const std::nothrow_t &) throw()
{
    MemBlock *pMem = NULL;

    if (MemManager::InstanceGet()->MemBlockGet(pMem, Size) == false)
    {
        LogErr << "PooledBase::operator new(): Failed to acquire memory block of size: "
               << Size << std::endl;
    }

    return pMem->BuffGet();
}


// overridden placement new operator (doesn't throw exceptions)
void *PooledBase::operator new(size_t Size, void *Ptr) throw()
{
    return ::operator new(Size, Ptr);
}


// overridden array new operator (throws exceptions)
void *PooledBase::operator new[](size_t Size) noexcept(false)
{
    MemBlock *pMem = NULL;

    if (MemManager::InstanceGet()->MemBlockGet(pMem, Size) == false)
    {
        LogErr << "PooledBase::operator new[](): Failed to acquire memory block of size: "
               << Size << std::endl;
        throw std::bad_alloc();
    }

    return pMem->BuffGet();
}


// overridden array new operator (doesn't throw exceptions)
void *PooledBase::operator new[](size_t Size, const std::nothrow_t &) throw()
{
    MemBlock *pMem = NULL;

    if (MemManager::InstanceGet()->MemBlockGet(pMem, Size) == false)
    {
        LogErr << "PooledBase::operator new[](): Failed to acquire memory block of size: "
               << Size << std::endl;
    }

    return pMem->BuffGet();
}


// overridden array placement new operator (doesn't throw exceptions)
void *PooledBase::operator new[](size_t Size, void *Ptr) throw()
{
    return ::operator new[](Size, Ptr);
}


// overridden delete operator (throws exceptions)
void  PooledBase::operator delete(void *Ptr) throw()
{
    if (MemManager::InstanceGet()->MemBlockPut(reinterpret_cast<char * &>(Ptr)) == false)
    {
        LogErr << "PooledBase::operator delete(): Failed to return memory block: "
               << Ptr << std::endl;
    }
}


// overridden delete operator (doesn't throw exceptions)
void  PooledBase::operator delete(void *Ptr, const std::nothrow_t &) throw()
{
    if (MemManager::InstanceGet()->MemBlockPut(reinterpret_cast<char * &>(Ptr)) == false)
    {
        LogErr << "PooledBase::operator delete(): Failed to return memory block: "
               << Ptr << std::endl;
    }
}


// overridden placement delete operator (doesn't throw exceptions)
void  PooledBase::operator delete(void *Ptr, void *pSys) throw()
{
    ::operator delete(Ptr, pSys);
}


// overridden array delete operator (throws exceptions)
void  PooledBase::operator delete[](void *Ptr) throw()
{
    if (MemManager::InstanceGet()->MemBlockPut(reinterpret_cast<char * &>(Ptr)) == false)
    {
        LogErr << "PooledBase::operator delete[](): Failed to return memory block: "
               << Ptr << std::endl;
    }
}


// overridden array delete operator (doesn't throw exceptions)
void  PooledBase::operator delete[](void *Ptr, const std::nothrow_t &) throw()
{
    if (MemManager::InstanceGet()->MemBlockPut(reinterpret_cast<char * &>(Ptr)) == false)
    {
        LogErr << "PooledBase::operator delete[](): Failed to return memory block: "
               << Ptr << std::endl;
    }
}


// overridden array placement delete operator (doesn't throw exceptions)
void  PooledBase::operator delete[](void *Ptr, void *pSys) throw()
{
    ::operator delete[](Ptr, pSys);
}

}   // namespace cp
