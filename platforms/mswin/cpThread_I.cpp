// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpThread_I.cpp
//
//  Description:    Thread Facility.
//
//  Platform:       mswin
//
//  History:
//  2012-12-11  asc Creation.
//  2013-01-09  asc Added default thread function.
//  2013-01-09  asc Moved state mutex and suspend semaphore into main class.
//  2013-01-09  asc Eliminated Terminate() method and st_Terminated state.
//  2013-02-15  asc Added exit callback function to provide "join" type capability.
//  2013-02-15  asc Moved thread state into common implementation.
//  2013-02-21  asc Moved Trampoline() into an embedded class to make InvokeUserFunc() private.
// ----------------------------------------------------------------------------

#include <process.h>

#include "cpThread.h"

namespace cp
{

// embedded class
class Thread::NativeThreadFunc
{
public:
    // ----------------------------------------------------------------------------
    //  Function Name:  Trampoline
    //
    //  Description:    This function is used as the native thread function.
    //                  It calls the user supplied portable function.
    //
    //  Inputs:         pContext - pointer to thread object
    //
    //  Outputs:        none
    //
    //  Returns:        none
    // ----------------------------------------------------------------------------
    static unsigned __stdcall Trampoline(void *pContext)
    {
        void *rv = NULL;

        if (pContext)
        {
            // transfer control to the user supplied function
            rv = reinterpret_cast<Thread *>(pContext)->InvokeUserFunc();
        }

        return reinterpret_cast<unsigned>(rv);
    }
};


// start thread execution
bool Thread::ThreadStart(uint8_t Priority, size_t StackSize)
{
    bool rv = (m_PtrFunc != NULL);

    // not currently used in this implementation
    (void)Priority;

    // posix defines this system-wide with ulimit
    (void)StackSize;

    if (rv)
    {
        // create a non-realtime thread
        m_Thread.m_Handle = (HANDLE)_beginthreadex(NULL, 0,
                                                   Thread::NativeThreadFunc::Trampoline, this,
                                                   (true ? 0 : CREATE_SUSPENDED),
                                                   &m_Thread.m_ThreadId);

        // test for success (note _beginthreadex returns 0 on error)
        rv = m_Thread.m_Handle != 0;
    }

    return rv;
}


void Thread::CleanUp()
{

}


uint8_t Thread::PriorityGet() const
{
    // not currently used in this implementation
    return k_DefaultThreadPriority;
}


void Thread::PrioritySet(uint8_t Priority)
{
    // not currently used in this implementation
    (void)Priority;
}

}   // namespace cp
