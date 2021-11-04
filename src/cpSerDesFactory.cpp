// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSerDesFactory.cpp
//
//  Description:    Serializer/Deserializer instance factory.
//
//  Platform:       common
//
//  History:
//  2011-06-23  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-11-15  asc Added IDL ser/des.
// ----------------------------------------------------------------------------

#include "cpSerDesNative.h"
#include "cpSerDesXml.h"
#include "cpSerDesIdl.h"

namespace cp
{

// singleton instance
SerDesFactory *SerDesFactory::m_PtrInstance = NULL;

// ----------------------------------------------------------------------------

// constructor
SerDesFactory::SerDesFactory() :
    m_Mutex("SerDes Factory Mutex")
{
    SerDes *ptr;

    // populate the pool with intrinsic serializers
    ptr = new (CP_NEW) SerDesNative;
    SerDesPut(ptr);

    ptr = new (CP_NEW) SerDesXml;
    SerDesPut(ptr);

    ptr = new (CP_NEW) SerDesIdl;
    SerDesPut(ptr);
}


// destructor
SerDesFactory::~SerDesFactory()
{
}


// singleton instance get method
SerDesFactory *SerDesFactory::InstanceGet()
{
    if (m_PtrInstance == NULL)
    {
        m_PtrInstance = new (CP_NEW) SerDesFactory;
    }

    if (m_PtrInstance == NULL)
    {
        LogErr << "SerDesFactory::InstanceGet(): Failed to create SerDesFactory instance."
               << std::endl;
    }

    return m_PtrInstance;
}


// detect the encoding of a serialized stream
String SerDesFactory::DetectEncoding(StreamBase &Stream)
{
    String rv;
    SerDesPoolMap_t::iterator i;

    m_Mutex.Lock();

    i = m_Pools.begin();

    while (i != m_Pools.end())
    {
        if ((i->second.size() > 0) && (i->second.front()->CheckEncoding(Stream)))
        {
            rv = (i->first);
            break;
        }

        ++i;
    }

    m_Mutex.Unlock();

    return rv;
}


// acquire an encoder
SerDes *SerDesFactory::SerDesGet(String const &Enc)
{
    SerDes *rv = NULL;
    SerDesPoolMap_t::iterator i;

    m_Mutex.Lock();

    // search for the encoder type in the pool map
    i = m_Pools.find(Enc);

    if (i != m_Pools.end())
    {
        // there should always be a minimum of one encoder in each pool
        if (i->second.size() > 0)
        {
            // see how many are in the pool and always keep at least one
            // so you can make more using its CreateInstance() method
            if (i->second.size() > 1)
            {
                // if more then one, return one of them
                rv = i->second.front();
                i->second.pop_front();
            }
            else
            {
                // if just one, create another and return that
                rv = i->second.front()->CreateInstance();
            }
        }
        else
        {
            LogErr << "SerDesFactory::SerDesGet(): Error - found an empty serializer pool: "
                   << Enc << std::endl;
        }
    }

    m_Mutex.Unlock();

    return rv;
}


// return an encoder
bool SerDesFactory::SerDesPut(SerDes * &pSerDes)
{
    bool rv = false;

    if (pSerDes != NULL)
    {
        m_Mutex.Lock();
        m_Pools[pSerDes->NameGet()].push_back(pSerDes);
        m_Mutex.Unlock();
        pSerDes = NULL;
        rv = true;
    }

    return rv;
}

}   // namespace cp
