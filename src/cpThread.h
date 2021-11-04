// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpThread.h
//
//  Description:    Thread Facility.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-01-09  asc Made the destructor virtual and added default thread function.
//  2013-01-09  asc Moved state mutex and suspend semaphore into main class.
//  2013-01-09  asc Eliminated Terminate() method and st_Terminated state.
//  2013-02-15  asc Added exit callback function to provide "join" type capability.
//  2013-02-15  asc Created a common implementation for non-platform-specific code.
//  2013-02-21  asc Added embedded class to make InvokeUserFunc() private.
//  2013-02-06  asc Added startup options to specify run mode and exit sync.
//  2013-04-17  asc Added accessor method to return state of exit flag.
// ----------------------------------------------------------------------------

#ifndef CP_THREAD_H
#define CP_THREAD_H

#include "cpMutex.h"
#include "cpSemLite.h"
#include "cpThread_I.h"

namespace cp
{

class Thread;

typedef void *(*ThreadFuncPtr_t)(Thread *);

// ----------------------------------------------------------------------------

class Thread : public Base
{
public:
    // embedded class
    class NativeThreadFunc;

    // enumerations
    enum RunState { state_Error, state_Running, state_Suspended };

    enum Options  { opt_Running    = 0,
                    opt_Suspended  = 1,
                    opt_NoExitSync = 2 };

    // constructor
    Thread(String const &Name,
           ThreadFuncPtr_t pFunction,
           void       *pContext  = NULL,
           uint8_t     Flags     = 0,
           uint8_t     Selector  = 0,
           uint8_t     Priority  = k_DefaultThreadPriority,
           size_t      StackSize = k_DefaultThreadStack);

    // destructor
    ~Thread();

    // accessors
    RunState StateGet() const;                              // get execution state
    uint8_t PriorityGet() const;                            // get execution priority
    uint8_t SelectorGet() const { return m_Selector; }      // get the user context pointer
    void *ContextGet() const { return m_PtrContext; }       // get the user context pointer
    bool ExitFlag() const { return m_ExitFlag; }            // return state of exit flag

    // manipulators
    void PrioritySet(uint8_t Priority);                     // set execution priority
    void Resume();                                          // resume execution
    void Suspend();                                         // suspend execution
    void ExitReq();                                         // request the thread to exit
    bool WaitExit(uint32_t Timeout);                        // wait for thread to exit

    // methods for thread function only
    bool ThreadPoll();                                      // polls state and returns true while running
    void ThreadExit();                                      // called by thread itself to schedule exit

private:
    // copy constructor (disabled)
    Thread(Thread const &rhs);

    // assignment operator (disabled)
    Thread &operator=(Thread const &rhs);

    void *InvokeUserFunc();                                 // calls the user thread function
    bool ThreadStart(uint8_t Priority, size_t StackSize);   // start thread execution
    void CleanUp();                                         // deallocate resources

    uint8_t             m_RunState;                         // thread run state
    uint8_t             m_Selector;                         // application defined method selector
    bool                m_ExitSync;                         // synchronize thread exit with destructor
    bool                m_ExitFlag;                         // thread exit control flag
    Thread_t            m_Thread;                           // native thread data storage
    ThreadFuncPtr_t     m_PtrFunc;                          // pointer to the user thread function or object
    void               *m_PtrContext;                       // pointer to the user thread context
    Mutex               m_MtxState;                         // public interface synchronization mutex
    SemLite             m_SemSuspend;                       // execution suspend semaphore
    SemLite             m_SemExit;                          // maintains environment until thread exits
};

}   // namespace cp

#endif  // CP_THREAD_H
