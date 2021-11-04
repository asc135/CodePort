// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpTimer.cpp
//
//  Description:    Timer Facility.
//
//  Platform:       common
//
//  History:
//  2013-01-15  asc Creation.
//  2013-02-26  asc Removed extraneous void * context from timer handler.
// ----------------------------------------------------------------------------

#include "cpPlatform.h"
#include "cpTimer.h"

namespace cp
{

// wait for a timer event
bool Timer::WaitEvent()
{
    return m_SemEvent.Take();
}


// register an event handler function
bool Timer::RegisterHandler(TimerHandler_t pHandler, void *pContext, uint32_t NumThreads)
{
    bool rv = false;

    if (m_Valid)
    {
        if (m_PtrDispatch == NULL)
        {
            m_PtrDispatch = new (CP_NEW) Dispatch(NumThreads);

            if (m_PtrDispatch)
            {
                // register the timer dispatch function, which in turn calls the user registered handler
                m_PtrDispatch->EventHandlerAdd(TimerEventDispatch, this);
            }
            else
            {
                LogErr << "Timer::RegisterHandler(): Failed to create a dispatch object: "
                       << NameGet() << std::endl;
            }
        }

        // store the user registered handler pointer and context
        if (m_PtrDispatch)
        {
            m_PtrContext = pContext;
            m_PtrHandler = pHandler;

            rv = true;
        }
    }

    return rv;
}


// signal a timer event
void Timer::SignalEvent()
{
    // perform any implementation specific actions
    LocalEventHook();

    // submit an event to the dispatch object
    if (m_PtrDispatch)
    {
        m_PtrDispatch->SubmitEvent();
    }

    // release anyone waiting on an event
    m_SemEvent.Give();
}


// dispatch function runs from the context of a dedicated dispatch thread
void *Timer::TimerEventDispatch(DispatchEvent *pDispEvent)
{
    // recover the timer instance pointer
    Timer *pTimer = reinterpret_cast<Timer *>(pDispEvent->pContext);

    if (pTimer != NULL)
    {
        // call the user registered handler, if defined
        if (pTimer->m_PtrHandler)
        {
            (*pTimer->m_PtrHandler)(pTimer);
        }
    }

    return pDispEvent;
}

}   // namespace cp
