// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcResolver.cpp
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

#include "cpIpcResolver.h"

namespace cp
{

// constructor
IpcResolver::IpcResolver() :
    m_Mutex("IPC Resolver Address Map Mutex"),
    m_PtrFunc(NULL),
    m_PtrContext(NULL)
{
}


// request node address
uint32_t IpcResolver::NodeAddrLookup(String const &NodeName)
{
    uint32_t rv = 0;
    AddrMap_t::iterator i;

    m_Mutex.Lock();

    i = m_AddrMap.find(NodeName);

    // if found in cache, save the result
    if (i != m_AddrMap.end())
    {
        rv = i->second;
    }

    m_Mutex.Unlock();

    // if not found in cache, request address from resolver
    if (rv == 0)
    {
        if (m_PtrFunc)
        {
            // if defined, call the function pointer
            rv = (*m_PtrFunc)(NodeName, m_PtrContext);
        }

        // if resolved, add it to the cache
        if (rv != 0)
        {
            AddressAdd(NodeName, rv);
        }
    }

    return rv;
}


// flush the address cache
void IpcResolver::Clear()
{
    m_Mutex.Lock();
    m_AddrMap.clear();
    m_Mutex.Unlock();
}


// add an address to the cache
void IpcResolver::AddressAdd(cp::String const &NodeName, uint32_t Address)
{
    m_Mutex.Lock();
    m_AddrMap[NodeName] = Address;
    m_Mutex.Unlock();
}

}   // namespace cp
