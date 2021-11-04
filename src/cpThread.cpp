// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpThread.cpp
//
//  Description:    Thread Facility.
//
//  Platform:       common
//
//  History:
//  2013-02-15  asc Creation.
//  2013-02-06  asc Added startup options to specify run mode and exit sync.
//  2013-04-17  asc Added protection from thread deleting thread object.
// ----------------------------------------------------------------------------

#include "cpMutex.h"
#include "cpSemLite.h"
#include "cpThread.h"

namespace cp
{

// constructor
Thread::Thread(String const &Name, ThreadFuncPtr_t pFunction, void *pContext,
               uint8_t Flags, uint8_t Selector, uint8_t Priority, size_t StackSize) :
    Base(Name),
    m_Selector(Selector),
    m_ExitSync(false),
    m_ExitFlag(false),
    m_PtrFunc(pFunction),
    m_PtrContext(pContext),
    m_MtxState("Thread State Mutex"),
    m_SemSuspend("Thread Suspend Semaphore", 0, 1),
    m_SemExit("Thread Exit Semaphore", 0, 2)
{
    // set the initial run state
    m_RunState = (Flags & opt_Suspended) ? state_Suspended : state_Running;

    // set the exit sync mode
    m_ExitSync = (Flags & opt_NoExitSync) ? false : true;

    // start the thread
    m_Valid = ThreadStart(Priority, StackSize);

    if (!m_Valid)
    {
        m_RunState = state_Error;
        LogErr << "Thread::Thread(): Failed to start thread: "
               << NameGet() << std::endl;
        CleanUp();
    }
}


// destructor
Thread::~Thread()
{
    // stop thread if it was running
    if (m_RunState != state_Error)
    {
        // inform the thread to exit
        ExitReq();

        if (m_ExitSync)
        {
            // wait for thread to exit
            m_SemExit.Take();
        }
    }

    // free resources
    CleanUp();
}


void Thread::Resume()
{
    if (m_Valid)
    {
        m_MtxState.Lock();

        // immediately set the state to running and give the semaphore that unblocks the thread
        if (m_RunState == state_Suspended)
        {
            m_RunState = state_Running;
            m_SemSuspend.Give();
        }

        m_MtxState.Unlock();
    }
}


void Thread::Suspend()
{
    if (m_Valid)
    {
        m_MtxState.Lock();

        // schedule a suspend at the next call to ThreadPoll
        if (m_RunState == state_Running)
        {
            m_RunState = state_Suspended;
            m_SemSuspend.TryTake();
        }

        m_MtxState.Unlock();
    }
}


// request the thread to exit
void Thread::ExitReq()
{
    if (m_Valid)
    {
        m_MtxState.Lock();

        // signal thread exit
        m_ExitFlag = true;

        // resume if suspended
        if (m_RunState == state_Suspended)
        {
            m_RunState = state_Running;
            m_SemSuspend.Give();
        }

        m_MtxState.Unlock();
    }
}


// wait for thread to exit
bool Thread::WaitExit(uint32_t Timeout)
{
    bool rv = true;

    if (m_ExitSync)
    {
        rv = m_SemExit.Take(Timeout);
    }

    return rv;
}


// calls the user thread function
void *Thread::InvokeUserFunc()
{
    bool exitSync = m_ExitSync;
    void *retVal = NULL;

    // test for suspended start condition
    if (ThreadPoll())
    {
        // check if a function pointer was defined
        if (m_PtrFunc != NULL)
        {
            // call the supplied function pointer
            retVal = (*m_PtrFunc)(this);
        }
    }

    // don't access m_ExitSync directly here because thread
    // object might already be out of scope at this point
    if (exitSync)
    {
        // signal that the thread will not reference object resources any longer
        // release destructor and up to one waiting client
        m_SemExit.Give();
        m_SemExit.Give();
    }

    return retVal;
}


Thread::RunState Thread::StateGet() const
{
    return static_cast<Thread::RunState>(m_RunState);
}


// polls state and returns true while running
bool Thread::ThreadPoll()
{
    uint32_t state = m_RunState;

    switch (state)
    {
    case state_Suspended:
        // block here...
        m_SemSuspend.Take();
        break;

    default:
        break;
    }

    return !m_ExitFlag;
}


// called by thread itself to schedule exit
void Thread::ThreadExit()
{
    m_ExitFlag = true;
}

}   // namespace cp
