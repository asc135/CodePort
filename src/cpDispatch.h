// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpDispatch.h
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

#ifndef CP_DISPATCH_H
#define CP_DISPATCH_H

#include "cpThread.h"
#include "cpItcQueue.h"
#include "cpPooledBase.h"

namespace cp
{

// forward references
class Dispatch;

// ----------------------------------------------------------------------------

// local types and definitions

class DispatchEvent : public PooledBase
{
public:
    // constructor
    DispatchEvent() :
        OpCode(0),
        pEvent(NULL),
        pContext(NULL)
    {}

    // destructor
    virtual ~DispatchEvent() {}

    uint32_t            OpCode;                             // the operation code
    void               *pEvent;                             // the event
    void               *pContext;                           // event context
};

// ----------------------------------------------------------------------------

typedef void *(*DispatchHandler_t)(DispatchEvent *pDispEvent);

// ----------------------------------------------------------------------------

class HandlerRecord
{
public:
    HandlerRecord() :
        pHandler(NULL),
        pContext(NULL)
    { }

    DispatchHandler_t   pHandler;
    void               *pContext;
};

// ----------------------------------------------------------------------------

typedef std::vector<HandlerRecord, Alloc<HandlerRecord> > HandlerStack_t;
typedef std::vector<Thread *, Alloc<Thread *> > ThreadStack_t;

// ----------------------------------------------------------------------------

class Dispatch : public PooledBase
{
public:
    // local enumerations
    enum Constants { k_MaxEvents = 64 };
    enum OpCodes { opc_NoOp = 0, opc_NewEvent, opc_Shutdown };

    // constructor
    Dispatch(uint32_t NumThreads = 1, uint32_t EventQueueDepth = k_MaxEvents);

    // destructor
    virtual ~Dispatch();

    // accessors
    uint32_t NumThreadsGet() { return m_Threads.size(); }   // return number of dispatch threads

    // manipulators
    bool SubmitEvent(void *pEvent = NULL, uint32_t Timeout = k_InfiniteTimeout);
    bool Shutdown();
    void NumThreadsSet(uint32_t NumThreads);                // set the number of dispatch threads

    void PreDispatchSet(DispatchHandler_t pHandler,
                        void *pContext);                    // set the pre-dispatch handler

    void PostDispatchSet(DispatchHandler_t pHandler,
                         void *pContext);                   // set the post-dispatch handler

    bool EventHandlerAdd(DispatchHandler_t pHandler,
                         void *pContext);                   // add an event handler

    bool EventHandlerDel(DispatchHandler_t pHandler);       // delete an event handler

    static void *ThreadFunction(Thread *pThread);           // static thread function

protected:
    virtual DispatchEvent *GenEvent();                      // generate a new event

    Mutex               m_StackMutex;                       // mutex to protect the stacks
    ItcQueue            m_EventQueue;                       // the event input queue
    HandlerRecord       m_PreDispatch;                      // function called before dispatch stack
    HandlerRecord       m_PostDispatch;                     // function called after dispatch stack
    HandlerStack_t      m_Handlers;                         // stack of user defined event handler function pointers
    ThreadStack_t       m_Threads;                          // stack of threads to execute event handlers

private:
    // copy constructor
    Dispatch(Dispatch const &rhs);                          // disabled copy constructor

    // assignment operator
    Dispatch &operator=(Dispatch const &rhs);               // disabled assignment operator
};

}   // namespace cp

#endif  // CP_DISPATCH_H
