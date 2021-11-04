// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcNodeUtil.cpp
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

#include "cpIpcNode.h"
#include "cpIpcSegment.h"

namespace cp
{

// constructor
IpcAccumMap::IpcAccumMap(IpcNode *pNode) :
    m_PtrNode(pNode),
    m_AccumQueue("IPC Accumulator Queue", k_IpcAccumQueueDepth),
    m_Thread("IPC Accumulator Thread", AccumThread, this),
    m_Mutex("IPC Context Mutex")
{
}


// destructor
IpcAccumMap::~IpcAccumMap()
{
    IpcSegment *pSegment = NULL;

    // send exit request to accumulator thread
    m_Thread.ExitReq();

    // release the thread
    ReleaseThread();

    // wait for it to exit
    m_Thread.WaitExit(k_ReceiveTimeout);

    // drain the queue
    while (m_AccumQueue.Get(reinterpret_cast<void * &>(pSegment), 0))
    {
        if (pSegment)
        {
            delete pSegment;
            pSegment = NULL;
        }
    }
}


// submit a segment for accumulation
bool IpcAccumMap::SubmitSegment(IpcSegment *pSegment)
{
    bool rv = (pSegment != NULL);

    if (rv)
    {
        // submit segment for accumulation
        rv = m_AccumQueue.Put(pSegment, k_DefaultTimeout);

        // delete segment if it fails to fit in the queue
        if (!rv)
        {
            LogErr << "IpcAccumMap::SubmitSegment(): Failed to submit segment to accumulator queue."
                   << std::endl;

            delete pSegment;
        }
    }

    return rv;
}


bool IpcAccumMap::GetResponse(uint32_t MsgId, IpcSegment *&pResponse, uint32_t Timeout)
{
    bool rv = false;
    IpcContext *pContext = NULL;

    // find or create context
    m_Mutex.Lock();
    pContext = &m_ContextMap[MsgId];
    m_Mutex.Unlock();

    // wait for and retrieve response
    rv = pContext->MessageGet(pResponse, Timeout);

    // remove context entry if response successfully retrieved
    if (rv)
    {
        RemoveContext(MsgId);
    }

    return rv;
}


// register a message handler function
bool IpcAccumMap::RegisterHandler(DispatchHandler_t pHandler, uint32_t MsgId, uint32_t NumThreads, void *pContext)
{
    bool rv = false;

    m_Mutex.Lock();
    rv = m_ContextMap[MsgId].RegisterHandler(pHandler, NumThreads, pContext);
    m_Mutex.Unlock();

    return rv;
}


// remove a message handler function
bool IpcAccumMap::RemoveHandler(DispatchHandler_t pHandler, uint32_t MsgId)
{
    bool rv = false;
    ContextMap_t::iterator i;

    m_Mutex.Lock();

    i = m_ContextMap.find(MsgId);

    if (i != m_ContextMap.end())
    {
        rv = i->second.RemoveHandler(pHandler);
    }

    m_Mutex.Unlock();

    return rv;
}


// remove a message context
bool IpcAccumMap::RemoveContext(uint32_t MsgId)
{
    bool rv = (MsgId != 0);
    ContextMap_t::iterator i;

    // abort if attempt to delete context 0
    if (!rv)
    {
        LogErr << "IpcAccumMap::RemoveContext(): Attempt to delete context 0 prevented, instance: "
               << this << std::endl;
        return rv;
    }

    m_Mutex.Lock();

    i = m_ContextMap.find(MsgId);
    rv = (i != m_ContextMap.end());

    if (rv)
    {
        m_ContextMap.erase(i);
    }
    else
    {
        LogErr << "IpcAccumMap::RemoveContext(): Could not locate context: "
               << MsgId << ", instance: " << this << std::endl;
    }

    m_Mutex.Unlock();

    return rv;
}


// release the accumulator thread
void IpcAccumMap::ReleaseThread()
{
    // submit a NULL pointer
    m_AccumQueue.Put(NULL, k_DefaultTimeout);
}


// accumulator thread function
void *IpcAccumMap::AccumThread(Thread *pThread)
{
    IpcAccumMap *pAccumMap = reinterpret_cast<IpcAccumMap *>(pThread->ContextGet());
    IpcSegment *pSegment = NULL;
    AccumMap_t::iterator i;

    while (pThread->ThreadPoll())
    {
        if (pAccumMap->m_AccumQueue.Get(reinterpret_cast<void *&>(pSegment), k_ReceiveTimeout))
        {
            // if timer expired or thread exit was requested
            if ((pSegment == NULL) || (pThread->ExitFlag()))
            {
                // do some housekeeping
                i = pAccumMap->m_AccumMap.begin();

                while (i != pAccumMap->m_AccumMap.end())
                {
                    // if accumulator is expired or thread exit has been requested
                    if ((i->second.Expired()) || (pThread->ExitFlag()))
                    {
                        // alert the node and delete the accumulator
                        pAccumMap->m_PtrNode->ExpiredAccumNotify(i->second.Head());
                        pAccumMap->m_AccumMap.erase(i);
                    }

                    ++i;
                }

                // if pSegment was not NULL and exit was requested
                if (pSegment)
                {
                    delete pSegment;
                    pSegment = NULL;
                }
            }
            else
            {
                // if a multi-part message, hand it to accumulator to reassemble
                if ((pSegment->Options() & IpcSegment::opt_Multipart))
                {
                    // get the globally unique message id
                    uint64_t guid = pSegment->Guid();

                    // get a reference to the appropriate accumulator
                    IpcAccum &accum = pAccumMap->m_AccumMap[guid];

                    // submit the segment to its accumulator
                    accum.SubmitSegment(pSegment);
                    pSegment = NULL;

                    // check if entire message has been reassembled
                    if (accum.Complete())
                    {
                        // get the complete message from accumulator
                        accum.MessageGet(pSegment);

                        // delete the accumulator
                        pAccumMap->m_AccumMap.erase(guid);
                    }
                    else
                    {
                        // update the accumulation timeout
                        accum.ResetTimeout(k_IpcAccumulatorTimeout);
                    }
                }

                // if single segment or a complete reassembled message
                if (pSegment)
                {
                    // alert the node of a valid message accumulation
                    pAccumMap->m_PtrNode->ValidMessageNotify(pSegment);

                    uint32_t msgId = pSegment->Context();
                    bool stored = true;

                    // put the message into its context object
                    pAccumMap->m_Mutex.Lock();
                    stored = pAccumMap->m_ContextMap[msgId].MessagePut(pSegment);
                    pAccumMap->m_Mutex.Unlock();

                    // transferred ownership
                    pSegment = NULL;

                    // if it wasn't stored, it was forwarded to a dispatcher
                    // so it is safe to remove the context object now unless
                    // it is for context ID 0 which is a persistent context
                    if (!stored && (msgId != 0))
                    {
                        pAccumMap->RemoveContext(msgId);
                    }
                }
            }
        }
    }

    return NULL;
}

// ----------------------------------------------------------------------------

// constructor
IpcTransmitQueue::IpcTransmitQueue() :
    m_MsgId(0),
    m_Mutex("IPC Transmit Queue Mutex"),
    m_SemTransmit("IPC Transmit Queue Semaphore", 0)
{
}


// destructor
IpcTransmitQueue::~IpcTransmitQueue()
{
    IpcSegment *pSeg = NULL;

    // empty the transmit queue
    while (SegmentGet(pSeg, 0))
    {
        LogMsg << "IpcTransmitQueue::~IpcTransmitQueue(): Purging IpcSegment: "
               << pSeg << std::endl;
        delete pSeg;
        pSeg = NULL;
    }
}


// wait for a segment
bool IpcTransmitQueue::SegmentGet(IpcSegment *&pSegment, uint32_t Timeout)
{
    bool rv = m_SemTransmit.Take(Timeout);
    IpcSegment *pSeg = NULL;

    if (rv)
    {
        m_Mutex.Lock();

        if (m_PriorityQueue.size() > 0)
        {
            // get the next segment in the transmit queue
            pSegment = m_PriorityQueue.front();
            m_PriorityQueue.pop_front();

            if (pSegment)
            {
                // check if there are any attached segments
                pSeg = pSegment->NextGet();

                if (pSeg)
                {
                    // assign the message ID to the attached segment
                    pSeg->MsgId(pSegment->MsgId());

                    // store the attached segment back into the queue
                    m_PriorityQueue.push_front(pSeg);

                    // clear the taken segment's next pointer
                    pSegment->NextSet(NULL);
                }
            }
            else
            {
                rv = false;
            }
        }
        else
        {
            // for some reason the semaphore was signalled but there
            // were no entries in the transmit queue...
            LogErr << "IpcTransmitQueue::SegmentGet(): Empty transmit queue was signalled, instance: "
                   << this << std::endl;

            rv = false;
        }

        // if queue still has entries keep the semaphore in the signalled state
        if (m_PriorityQueue.size() > 0)
        {
            if (m_SemTransmit.CountGet() == 0)
            {
                m_SemTransmit.Give();
            }
        }

        m_Mutex.Unlock();
    }

    return rv;
}


// queue up a message for transmission
uint32_t IpcTransmitQueue::TransmitMessage(IpcSegment *pSegment)
{
    uint32_t rv = 0;
    bool inserted = false;

    // make sure pointer is valid
    if (pSegment == NULL)
    {
        // since 0 is never used for a valid message ID it serves as the error code
        return 0;
    }

    // get the segment transmit priority
    uint8_t priority = pSegment->Priority();

    // synchronize access to transmit queue
    m_Mutex.Lock();

    // increment message ID and skip over 0 at wrap-around
    if (++m_MsgId == 0)
    {
        ++m_MsgId;
    }

    // assign the message ID to the first (or only) segment in a message
    // the transmit thread will assign the message ID to the rest of the
    // message's segments (if any) as it sends them to the transport device
    pSegment->MsgId(m_MsgId);
    rv = m_MsgId;

    // create an iterator to the transmit queue
    PriorityQueue_t::iterator i = m_PriorityQueue.begin();

    // iterate over any existing entries
    while (i != m_PriorityQueue.end())
    {
        if (priority < (*i)->Priority())
        {
            // insert it before an item with lower priority (numerically higher value)
            m_PriorityQueue.insert(i, pSegment);
            inserted = true;
            break;
        }

        ++i;
    }

    // if it wasn't inserted by priority, put it at end of transmit queue
    if (!inserted)
    {
        m_PriorityQueue.push_back(pSegment);
    }

    m_Mutex.Unlock();

    // signal the transmit thread
    m_SemTransmit.Give();

    return rv;
}


// release the transmit thread
void IpcTransmitQueue::ReleaseThread()
{
    // put a NULL segment in the queue
    m_Mutex.Lock();
    m_PriorityQueue.push_back(NULL);
    m_Mutex.Unlock();

    // signal the transmit thread
    m_SemTransmit.Give();
}

}   // namespace cp
