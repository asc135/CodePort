// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpPlatform.h
//
//  Description:    Common Platform Header.
//
//  Platform:       aix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-11-15  asc Added cstdlib and csignal headers.
//  2021-12-16  asc Added ThreadYield_Impl().
//  2022-04-07  asc Added vasprintf.h include.
//  2022-05-23  asc Added netdb.h include.
//  2023-04-04  asc Added desc_t definition.
// ----------------------------------------------------------------------------

#ifndef CP_PLATFORM_H
#define CP_PLATFORM_H

// ----------------------------------------------------------------------------

#include <unistd.h>
//#include <fcntl.h>
//#include <termios.h>
//#include <sys/time.h>
//#include <sys/ioctl.h>
//#include <sys/types.h>
//#include <sys/select.h>
//#include <sys/signal.h>
//#include <sys/syscall.h>
//#include <netinet/in.h>
#include <netdb.h>

// ----------------------------------------------------------------------------

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include <iomanip>
#include "cpTypes.h"
#include "cpConstants.h"
#include "vasprintf.h"

// ----------------------------------------------------------------------------

#define CP_NEW std::nothrow
#define CP_POSIX_CLOCK CLOCK_MONOTONIC

// ----------------------------------------------------------------------------

namespace cp
{

extern std::ostream &LogMsg;
extern std::ostream &LogErr;

// file descriptor for this platform
typedef int desc_t;

uint32_t ThreadId_Impl();
uint32_t ThreadYield_Impl();

}   // namespace cp

#endif  // CP_PLATFORM_H