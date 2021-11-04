// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpPlatform.h
//
//  Description:    Common Platform Definitions.
//
//  Platform:       mswin
//
//  History:
//  2012-12-10  asc Creation.
//  2013-04-03  asc Added reference counted winsock init/cleanup.
//  2013-11-15  asc Added cstdlib and csignal headers.
// ----------------------------------------------------------------------------

#ifndef CP_PLATFORM_H
#define CP_PLATFORM_H

// ----------------------------------------------------------------------------

//#include <unistd.h>
//#include <fcntl.h>
//#include <termios.h>
//#include <sys/time.h>
//#include <sys/ioctl.h>
//#include <sys/types.h>
//#include <sys/select.h>
//#include <sys/signal.h>
//#include <sys/syscall.h>
//#include <netinet/in.h>

#include <windows.h>

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

// ----------------------------------------------------------------------------

#define CP_NEW std::nothrow
//#define CP_POSIX_CLOCK CLOCK_MONOTONIC

// ----------------------------------------------------------------------------

namespace cp
{

extern std::ostream &LogMsg;
extern std::ostream &LogErr;
extern uint32_t k_IoPendingInterval;

uint32_t ThreadId_Impl();
void PrintGetLastError(DWORD LastError);
bool WinSockGet();
bool WinSockPut();

}   // namespace cp

#endif  // CP_PLATFORM_H
