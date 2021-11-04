// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpQueue.h
//
//  Description:    Inter-Process Message Queue Facility.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Added timeout parameter to SendData() and recvData().
// ----------------------------------------------------------------------------

#ifndef CP_QUEUE_H
#define CP_QUEUE_H

#include "cpIoDev.h"
#include "cpQueue_I.h"

namespace cp
{

class Queue : public IoDev
{
public:
    // constructor, owner
    Queue(String const &Name, size_t MsgSize, uint32_t MaxMsgs);

    // constructor, user
    Queue(String const &Name);

    // destructor
    virtual ~Queue();

    // manipulators
    virtual void Flush();

    // accessors
    bool   IsFull() const;
    bool   IsEmpty() const;
    uint32_t NumFree() const;
    uint32_t NumUsed() const;
    uint32_t MaxMsgsGet() const;

protected:
    virtual bool SendReady(uint32_t Timeout);               // returns true if device ready for send
    virtual bool RecvReady(uint32_t Timeout);               // returns true if device ready for receive
    virtual int SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout);
    virtual int RecvData(char       *pBuf, size_t RcvLen, size_t BytesRead,    uint32_t Timeout);

private:
    bool                m_Cleanup;                          // context that creates device needs to delete it
    size_t              m_MsgSize;                          // queue message size
    uint32_t            m_MaxMsgs;                          // maximum number of queue messages
    Queue_t             m_MsgQueue;                         // native data storage
    String              m_SysName;                          // native instance name
};

}   // namespace cp

#endif  // CP_QUEUE_H
