// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcContext.cpp
//
//  Description:    IPC message context.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
//  2013-08-22  asc Moved storage of event context into dispatch object.
// ----------------------------------------------------------------------------

#include "cpIpcContext.h"
#include "cpIpcPacket.h"

namespace cp
{

// mutex to protect semaphore pool
Mutex IpcContext::m_MtxSemPool("Semaphore Pool Mutex");

// a container to hold semaphores
IpcContext::SemPool_t IpcContext::m_SemPool;


// pre-dispatch handler
void *PreDispatch(DispatchEvent *pDispEvent)
{
    IpcSegment *pSeg = NULL;
    IpcPacket *pPkt = NULL;

    // check if event is real
    if (pDispEvent)
    {
        pSeg = reinterpret_cast<IpcSegment *>(pDispEvent->pEvent);
    }

    // check if segment exists
    if (pSeg)
    {
        pPkt = new (CP_NEW) IpcPacket;
    }

    // check if a packet was created
    if (pPkt)
    {
        // take ownership of segments and decode
        if (!pPkt->PtrSeg(pSeg))
        {
            LogErr << "IpcContext::PreDispatch(): Failed to decode message."
                   << std::endl;
        }

        // assign the event to be the packet so it is accessed
        // as such by all handlers in the dispatch stack
        pDispEvent->pEvent = pPkt;
    }
    else
    {
        delete pSeg;
        pDispEvent->pEvent = NULL;
        LogErr << "IpcContext::PreDispatch(): Failed to create an IPC packet." << std::endl;
    }

    return pDispEvent;
}


// post-dispatch handler
void *PostDispatch(DispatchEvent *pDispEvent)
{
    IpcPacket *pPkt = NULL;

    // check if event is real
    if (pDispEvent)
    {
        pPkt = reinterpret_cast<IpcPacket *>(pDispEvent->pEvent);
    }

    // delete the packet
    if (pPkt)
    {
        delete pPkt;
    }

    return pDispEvent;
}


// constructor
IpcContext::IpcContext() :
    m_PtrHead(NULL),
    m_PtrSem(NULL),
    m_PtrDispatcher(NULL)
{
    // acquire a semaphore from the pool
    GetSemFromPool(m_PtrSem);
}


// copy constructor
IpcContext::IpcContext(IpcContext const &rhs) :
    m_PtrHead(NULL),
    m_PtrSem(NULL),
    m_PtrDispatcher(NULL)
{
    // invoke assignment operator
    *this = rhs;
}


// destructor
IpcContext::~IpcContext()
{
    // purge any message segments
    if (m_PtrHead)
    {
        delete m_PtrHead;
    }

    // return the semaphore object to pool
    PutSemIntoPool(m_PtrSem);

    // delete the dispatch object
    if (m_PtrDispatcher)
    {
        delete m_PtrDispatcher;
    }
}


// assignment operator
IpcContext &IpcContext::operator=(IpcContext const &rhs)
{
    // detect self assignment
    if (this != &rhs)
    {
        m_PtrHead = rhs.m_PtrHead;

        // acquire a semaphore from the pool
        GetSemFromPool(m_PtrSem);

        m_PtrDispatcher = rhs.m_PtrDispatcher;
    }

    return *this;
}


// put a message into the context
bool IpcContext::MessagePut(IpcSegment *pSegment)
{
    // returns true if message was stored and false
    // if it was passed directly to a dispatcher
    bool rv = (m_PtrDispatcher == NULL);

    // check for invalid pointer
    if (pSegment == NULL)
    {
        // pretend it was stored
        LogErr << "IpcContext::MessagePut(): Prevented attempt to store a NULL segment pointer, instance: "
               << this << std::endl;
        return true;
    }

    if (rv)
    {
        // remove any existing message segments
        if (m_PtrHead != NULL)
        {
            delete m_PtrHead;
        }

        // store the new message
        m_PtrHead = pSegment;

        // signal the arrival of the message
        if (m_PtrSem)
        {
            m_PtrSem->Give();
        }
    }
    else
    {
        if (m_PtrDispatcher->SubmitEvent(pSegment, k_DefaultTimeout) ==  false)
        {
            LogErr << "IpcContext::MessagePut(): Failed to submit event to registered dispatch object, instance: "
                   << this << std::endl;
            delete pSegment;
        }
    }

    return rv;
}


// get a message from the context
bool IpcContext::MessageGet(IpcSegment *&pSegment, uint32_t Timeout)
{
    bool rv = false;

    // check if a dispatcher has been registered
    if (m_PtrDispatcher)
    {
        // (.)(.) for now, just don't allow manual intervention
        return false;
    }

    if (m_PtrSem)
    {
        rv = m_PtrSem->Take(Timeout);
    }

    if (rv && m_PtrHead)
    {
        pSegment = m_PtrHead;
        m_PtrHead = NULL;
    }

    return rv;
}


// register a message handler function
bool IpcContext::RegisterHandler(DispatchHandler_t pHandler, uint32_t NumThreads, void *pContext)
{
    // check if a dispatcher instance needs to be created
    if (m_PtrDispatcher == NULL)
    {
        // otherwise create a new dispatch object
        m_PtrDispatcher = new (CP_NEW) Dispatch(NumThreads);
    }

    // set new handler and number of execution threads
    if (m_PtrDispatcher)
    {
        if (pHandler)
        {
            // register pre and post handler functions
            m_PtrDispatcher->PreDispatchSet(PreDispatch, NULL);
            m_PtrDispatcher->PostDispatchSet(PostDispatch, NULL);

            // register event handler and set number of threads
            m_PtrDispatcher->EventHandlerAdd(pHandler, pContext);
            m_PtrDispatcher->NumThreadsSet(NumThreads);
        }
    }
    else
    {
        LogErr << "IpcContext::RegisterHandler(): Failed to create an IPC dispatch object, instance: "
               << this << std::endl;
    }

    return (m_PtrDispatcher != NULL);
}


// remove a message handler function
bool IpcContext::RemoveHandler(DispatchHandler_t pHandler)
{
    bool rv = false;

    // set new handler and number of execution threads
    if (m_PtrDispatcher)
    {
        rv = m_PtrDispatcher->EventHandlerDel(pHandler);
    }

    return rv;
}


// get a semaphore from the pool
bool IpcContext::GetSemFromPool(SemLite *&pSem)
{
    // check if pointer already has a valid semaphore
    if (pSem)
    {
        return true;
    }

    // synchronize access to the pool container
    m_MtxSemPool.Lock();

    // check if any semaphore objects are in inventory
    if (m_SemPool.size() > 0)
    {
        // take one from the end
        pSem = m_SemPool.back();
        m_SemPool.pop_back();
    }

    m_MtxSemPool.Unlock();

    // if container didn't have one to supply, create one
    if (pSem == NULL)
    {
        pSem = new (CP_NEW) SemLite("Comm Handle Semaphore", 0, 1);
    }

    return (pSem != NULL);
}


// put a semaphore back in the pool
void IpcContext::PutSemIntoPool(SemLite *&pSem)
{
    if (pSem)
    {
        // synchronize access to the pool container
        m_MtxSemPool.Lock();

        // make sure it is empty
        pSem->TryTake();

        // add it to the end
        m_SemPool.push_back(pSem);
        pSem = NULL;

        m_MtxSemPool.Unlock();
    }
}

}   // namespace cp
