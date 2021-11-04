// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcResolver.h
//
//  Description:    IPC Name Resolution and Caching Module.
//
//  Platform:       common
//
//  History:
//  2012-11-26  asc Creation.
//  2013-04-22  asc Added means to manually add an entry to the cache.
//  2013-08-28  asc Removed virtual default resolver method.
// ----------------------------------------------------------------------------

#ifndef CP_IPCRESOLVER_H
#define CP_IPCRESOLVER_H

#include <map>

#include "cpMutex.h"

namespace cp
{

class IpcResolver
{
public:
    // local types
    typedef uint32_t (*ResolveFunc_t)(String const &NodeName, void *pContext);
    typedef std::map<String, uint32_t, std::less<String>, Alloc< std::pair<String const, uint32_t > > > AddrMap_t;

    // constructor
    IpcResolver();

    // destructor
    ~IpcResolver() {}

    // accessors
    uint32_t NodeAddrLookup(String const &NodeName);        // request node address

    // mutators
    void Clear();                                           // flush the address cache

    void AddressAdd(cp::String const &NodeName,
                    uint32_t Address);                      // add an address to the cache

    void FunctionSet(ResolveFunc_t pFunc, void *pContext)   // set the resolver function
    {
        m_PtrFunc = pFunc;
        m_PtrContext = pContext;
    }

private:
    Mutex               m_Mutex;                            // mutex to protect the address map
    AddrMap_t           m_AddrMap;                          // map of resolved node addresses
    ResolveFunc_t       m_PtrFunc;                          // pointer to resolver function
    void               *m_PtrContext;                       // context to resolver function
};

}   // namespace cp

#endif  // CP_IPCRESOLVER_H
