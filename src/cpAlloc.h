// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpAlloc.h
//
//  Description:    Custom STL Allocator.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_ALLOC_H
#define CP_ALLOC_H

#include <cstddef>

#include "cpMemMgr.h"

namespace cp
{

template<class T>
class Alloc
{
public:
    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;
    typedef T        *pointer;
    typedef const T  *const_pointer;
    typedef T        &reference;
    typedef const T  &const_reference;
    typedef T         value_type;

    Alloc() throw()
    {
    }

    Alloc(const Alloc &) throw()
    {
    }

    ~Alloc() throw()
    {
    }

    pointer allocate(size_type n, const void *p = 0)
    {
        MemBlock *pBlk;
        char *pMem = NULL;

        // avoid compiler warning
        (void)p;

        if (MemManager::InstanceGet()->MemBlockGet(pBlk, (n * sizeof(T))))
        {
            pMem = pBlk->BuffGet();
        }

        T *t = (T *)pMem;
//        std::cout << "  used Alloc to allocate   at address " << reinterpret_cast<void *>(t) << " (+)" << std::endl;
        return t;
    }

    void deallocate(void *p, size_type)
    {
        char *pMem = reinterpret_cast<char * &>(p);

        if (pMem)
        {
            MemManager::InstanceGet()->MemBlockPut(pMem);
//            std::cout << "  used Alloc to deallocate at address " << p << " (-)" << std::endl;
        }
    }

    pointer address(reference x) const
    {
        return &x;
    }

    const_pointer address(const_reference x) const
    {
        return &x;
    }

    Alloc<T> &operator=(const Alloc &)
    {
        return *this;
    }

    void construct(pointer p, const T &val)
    {
        new ((T *) p) T(val);
    }

    void destroy(pointer p)
    {
        p->~T();
    }

    size_type max_size() const
    {
        return size_t(-1);
    }

    template<class U> struct rebind
    {
        typedef Alloc<U> other;
    };

    template<class U> Alloc(const Alloc<U> &)
    {
    }

    template<class U> Alloc &operator=(const Alloc<U> &)
    {
        return *this;
    }
};

template <class T1, class T2>
bool operator==(Alloc<T1> const &, Alloc<T2> const &) throw()
{
    return true;
}

template <class T1, class T2>
bool operator!=(Alloc<T1> const &, Alloc<T2> const &) throw()
{
    return false;
}

}   // namespace cp

#endif  // CP_ALLOC_H
