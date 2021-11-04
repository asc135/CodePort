// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcContext.h
//
//  Description:    IPC message context.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
//  2013-08-22  asc Moved storage of event context into dispatch object.
// ----------------------------------------------------------------------------

#ifndef CP_IPCCONTEXT_H
#define CP_IPCCONTEXT_H

#include "cpDispatch.h"

namespace cp
{

// forward references
class IpcNode;
class IpcSegment;

// ----------------------------------------------------------------------------

// a context object directs a response message to
// the original thread that sent the request message
class IpcContext
{
public:
    // local custom types
    typedef std::vector<SemLite *, Alloc<SemLite *> > SemPool_t;    // (.)(.) 2013-02-05 this might be leaking OS condition variables and mutex resources

    // constructor
    IpcContext();

    // copy constructor
    IpcContext(IpcContext const &rhs);

    // destructor
    ~IpcContext();

    // operators
    IpcContext &operator=(IpcContext const &rhs);

    bool MessagePut(IpcSegment *pSegment);                  // put a message into the context

    bool MessageGet(IpcSegment *&pSegment,
                    uint32_t Timeout = k_InfiniteTimeout);  // get a message from the context

    bool RegisterHandler(DispatchHandler_t pHandler,
                         uint32_t NumThreads,
                         void *pContext);                   // register a message handler function

    bool RemoveHandler(DispatchHandler_t pHandler);         // remove a message handler function

private:
    bool GetSemFromPool(SemLite *&pSem);                    // get a semaphore from the pool
    void PutSemIntoPool(SemLite *&pSem);                    // put a semaphore back in the pool

    IpcSegment         *m_PtrHead;                          // head of linked list of received message segments
    SemLite            *m_PtrSem;                           // semaphore used to block and signal receive client
    Dispatch           *m_PtrDispatcher;                    // pointer to a message dispatch instance
    static Mutex        m_MtxSemPool;                       // mutex to protect semaphore pool
    static SemPool_t    m_SemPool;                          // a container to hold semaphores
};

}   // namespace cp

#endif  // CP_IPCCONTEXT_H
