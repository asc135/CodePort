// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcDecoder.h
//
//  Description:    IPC message decoder.
//
//  Platform:       common
//
//  History:
//  2013-01-18  asc Creation.
// ----------------------------------------------------------------------------

#ifndef CP_IPCDECODER_H
#define CP_IPCDECODER_H

#include "cpDatum.h"
#include "cpDispatch.h"
#include "cpIpcStreamSeg.h"

namespace cp
{

class IpcDecoder : public PooledBase
{
public:
    // constructor
    IpcDecoder() :
        m_Message("Decoded")
    {}

    // destructor
    ~IpcDecoder() {}

    // accessors
    Buffer &Buf()                       { return m_Buffer;  }
    Datum &Msg()                        { return m_Message; }
    IpcStreamSeg &Stream()              { return m_Stream;  }

    // mutators
    bool LoadSegment(IpcSegment *pSeg);                     // load a segment list into the decoder

private:
    // copy constructors
    IpcDecoder(IpcDecoder &rhs);

    // operators
    IpcDecoder &operator=(IpcDecoder &rhs);

    Buffer              m_Buffer;                           // buffer containing decoded message
    Datum               m_Message;                          // Datum containing decoded message
    IpcStreamSeg        m_Stream;                           // segment stream used to decode incoming Datum
};

}   // namespace cp

#endif  // CP_IPCDECODER_H
