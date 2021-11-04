// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcNodeUtil.h
//
//  Description:    IPC endpoint node utilities.
//
//  Platform:       common
//
//  History:
//  2012-10-02  asc Creation.
//  2013-03-22  asc Added support for accumulator timeout handling.
//  2013-04-24  asc Added ReleaseThread() method.
//  2013-08-21  asc Removed inactivity timer.  Checking timeouts at message arrival.
// ----------------------------------------------------------------------------

#ifndef CP_IPCNODEUTIL_H
#define CP_IPCNODEUTIL_H

#include <map>

#include "cpTimer.h"
#include "cpIpcContext.h"
#include "cpIpcAccum.h"
#include "cpItcQueue.h"

namespace cp
{

// external declarations

// forward references
class IpcNode;
class IpcSegment;

// ----------------------------------------------------------------------------

class IpcAccumMap
{
public:
    // local types
    typedef std::map<uint64_t, IpcAccum,   std::less<uint64_t>, Alloc< std::pair<uint64_t const, IpcAccum   > > > AccumMap_t;
    typedef std::map<uint32_t, IpcContext, std::less<uint32_t>, Alloc< std::pair<uint32_t const, IpcContext > > > ContextMap_t;

    // constructor
    IpcAccumMap(IpcNode *pNode);

    // destructor
    ~IpcAccumMap();

    // accessors
    bool SubmitSegment(IpcSegment *pSegment);               // submit a segment for accumulation

    bool GetResponse(uint32_t MsgId,
                     IpcSegment *&pResponse,
                     uint32_t Timeout = k_InfiniteTimeout); // get a response from its context

    bool RegisterHandler(DispatchHandler_t pHandler,
                         uint32_t MsgId,
                         uint32_t NumThreads,
                         void *pContext);                   // register a message handler function

    bool RemoveHandler(DispatchHandler_t pHandler,
                       uint32_t MsgId);                     // remove a message handler function

    bool RemoveContext(uint32_t MsgId);                     // remove a message context
    void ReleaseThread();                                   // release the accumulator thread

private:
    static void *AccumThread(Thread *pThread);              // accumulator thread function
    static void *AccumTimerFunc(Timer *pTimer);             // Accumulator Timer Function

    IpcNode            *m_PtrNode;                          // pointer to node that owns this instance
    ItcQueue            m_AccumQueue;                       // incoming segment queue
    Thread              m_Thread;                           // segment processing thread
    Mutex               m_Mutex;                            // mutex to protect the context map
    AccumMap_t          m_AccumMap;                         // map of message segment accumulators
    ContextMap_t        m_ContextMap;                       // map of message context objects
};

// ----------------------------------------------------------------------------

class IpcTransmitQueue
{
public:
    // local types
    typedef std::list<IpcSegment *, Alloc<IpcSegment *> > PriorityQueue_t;

    // constructor
    IpcTransmitQueue();

    // destructor
    ~IpcTransmitQueue();

    // accessors
    bool SegmentGet(IpcSegment *&pSegment,
                    uint32_t Timeout = k_ReceiveTimeout);   // wait for a segment

    uint32_t TransmitMessage(IpcSegment *pSegment);         // queue up a message for transmission
    void ReleaseThread();                                   // release the transmit thread

private:
    uint32_t            m_MsgId;                            // message ID counter for this node
    Mutex               m_Mutex;                            // mutex to protect the transmit queue
    SemLite             m_SemTransmit;                      // semaphore to signal transmit thread
    PriorityQueue_t     m_PriorityQueue;                    // message transmit priority queue
};

}   // namespace cp

#endif  // CP_IPCNODEUTIL_H
