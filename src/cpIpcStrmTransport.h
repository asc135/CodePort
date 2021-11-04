// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcStrmTransport.h
//
//  Description:    IPC stream transport.
//
//  Platform:       common
//
//  History:
//  2013-04-06  asc Creation.
// ----------------------------------------------------------------------------

#ifndef CP_IPCSTRMTRANSPORT_H
#define CP_IPCSTRMTRANSPORT_H

#include "cpIpcTransport.h"

namespace cp
{

// forward references
class Buffer;

// ----------------------------------------------------------------------------

class IpcStrmTransport : public IpcTransport
{
public:
    // enumerations
    enum Constants { k_IpcStrmHeaderLen = 0x0008,
                     k_IpcStrmHeaderId  = 0x1a19 };

    enum OpCodes { OpCode_None   = 0x0000,
                   OpCode_RegMsg = 0x0001,
                   OpCode_DevMsg = 0x0002,
                   OpCode_IpcMsg = 0x0003  };

    // constructor
    IpcStrmTransport(String const &Name);

    // destructor
    virtual ~IpcStrmTransport();

    // input/output functions
    virtual bool Send(IpcSegment *pSeg, uint32_t Timeout = cp::k_InfiniteTimeout);
    virtual bool Recv(IpcSegment *pSeg, uint32_t Timeout = cp::k_InfiniteTimeout);

    // utility methods
    static bool EncodeHeader(cp::Buffer       &Header, OpCodes  OpCode, size_t  Length);
    static bool DecodeHeader(cp::Buffer const &Header, OpCodes &OpCode, size_t &Length);
};

}   // namespace cp

#endif  // CP_IPCSTRMTRANSPORT_H
