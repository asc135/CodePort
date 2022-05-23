// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpUtil_I.cpp
//
//  Description:    Utility Function Library.
//
//  Platform:       posix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Renamed static Sleep() to NativeSleep().
//  2012-03-16  asc Added Ipv4ToStr() and StrToIpv4() functions.
//  2013-06-13  asc Added StartProcess() function.
//  2013-12-18  asc Added DirCreate() and moved PathCreate() into common module.
//  2014-10-22  asc Applied patch from K. Holmes to fix Time64() overflow on 32-bit systems.
//  2021-12-16  asc Added ThreadYield_Impl() for portability.
//  2021-12-17  asc Added AF_INET check for portability.
//  2022-05-22  asc Added HostName() and DomainName() functions.
// ----------------------------------------------------------------------------

#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <cstdlib>      // exit()
#include <ctime>

#include "cpUtil.h"

#ifndef AF_INET
#include <sys/socket.h>
#endif

namespace cp
{

// module local function used by the public delay calls
static bool NativeSleep(timespec &Interval)
{
    bool rv = false;
    bool exitFlag = false;
    timespec remains;

    // loop until sleep is complete or unrecoverably fails
    while (!exitFlag)
    {
        if (nanosleep(&Interval, &remains) == 0)
        {
            rv = true;
            exitFlag = true;
        }
        else
        {
            // errno is set by nanosleep if it fails
            switch (errno)
            {
            case EINTR:     // sleep was interrupted by a signal so make another call to nanosleep
                Interval.tv_sec = remains.tv_sec;
                Interval.tv_nsec = remains.tv_nsec;
                break;

            case EFAULT:    // there was a problem copying information from user space
            case EINVAL:    // value is out of range (0 > duration_nsec > 999,999,999) or (duration_sec < 0)
            default:
                exitFlag = true;
                break;
            }
        }
    }

    return rv;
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
    struct stat st;

    if (stat(PathName.c_str(), &st) == 0)
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
        rv = (mkdir(PathName.c_str(), 0755) == 0);
    }

    return rv;
}


// start a new process
uint32_t StartProcess(cp::String const &FilePath, StringVec_t const &Args, StringVec_t const &EnvVars)
{
    uint32_t procId = 0;
    CStringVec_t arg;
    CStringVec_t env;
    StringVec_t::const_iterator i;
    int pid = 0;

    // setup arg vector
    i = Args.begin();

    while (i != Args.end())
    {
        arg.push_back(i->c_str());
        ++i;
    }

    // terminate the list
    arg.push_back(NULL);

    // setup env vector
    i = EnvVars.begin();

    while (i != EnvVars.end())
    {
        env.push_back(i->c_str());
        ++i;
    }

    // terminate the list
    env.push_back(NULL);

    // launch the executable
    pid = fork();

    if (pid < 0)
    {
        cp::LogErr << "cp::StartProcess(): Failed to fork() process: " << FilePath << std::endl;
    }
    else
    {
        if (pid == 0)
        {
            // the child process
            if (execve(FilePath.c_str(), (char * const *)arg.data(), (char * const *)env.data()) < 0)
            {
                cp::LogErr << "cp::StartProcess(): Failed to execve() process: " << FilePath << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // the parent process
            procId = static_cast<uint32_t>(pid);
        }
    }

    return procId;
}


// get current task identifier
uint32_t TaskId()
{
    return static_cast<uint32_t>(getpid());
}


// get current thread identifier
uint32_t ThreadId()
{
    return ThreadId_Impl();
}


// relinquish current thread's execution
bool ThreadYield()
{
    return (ThreadYield_Impl() == 0);
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
    return static_cast<uint32_t>(time(NULL));
}


// return time in milliseconds (unix epoch)
uint64_t Time64()
{
    timeval tv;
    uint64_t ms = 0;

    if (gettimeofday(&tv, NULL) == 0)
    {
        uint64_t tv_sec = tv.tv_sec;
        uint64_t tv_usec = tv.tv_usec;
        ms = tv_sec * 1000 + tv_usec / 1000;
    }
    else
    {
        LogErr << "cp::Time64(): gettimeofday() returned error!" << std::endl;
    }

    return ms;
}


// suspend execution for second intervals
bool Sleep(uint32_t Delay)
{
    timespec tv;

    tv.tv_sec = Delay;
    tv.tv_nsec = 0;

    return cp::NativeSleep(tv);
}


// suspend execution for millisecond intervals
bool MilliSleep(uint32_t Delay)
{
    timespec tv;

    tv.tv_sec = Delay / 1000;
    tv.tv_nsec = (Delay % 1000) * 1000000;

    return cp::NativeSleep(tv);
}


// suspend execution for microsecond intervals
bool MicroSleep(uint32_t Delay)
{
    timespec tv;

    tv.tv_sec = Delay / 1000000;
    tv.tv_nsec = (Delay % 1000000) * 1000;

    return cp::NativeSleep(tv);
}


// suspend execution for nanosecond intervals
bool NanoSleep(uint32_t Delay)
{
    timespec tv;

    tv.tv_sec = Delay / 1000000000;
    tv.tv_nsec = (Delay % 1000000000);

    return cp::NativeSleep(tv);
}


// convert a numeric IPv4 address to a string
String Ipv4ToStr(uint32_t Addr)
{
    struct in_addr addr_in;
    char ipAddr[INET_ADDRSTRLEN + 1];

    addr_in.s_addr = htonl(Addr);
    inet_ntop(AF_INET, &addr_in, ipAddr, sizeof(ipAddr));

    return ipAddr;
}


// convert a string to a numeric IPv4 address
uint32_t StrToIpv4(String Addr)
{
    uint32_t rv = 0;
    struct in_addr addr;

    if (inet_pton(AF_INET, Addr.c_str(), &addr) > 0)
    {
        rv = ntohl(addr.s_addr);
    }

    return rv;
}


// obtain the system's host name
String &HostName(String &Name)
{
    char buf[256];

    if (gethostname(buf, sizeof(buf)) == 0)
    {
        Name = buf;
    }
    else
    {
        Name.clear();
    }

    return Name;
}


// obtain the system's domain name
String &DomainName(String &Name)
{
    char buf[256];

    if (getdomainname(buf, sizeof(buf)) == 0)
    {
        Name = buf;
    }
    else
    {
        Name.clear();
    }

    return Name;
}

}   // namespace cp
