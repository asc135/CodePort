// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpUtil_I.cpp
//
//  Description:    Utility Function Library.
//
//  Platform:       mswin
//
//  History:
//  2012-12-10  asc Creation.
//  2012-03-16  asc Added Ipv4ToStr() and StrToIpv4() functions.
//  2013-06-13  asc Added StartProcess() function.
//  2013-12-18  asc Added DirCreate() and moved PathCreate() into common module.
// ----------------------------------------------------------------------------

#include <cstdlib>      // exit()
#include <ws2tcpip.h>
#include <direct.h>     // _mkdir()

#include "cpUtil.h"
#include "cpStreamBuf.h"
#include "cpBuffer.h"

namespace cp
{

// module local function used by the public delay calls
static bool NativeSleep(uint32_t Delay)
{
    ::Sleep(Delay);
    return true;
}


// get the platform path separator
String const PathSep()
{
    return k_PathSeparator;
}


// get the current temp directory path
String const TempDir()
{
    return k_PathTempDir;
}


// test if a path exists
bool PathExists(String const &PathName)
{
    bool rv = false;
    struct _stat st;

    if (_stat(PathName.c_str(), &st) == 0)
    {
        rv = true;
    }

    return rv;
}


// create a directory
bool DirCreate(String const &PathName)
{
    bool rv = true;

    if (!PathExists(PathName))
    {
        rv = (_mkdir(PathName.c_str()) == 0);
    }

    return rv;
}


// start a new process
uint32_t StartProcess(cp::String const &FilePath, StringVec_t const &Args, StringVec_t const &EnvVars)
{
    uint32_t procId = 0;
    String arg;
    StreamBuf env;
    Buffer strBlock;
    Buffer envBlock;
    StringVec_t::const_iterator i;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // setup arg string
    i = Args.begin();

    while (i != Args.end())
    {
        arg = arg + *i + " ";
        ++i;
    }

    strBlock = arg;

    // setup env block
    i = EnvVars.begin();

    while (i != EnvVars.end())
    {
        env.ArrayWr(i->c_str(), i->length());
        env.Write('\0');
        ++i;
    }

    // terminate the block
    env.Write('\0');
    env.Seek(0);
    env.Read(envBlock, env.LenGet());

    // prespare the win32 structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // launch the executable
    if (CreateProcess(NULL,
                      strBlock.c_str(),
                      NULL,
                      NULL,
                      FALSE,
                      0,
                      envBlock.c_str(),
                      NULL,
                      &si,
                      &pi))
    {
        procId = pi.dwProcessId;

        // close process handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
    {
        cp::LogErr << "cp::StartProcess(): Failed to start process: " << FilePath << std::endl;
    }

    return procId;
}


// get current task identifier
uint32_t TaskId()
{
    return GetCurrentProcessId();
}


// get current thread identifier
uint32_t ThreadId()
{
    return ThreadId_Impl();
}


// relinquish current thread's execution
bool ThreadYield()
{
    cp::NativeSleep(0);
    return true;
}


// enter critical secion
bool CriticalEnter()
{
    bool rv = false;
    // (.)(.) need to implement CriticalEnter()
    return rv;
}


// exit critical section
bool CriticalExit()
{
    bool rv = false;
    // (.)(.) need to implement CriticalExit()
    return rv;
}


// return time in seconds (unix epoch)
uint32_t Time32()
{
    return ((Time64() / 1000) & 0xffffffff);
}


// return time in milliseconds (unix epoch)
uint64_t Time64()
{
    uint64_t time64;
    uint64_t offset = 116444736;
    FILETIME ftime;

    GetSystemTimeAsFileTime(&ftime);
    time64 = *reinterpret_cast<uint64_t *>(&ftime);
    time64 /= 10000;
    time64 += (offset * 1000 * 1000 * 1000);

    return time64;
}


// suspend execution for second intervals
bool Sleep(uint32_t Delay)
{
    return cp::NativeSleep(Delay * 1000);
}


// suspend execution for millisecond intervals
bool MilliSleep(uint32_t Delay)
{
    return cp::NativeSleep(Delay);
}


// suspend execution for microsecond intervals
bool MicroSleep(uint32_t Delay)
{
    (void)Delay;
    return cp::NativeSleep(1);    // (.)(.)
}


// suspend execution for nanosecond intervals
bool NanoSleep(uint32_t Delay)
{
    (void)Delay;
    return cp::NativeSleep(1);    // (.)(.)
}


// convert a numeric IPv4 address to a string
String Ipv4ToStr(uint32_t Addr)
{
    struct in_addr addr_in;

    addr_in.s_addr = htonl(Addr);
    return inet_ntoa(addr_in);
}


// convert a string to a numeric IPv4 address
uint32_t StrToIpv4(String Addr)
{
    return ntohl(inet_addr(Addr.c_str()));
}

}   // namespace cp
