// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpTimer_I.h
//
//  Description:    Timer Facility.
//
//  Platform:       posix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-01-14  asc Switched to using callback instead of signals due to cygwin bugs.
// ----------------------------------------------------------------------------

#ifndef CP_TIMER_I_H
#define CP_TIMER_I_H

#include <csignal>
#include <ctime>

namespace cp
{

// timer callback function runs from the context of an OS thread
void TimerCallback(union sigval Value);

struct Timer_t
{
    timer_t             timerid;
    sigevent            sev;
    struct sigaction    sa;
    sigset_t            mask;
    itimerspec          period;
};

}   // namespace cp

#endif  // CP_TIMER_I_H
