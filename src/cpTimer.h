// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpTimer.h
//
//  Description:    Timer Facility.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-01-09  asc Switched to using cp::Dispatch mechanism for event output.
//  2013-01-15  asc Changed default period from 0ms to 1000ms.
//  2013-02-26  asc Removed extraneous void * context from timer handler.
//  2013-03-25  asc Added embedded class to make SignalEvent() private.
// ----------------------------------------------------------------------------

#ifndef CP_TIMER_H
#define CP_TIMER_H

#include "cpDispatch.h"
#include "cpTimer_I.h"

namespace cp
{

class Timer;

typedef void *(*TimerHandler_t)(Timer *pTimer);

// ----------------------------------------------------------------------------

class Timer : public Base
{
public:
    // embedded class
    class NativeTimerFunc;

    // local enumerations
    enum Mode  { Delay, Periodic };
    enum Scale { Hour, Min, Sec, Milli, Micro, Nano, Pico, Femto };

    Timer(String const &Name,
          uint8_t Mode,
          uint8_t Scale,
          uint64_t Period);

    virtual ~Timer();

    // accessors
    bool IsRunning()      const { return m_Running;      }  // get timer running state
    Mode ModeGet()        const { return (Mode)m_Mode;   }  // get timer mode
    Scale ScaleGet()      const { return (Scale)m_Scale; }  // get timer scale
    uint64_t PeriodGet()  const { return m_Period;       }  // get timer period
    void *ContextGet()    const { return m_PtrContext;   }  // get the user context pointer
    bool WaitEvent();                                       // wait for a timer event

    // manipulators
    bool Start();                                           // start the timer
    bool Stop();                                            // stop the timer
    void ModeSet(uint8_t Mode)      { m_Mode = Mode; }      // set timer mode
    void ScaleSet(uint8_t Scale)    { m_Scale = Scale; }    // set timer scale
    void PeriodSet(uint64_t Period) { m_Period = Period; }  // set timer period
    bool RegisterHandler(TimerHandler_t pHandler,
                         void *pContext,
                         uint32_t NumThreads = 1);          // register an event handler function

private:
    void SignalEvent();                                     // signal a timer event
    void LocalEventHook();                                  // perform any implementation specific actions
    static void *TimerEventDispatch(DispatchEvent *pDispEvent);

    bool                m_Running;                          // true when running
    bool                m_Spare;                            // spare variable for long word alignment
    uint8_t             m_Mode;                             // mode of timer operation
    uint8_t             m_Scale;                            // time scale for timer period
    uint64_t            m_Period;                           // current timer period
    SemLite             m_SemEvent;                         // synchronization semaphore
    Timer_t             m_Timer;                            // native data storage
    TimerHandler_t      m_PtrHandler;                       // timer handler callback function
    void               *m_PtrContext;                       // timer handler callback context
    Dispatch           *m_PtrDispatch;                      // pointer to dispatch object
};

}   // namespace cp

#endif  // CP_TIMER_H
