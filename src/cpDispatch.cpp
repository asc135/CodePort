// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpDispatch.cpp
//
//  Description:    Event dispatch base class.
//
//  Platform:       common
//
//  History:
//  2012-10-03  asc Creation.
//  2012-12-13  asc Added multiple dispatch thread capability.
//  2013-01-10  asc Improved thread management mechanism.
//  2013-08-22  asc Added support for multiple dispatch functions.
//  2013-08-23  asc Added support for pre-dispatch and post-dispatch handlers.
//  2013-08-26  asc Allowed user to specify queue depth.  Cleaned up shutdown.
// ----------------------------------------------------------------------------

#include "cpDispatch.h"

namespace cp
{

// constructor
Dispatch::Dispatch(uint32_t NumThreads, uint32_t EventQueueDepth) :
    m_StackMutex("Dispatch Stack Mutex"),
    m_EventQueue("Event Queue Pipe", EventQueueDepth)
{
    // use the general purpose flag 1 of event queue to control thread launch
    m_EventQueue.Flag1Set(true);
    NumThreadsSet(NumThreads);
}


// disabled copy constructor
Dispatch::Dispatch(Dispatch const &rhs) :
    m_StackMutex("Dispatch Stack Mutex"),
    m_EventQueue("Event Queue Pipe", rhs.m_EventQueue.Capacity())
{
    // invoke assignment operator
    *this = rhs;
}


// destructor
Dispatch::~Dispatch()
{
    m_StackMutex.Lock();
    uint32_t threadCount = m_Threads.size();

    // disable thread create operations
    m_EventQueue.Flag1Set(false);

    m_StackMutex.Unlock();

    // send as many shutdown signals as there are dispatch threads
    for (uint32_t i = 0; i < threadCount; ++i)
    {
        Shutdown();
    }

    // delete the dispatch threads
    NumThreadsSet(0);
}


// disabled assignment operator
Dispatch &Dispatch::operator=(Dispatch const &rhs)
{
    (void)rhs;
    return *this;
}


// submit an event for dispatch
bool Dispatch::SubmitEvent(void *pEvent, uint32_t Timeout)
{
    bool rv = false;
    DispatchEvent *pDispEvent = GenEvent();

    if (pDispEvent)
    {
        pDispEvent->OpCode   = opc_NewEvent;
        pDispEvent->pEvent   = pEvent;

        rv = m_EventQueue.Put(pDispEvent, Timeout);

        // clean up on failure to insert instance into queue
        if (!rv)
        {
            delete pDispEvent;
            LogErr << "Dispatch::SubmitEvent(): Failed to insert event into queue, instance: "
                   << this << std::endl;
        }
    }

    return rv;
}


// shut down dispatch handling
bool Dispatch::Shutdown()
{
    bool rv = false;
    DispatchEvent *pDispEvent = GenEvent();

    if (pDispEvent)
    {
        pDispEvent->OpCode = opc_Shutdown;
        pDispEvent->pEvent = NULL;

        rv = m_EventQueue.Put(pDispEvent, k_DefaultTimeout);

        // clean up if failed to insert instance into queue
        if (!rv)
        {
            delete pDispEvent;
            LogErr << "Dispatch::Shutdown(): Failed to insert event into queue, instance: "
                   << this << std::endl;
        }
    }

    return rv;
}


// set the number of dispatch threads
void Dispatch::NumThreadsSet(uint32_t NumThreads)
{
    m_StackMutex.Lock();

    bool createThreads = (NumThreads > m_Threads.size());
    bool deleteThreads = (NumThreads < m_Threads.size());
    uint32_t threadCount = 0;

    // allow threads to be created if m_EventQueue.Flag1Get() returns true
    if (createThreads && m_EventQueue.Flag1Get())
    {
        threadCount = (NumThreads - m_Threads.size());

        for (uint32_t i = 0; i < threadCount; ++i)
        {
            m_Threads.push_back(new (CP_NEW) Thread("Dispatch Thread", ThreadFunction, this));
        }
    }

    // always allow threads to be deleted
    if (deleteThreads)
    {
        threadCount = (m_Threads.size() - NumThreads);

        for (uint32_t i = 0; i < threadCount; ++i)
        {
            delete m_Threads.back();
            m_Threads.pop_back();
        }
    }

    m_StackMutex.Unlock();
}


// set the pre-dispatch handler
void Dispatch::PreDispatchSet(DispatchHandler_t pHandler, void *pContext)
{
    m_PreDispatch.pHandler = pHandler;
    m_PreDispatch.pContext = pContext;
}


// set the post-dispatch handler
void Dispatch::PostDispatchSet(DispatchHandler_t pHandler, void *pContext)
{
    m_PostDispatch.pHandler = pHandler;
    m_PostDispatch.pContext = pContext;
}


// add an event handler
bool Dispatch::EventHandlerAdd(DispatchHandler_t pHandler, void *pContext)
{
    bool rv = false;
    bool found = false;
    HandlerRecord hdlr;
    HandlerStack_t::iterator i;

    // return if invalid function pointer
    if (pHandler == NULL)
    {
        return false;
    }

    // initialize handler record
    hdlr.pHandler = pHandler;
    hdlr.pContext = pContext;

    m_StackMutex.Lock();

    i = m_Handlers.begin();

    while (i != m_Handlers.end())
    {
        if (i->pHandler == pHandler)
        {
            found = true;
            break;
        }

        ++i;
    }

    if (!found)
    {
        m_Handlers.push_back(hdlr);
        rv = true;
    }

    m_StackMutex.Unlock();

    return rv;
}


// delete an event handler
bool Dispatch::EventHandlerDel(DispatchHandler_t pHandler)
{
    bool rv = false;
    HandlerStack_t::iterator i;

    m_StackMutex.Lock();

    i = m_Handlers.begin();

    while (i != m_Handlers.end())
    {
        if (i->pHandler == pHandler)
        {
            m_Handlers.erase(i);
            rv = true;
            break;
        }

        ++i;
    }

    m_StackMutex.Unlock();

    return rv;
}


// generate a new event
DispatchEvent *Dispatch::GenEvent()
{
    DispatchEvent *pDispatch = new (CP_NEW) DispatchEvent;

    if (pDispatch == NULL)
    {
        LogErr << "Dispatch::GenEvent(): Failed to create a dispatch event object, instance: "
               << this << std::endl;
    }

    return pDispatch;
}


// static thread function
void *Dispatch::ThreadFunction(Thread *pThread)
{
    Dispatch *pDispatch = reinterpret_cast<Dispatch *>(pThread->ContextGet());
    DispatchEvent *pDispEvent = NULL;

    while (pThread->ThreadPoll())
    {
        if (pDispatch->m_EventQueue.Get(reinterpret_cast<void *&>(pDispEvent), k_ReceiveTimeout))
        {
            if (pDispEvent)
            {
                switch (pDispEvent->OpCode)
                {
                case opc_NewEvent:
                    {
                        HandlerStack_t h;
                        HandlerStack_t::iterator i;

                        // run the pre-dispatch handler, if any
                        if (pDispatch->m_PreDispatch.pHandler)
                        {
                            pDispEvent->pContext = pDispatch->m_PreDispatch.pContext;
                            (*pDispatch->m_PreDispatch.pHandler)(pDispEvent);
                        }

                        // get a copy of the handler stack
                        pDispatch->m_StackMutex.Lock();
                        h = pDispatch->m_Handlers;
                        pDispatch->m_StackMutex.Unlock();

                        // check if any handlers were registered
                        if (h.size() > 0)
                        {
                            i = h.begin();

                            while (i != h.end())
                            {
                                // if so, run the handlers
                                pDispEvent->pContext = i->pContext;
                                (*(i->pHandler))(pDispEvent);
                                ++i;
                            }
                        }

                        // run the post-dispatch handler, if any
                        if (pDispatch->m_PostDispatch.pHandler)
                        {
                            pDispEvent->pContext = pDispatch->m_PostDispatch.pContext;
                            (*pDispatch->m_PostDispatch.pHandler)(pDispEvent);
                        }
                    }
                    break;

                case opc_Shutdown:
                    pThread->ExitReq();
                    break;

                case opc_NoOp:
                    // fall through
                default:
                    break;
                }

                // clean up the dispatch event instance
                delete pDispEvent;
                pDispEvent = NULL;
            }
        }
    }

    return NULL;
}

}   // namespace cp
