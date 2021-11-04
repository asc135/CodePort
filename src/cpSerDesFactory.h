// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSerDesFactory.h
//
//  Description:    Serializer/Deserializer instance factory.
//
//  Platform:       common
//
//  History:
//  2011-06-23  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_SERDESFACTORY_H
#define CP_SERDESFACTORY_H

#include <map>

#include "cpMutex.h"

namespace cp
{

class SerDes;
class StreamBase;

// ----------------------------------------------------------------------------

class SerDesFactory
{
public:
    // type definitions
    typedef std::list<SerDes *> SerDesPool_t;
    typedef std::map<String, SerDesPool_t> SerDesPoolMap_t;

    // destructor
    ~SerDesFactory();

    // singleton instance get method
    static SerDesFactory *InstanceGet();

    // accessors
    String DetectEncoding(StreamBase &Stream);              // detect the encoding of a serialized stream
    SerDes *SerDesGet(String const &Enc);                   // acquire an encoder by name
    bool SerDesPut(SerDes * &pSerDes);                      // return an encoder

private:
    // constructor
    SerDesFactory();

    Mutex                   m_Mutex;                        // thread protection mutex
    SerDesPoolMap_t         m_Pools;                        // encoder pools
    static SerDesFactory   *m_PtrInstance;                  // singleton instance
};

}   // namespace cp

#endif  // CP_SERDESFACTORY_H
