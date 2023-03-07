// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpThread_I.cpp
//
//  Description:    Thread Facility.
//
//  Platform:       posix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-09-28  asc Replaced cp::Sem with cp::SemLite.
//  2013-01-09  asc Added default thread function.
//  2013-01-09  asc Moved state mutex and suspend semaphore into main class.
//  2013-01-09  asc Eliminated Terminate() method and st_Terminated state.
//  2013-02-15  asc Added exit callback function to provide "join" type capability.
//  2013-02-15  asc Moved thread state into common implementation.
//  2013-02-21  asc Moved Trampoline() into an embedded class to make InvokeUserFunc() private.
//  2022-05-25  asc Added support for setting the stack size via the constructor parameter.
//  2022-05-25  asc Added a minimum thread stack size due to AIX's default 92KB stack size.
//  2023-03-06  asc Added ability to abort the thread.
// ----------------------------------------------------------------------------

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
    static void *Trampoline(void *pContext)
    {
        void *rv = NULL;

        if (pContext)
        {
            // transfer control to the user supplied function
            rv = reinterpret_cast<Thread *>(pContext)->InvokeUserFunc();
        }

        return rv;
    }
};


// start thread execution
bool Thread::ThreadStart(uint8_t Priority, size_t StackSize)
{
    bool rv = (m_PtrFunc != NULL);
    pthread_attr_t attr;

    // initialize attributes
    if (rv)
    {
        rv = (pthread_attr_init(&attr) == 0);
    }

    // not currently used in this implementation
    (void)Priority;

    // set the requested stack size
    if (rv)
    {
        // minimum stack size is 2MB
        size_t minStackSize = 2 * 1024 * 1024;
        size_t stackSize = (StackSize > minStackSize) ? StackSize : minStackSize;
        size_t defaultSize = 0;

        // get the default stack size from pthreads
        if (pthread_attr_getstacksize(&attr, &defaultSize) == 0)
        {
            // if default is too small, set it to the minimum or the requested size
            if (defaultSize < stackSize)
            {
                // posix defines this system-wide with ulimit
                rv = (pthread_attr_setstacksize(&attr, stackSize) == 0);
            }
        }
    }

    if (rv)
    {
        // create a non-realtime posix thread
        rv = (pthread_create(&m_Thread, &attr, Thread::NativeThreadFunc::Trampoline, this) == 0);

        // destroy the attributes structure
        pthread_attr_destroy(&attr);
    }

    // detach thread to free resources at thread termination
    if (rv)
    {
        rv = (pthread_detach(m_Thread) == 0);
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


// enable or disable abortable state
void Thread::Abortable(bool Enable)
{
    int type = Enable ? PTHREAD_CANCEL_ASYNCHRONOUS : PTHREAD_CANCEL_DEFERRED;
    int oldtype = 0;
    m_Abortable = Enable;
    pthread_setcanceltype(type, &oldtype);
}


// abort execution if thread was set to abortable
void Thread::Abort()
{
    if (m_Abortable)
    {
        pthread_cancel(m_Thread);
        m_RunState = state_Error;       // prevents Thread instance destructor from getting blocked
    }
}

}   // namespace cp
