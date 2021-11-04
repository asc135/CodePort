// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpPlatform.cpp
//
//  Description:    Common Platform Definitions.
//
//  Platform:       mswin
//
//  History:
//  2012-12-10  asc Creation.
//  2013-04-03  asc Added reference counted winsock init/cleanup.
// ----------------------------------------------------------------------------

#include "cpMutex.h"

namespace cp
{

std::ostream &LogMsg = std::cout;
std::ostream &LogErr = std::cerr;
uint32_t k_IoPendingInterval = 100;     // milliseconds
uint32_t g_WinSockRefCount = 0;
Mutex g_WinSockInitMutex("Winsock Init Mutex");

// thread ID implementation
uint32_t ThreadId_Impl()
{
    return GetCurrentThreadId();
}

// print native Windows error message from GetLastError()
void PrintGetLastError(DWORD LastError)
{
    LPVOID lpMsgBuf;

    FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    LastError,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR) &lpMsgBuf,
    0, NULL );

    LogErr << (char *)lpMsgBuf << std::endl;

    LocalFree(lpMsgBuf);
}


bool WinSockInit()
{
    WSADATA wsaData;
    int result = 0;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (result != 0)
    {
        LogErr << "cp::WinSockInit(): Winsock initialization failed." << std::endl;
    }

    return (result == 0);
}


bool WinSockCleanup()
{
    int result = 0;

    // Initialize Winsock
    result = WSACleanup();

    if (result != 0)
    {
        LogErr << "cp::WinSockCleanup(): Winsock cleanup failed." << std::endl;
    }

    return (result == 0);
}


bool WinSockGet()
{
    bool rv = true;

    g_WinSockInitMutex.Lock();

    // check if this is the first request to use winsock
    if (g_WinSockRefCount == 0)
    {
        // initialize the winsock library
        rv = WinSockInit();
    }

    // increment the reference counter
    ++g_WinSockRefCount;

    g_WinSockInitMutex.Unlock();

    return rv;
}


bool WinSockPut()
{
    bool rv = true;

    g_WinSockInitMutex.Lock();

    // decrement the reference counter
    if (g_WinSockRefCount)
    {
        --g_WinSockRefCount;

        // shut down the winsock library
        if (g_WinSockRefCount == 0)
        {
            rv = WinSockCleanup();
        }
    }

    g_WinSockInitMutex.Unlock();

    return rv;
}

}   // namespace cp
