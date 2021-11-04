// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpTimer_I.h
//
//  Description:    Timer Facility.
//
//  Platform:       mswin
//
//  History:
//  2013-01-16  asc Creation.
// ----------------------------------------------------------------------------

#ifndef CP_TIMER_I_H
#define CP_TIMER_I_H

namespace cp
{

// timer callback function runs in the context of an OS thread
VOID CALLBACK TimerCallback(PVOID pContext, BOOLEAN TimerOrWaitFired);

struct Timer_t
{
    HANDLE              hTimerQueue;                        // handle to the timer queue
    HANDLE              hTimer;                             // handle to the timer
};

}   // namespace cp

#endif  // CP_TIMER_I_H
