// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpTimer_I.cpp
//
//  Description:    Timer Facility.
//
//  Platform:       posix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-01-09  asc Switched to using cp::Dispatch mechanism for event output.
//  2013-01-14  asc Switched to using callback instead of signals due to cygwin bugs.
//  2013-03-25  asc Moved TimerCallback() into an embedded class to make SignalEvent() private.
// ----------------------------------------------------------------------------

#include "cpPlatform.h"
#include "cpTimer.h"

namespace cp
{

// embedded class
class Timer::NativeTimerFunc
{
public:
    // timer callback function runs from the context of an OS thread
    static void TimerCallback(union sigval Value)
    {
        // recover pointer to the timer instance
        Timer *pTimer = reinterpret_cast<Timer *>(Value.sival_ptr);

        if (pTimer != NULL)
        {
            // signal the occurrence of an event
            pTimer->SignalEvent();
        }
        else
        {
            LogErr << "Timer::TimerCallback(): Failed to obtain pointer to timer instance."
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
    bool success = true;

    if (success)
    {
        // set up timer notification structure
        m_Timer.sev.sigev_notify = SIGEV_THREAD;
        m_Timer.sev.sigev_notify_function = Timer::NativeTimerFunc::TimerCallback;
        m_Timer.sev.sigev_notify_attributes = NULL;
        m_Timer.sev.sigev_value.sival_ptr = this;

        // create a posix timer
        if (timer_create(CP_POSIX_CLOCK, &m_Timer.sev, &m_Timer.timerid) == k_Error)
        {
            success = false;
            LogErr << "Timer::Timer(): Error calling timer_create(): "
                   << NameGet() << std::endl;
        }
    }

    // set the validity flag to initialization outcome
    m_Valid = success;
}


// destructor
Timer::~Timer()
{
    if (m_Valid)
    {
        // disarm and delete the posix timer
        timer_delete(m_Timer.timerid);
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
    uint32_t secs = 0;

    // convert period to time structure
    switch (m_Scale)
    {
    case Hour:
        m_Timer.period.it_value.tv_sec = m_Period * 3600;
        m_Timer.period.it_value.tv_nsec = 0;
        break;

    case Min:
        m_Timer.period.it_value.tv_sec = m_Period * 60;
        m_Timer.period.it_value.tv_nsec = 0;
        break;

    case Sec:
        m_Timer.period.it_value.tv_sec = m_Period;
        m_Timer.period.it_value.tv_nsec = 0;
        break;

    case Milli:
        secs = m_Period / 1000;
        m_Timer.period.it_value.tv_sec = secs;
        m_Timer.period.it_value.tv_nsec = (m_Period - (secs * 1000)) * 1000000;
        break;

    case Micro:
        secs = m_Period / 1000000;
        m_Timer.period.it_value.tv_sec = secs;
        m_Timer.period.it_value.tv_nsec = (m_Period - (secs * 1000000)) * 1000;
        break;

    case Nano:
        secs = m_Period / 1000000000;
        m_Timer.period.it_value.tv_sec = secs;
        m_Timer.period.it_value.tv_nsec = (m_Period - (secs * 1000000000));
        break;

    case Pico:
        secs = m_Period / 1000000 / 1000000;
        m_Timer.period.it_value.tv_sec = secs;
        m_Timer.period.it_value.tv_nsec = (m_Period - (secs * 1000000 * 1000000)) / 1000;
        break;

    case Femto:
        secs = m_Period / 1000000 / 1000000 / 1000;
        m_Timer.period.it_value.tv_sec = secs;
        m_Timer.period.it_value.tv_nsec = (m_Period - (secs * 1000000 * 1000000 * 1000)) / 1000000;
        break;

    default:
        LogErr << "Timer::Start(): Invalid scale value: "
               << NameGet() << std::endl;
        m_Timer.period.it_value.tv_sec = 0;
        m_Timer.period.it_value.tv_nsec = 0;
        break;
    }

    if (m_Mode == Delay)
    {
        // no reload
        m_Timer.period.it_interval.tv_sec = 0;
        m_Timer.period.it_interval.tv_nsec = 0;
    }
    else
    {
        // reload and repeat
        m_Timer.period.it_interval.tv_sec = m_Timer.period.it_value.tv_sec;
        m_Timer.period.it_interval.tv_nsec = m_Timer.period.it_value.tv_nsec;
    }

    // arm the timer
    if (timer_settime(m_Timer.timerid, 0, &m_Timer.period, NULL) == k_Error)
    {
        rv = false;
        LogErr << "Timer::Start(): Error calling timer_settime(): "
               << NameGet() << std::endl;
    }

    m_Running = rv;

    return rv;
}


// stop the timer
bool Timer::Stop()
{
    bool rv = true;

    m_Timer.period.it_value.tv_sec = 0;
    m_Timer.period.it_value.tv_nsec = 0;

    // disarm the timer
    if (timer_settime(m_Timer.timerid, 0, &m_Timer.period, NULL) == k_Error)
    {
        rv = false;
        LogErr << "Timer::Stop(): Error calling timer_settime(): "
               << NameGet() << std::endl;
    }
    else
    {
        m_Running = false;
    }

    return rv;
}


// perform any implementation specific actions
void Timer::LocalEventHook()
{
}

}   // namespace cp
