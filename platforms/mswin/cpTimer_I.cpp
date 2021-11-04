// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpTimer_I.cpp
//
//  Description:    Timer Facility.
//
//  Platform:       mswin
//
//  History:
//  2013-01-16  asc Creation.
//  2013-03-25  asc Moved TimerCallback() into an embedded class to make SignalEvent() private.
// ----------------------------------------------------------------------------

#include <map>

#define _WIN32_WINNT 0x0500

#include "cpPlatform.h"
#include "cpThread.h"
#include "cpTimer.h"

#include <winbase.h>

namespace cp
{

// embedded class
class Timer::NativeTimerFunc
{
public:
    // timer callback function runs in the context of an OS thread
    static VOID CALLBACK TimerCallback(PVOID pContext, BOOLEAN TimerOrWaitFired)
    {
        (void)TimerOrWaitFired;

        Timer *pTimer = reinterpret_cast<Timer *>(pContext);

        if (pTimer != NULL)
        {
            // signal the occurrence of an event
            pTimer->SignalEvent();
        }
        else
        {
            LogErr << "Timer::TimerCallback():  Failed to obtain pointer to timer instance."
                   << std::endl;
        }
    }
};


// constructor
Timer::Timer(String const &Name, uint8_t Mode, uint8_t Scale, uint64_t Period) :
    Base(Name),
    m_Running(false),
    m_Spare(false),
    m_Mode(Mode),
    m_Scale(Scale),
    m_Period(Period),
    m_SemEvent("Timer Semaphore", 0, 1),
    m_PtrHandler(NULL),
    m_PtrContext(NULL),
    m_PtrDispatch(NULL)
{
    m_Timer.hTimerQueue = CreateTimerQueue(); 
    m_Timer.hTimer = INVALID_HANDLE_VALUE;

    if (m_Timer.hTimerQueue != INVALID_HANDLE_VALUE)
    {
        m_Valid = true;
    }
    else
    {
        LogErr << "Timer::Timer(): Failed to create timer queue: "
               << NameGet() << std::endl;
    }
}


// destructor
Timer::~Timer()
{
    // delete any and all timers
    if (m_Valid)
    {
        DeleteTimerQueueEx(m_Timer.hTimerQueue, INVALID_HANDLE_VALUE);
    }

    if (m_PtrDispatch)
    {
        delete m_PtrDispatch;
    }
}


// start the timer
bool Timer::Start()
{
    bool rv = true;
    uint32_t millisecs = 0;
    uint32_t period = 0;

    // if timer is currently running, stop it first
    if (m_Running)
    {
        Stop();
    }

    // convert period to milliseconds
    switch (m_Scale)
    {
    case Hour:
        millisecs = m_Period * 60 * 60 * 1000;
        break;

    case Min:
        millisecs = m_Period * 60 * 1000;
        break;

    case Sec:
        millisecs = m_Period * 1000;
        break;

    case Milli:
        millisecs = m_Period;
        break;

    case Micro:
        millisecs = m_Period / 1000;
        break;

    case Nano:
        millisecs = m_Period / 1000 / 1000;
        break;

    case Pico:
        millisecs = m_Period / 1000 / 1000 / 1000;
        break;

    case Femto:
        millisecs = m_Period / 1000 / 1000 / 1000 / 1000;
        break;

    default:
        LogErr << "Timer::Start(): Invalid scale value: "
               << NameGet() << std::endl;
        millisecs = 0;
        rv = false;
        break;
    }

    // set the repeat period
    if (m_Mode == Periodic)
    {
        period = millisecs;
    }

    // start the timer
    if (rv)
    {
        rv = CreateTimerQueueTimer(&m_Timer.hTimer,
                                   m_Timer.hTimerQueue,
                                   Timer::NativeTimerFunc::TimerCallback,
                                   this,
                                   millisecs,
                                   period,
                                   WT_EXECUTEDEFAULT);

        if (rv)
        {
            m_Running = true; 
        }
        else
        {
            LogErr << "Timer::Start(): Failed to create timer: "
                   << NameGet() << std::endl;
        }
    }

    return rv;
}


// stop the timer
bool Timer::Stop()
{
    bool rv = true;

    if (m_Timer.hTimer != INVALID_HANDLE_VALUE)
    {
        rv = DeleteTimerQueueTimer(m_Timer.hTimerQueue,
                                   m_Timer.hTimer,
                                   INVALID_HANDLE_VALUE);
    }

    if (rv)
    {
        m_Timer.hTimer = INVALID_HANDLE_VALUE;
        m_Running = false;
    }
    else
    {
        LogErr << "Timer::Stop(): Failed to delete timer: "
               << NameGet() << std::endl;
    }

    return rv;
}


// perform any implementation specific actions
void Timer::LocalEventHook()
{
}

}   // namespace cp
