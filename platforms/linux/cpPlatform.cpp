// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpPlatform.cpp
//
//  Description:    Common Platform Definitions.
//
//  Platform:       linux
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2021-12-16  asc Added ThreadYield_Impl().
//  2021-12-16  asc Replaced syscall(SYS_gettid) with pthread_self().
//  2022-07-22  asc Replaced deprecated pthread_yield() with sched_yield().
// ----------------------------------------------------------------------------

#include <sched.h>

#include "cpPlatform.h"

namespace cp
{

std::ostream &LogMsg = std::cout;
std::ostream &LogErr = std::cerr;

// thread ID implementation
uint32_t ThreadId_Impl()
{
    return static_cast<uint32_t>(pthread_self());
}

// thread yield implementation
uint32_t ThreadYield_Impl()
{
    return static_cast<uint32_t>(sched_yield());
}

}   // namespace cp
