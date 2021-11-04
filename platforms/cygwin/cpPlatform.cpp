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
//  Platform:       cygwin
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-10-22  asc Switched thread id function from posix to windows.
// ----------------------------------------------------------------------------

#include <windows.h>

#include "cpPlatform.h"

namespace cp
{

std::ostream &LogMsg = std::cout;
std::ostream &LogErr = std::cerr;

// thread ID implementation
uint32_t ThreadId_Impl()
{
    return GetCurrentThreadId();
}

}   // namespace cp
