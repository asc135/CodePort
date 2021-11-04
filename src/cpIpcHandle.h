// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcHandle.h
//
//  Description:    IPC transaction handle.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
//  2013-07-17  asc Added Response() accessor method.
// ----------------------------------------------------------------------------

#ifndef CP_IPCHANDLE_H
#define CP_IPCHANDLE_H

#include "cpDatum.h"
#include "cpIpcDecoder.h"

namespace cp
{

// forward references
class IpcNode;

// ----------------------------------------------------------------------------

// this class represents the message transaction used by the application
// to collect messages received in response to unsolicited transmissions
class IpcHandle : public PooledBase
{
public:
    // constructor
    IpcHandle(IpcNode *pNode = NULL);

    // copy constructor
    IpcHandle(IpcHandle const &rhs) { *this = rhs; }

    // destructor
    ~IpcHandle();

    // assignment operator
    IpcHandle &operator=(IpcHandle const &rhs);

    // accessors
    uint32_t MsgId() const { return m_MsgId; }              // return the message ID
    bool IsGood() { return (m_MsgId != 0); }                // return validity of transaction
    IpcDecoder *Decoder() { return m_PtrDecoder; }          // return a pointer to the decoder object

    // manipulators
    void MsgId(uint32_t MsgId) { m_MsgId = MsgId; }         // set the message ID

    // communications methods
    bool WaitReply(uint32_t Timeout = k_ResponseTimeout);   // wait for a reply
    bool RegisterHandler(DispatchHandler_t pHandler);       // register a reply handler function
    Datum &Response();                                      // return the received response

private:
    uint32_t            m_MsgId;                            // message ID of original outgoing message
    IpcNode            *m_PtrNode;                          // pointer to the associated comms node
    IpcDecoder         *m_PtrDecoder;                       // pointer to a message decoder object
};

}   // namespace cp

#endif  // CP_IPCHANDLE_H
