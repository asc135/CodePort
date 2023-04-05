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
//  2022-06-09  asc Added CpuTime64() function.
//  2022-06-10  asc Added RunProgramGetOutput() function.
//  2023-04-04  asc Added file size, attribute, type, and ipv6 functions.
// ----------------------------------------------------------------------------

#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <cstdlib>      // exit()
#include <ctime>

#include "cpUtil.h"
#include "cpBuffer.h"
#include "cpSubProcess.h"

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
uint32_t StartProcess(String const &FilePath, StringVec_t const &Args, StringVec_t const &EnvVars)
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
        LogErr << "StartProcess(): Failed to fork() process: " << FilePath << std::endl;
    }
    else
    {
        if (pid == 0)
        {
            // the child process
            if (execve(FilePath.c_str(), (char * const *)arg.data(), (char * const *)env.data()) < 0)
            {
                LogErr << "StartProcess(): Failed to execve() process: " << FilePath << std::endl;
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


// run a program and get its output
bool RunProgramGetOutput(String const &Command, StringVec_t &Output)
{
    SubProcess proc(Command, k_FlowOut);
    bool rv = proc.IsValid();

    if (rv)
    {
        Buffer buf;
        proc.WaitUntilDone();
        proc.BufferExtract(buf);

        if (buf.Size())
        {
            BufferToLines(buf.c_str(), buf.LenGet(), Output);
        }
    }

    return rv;
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
        LogErr << "Time64(): gettimeofday() returned error!" << std::endl;
    }

    return ms;
}


// return process CPU time in milliseconds
uint64_t CpuTime64()
{
    uint64_t rv = 0;
    timespec tm;

    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tm) == 0)
    {
        rv = (tm.tv_sec * 1000) + (tm.tv_nsec / 1000000);
    }

    return rv;
}


// suspend execution for second intervals
bool Sleep(uint32_t Delay)
{
    timespec tv;

    tv.tv_sec = Delay;
    tv.tv_nsec = 0;

    return NativeSleep(tv);
}


// suspend execution for millisecond intervals
bool MilliSleep(uint32_t Delay)
{
    timespec tv;

    tv.tv_sec = Delay / 1000;
    tv.tv_nsec = (Delay % 1000) * 1000000;

    return NativeSleep(tv);
}


// suspend execution for microsecond intervals
bool MicroSleep(uint32_t Delay)
{
    timespec tv;

    tv.tv_sec = Delay / 1000000;
    tv.tv_nsec = (Delay % 1000000) * 1000;

    return NativeSleep(tv);
}


// suspend execution for nanosecond intervals
bool NanoSleep(uint32_t Delay)
{
    timespec tv;

    tv.tv_sec = Delay / 1000000000;
    tv.tv_nsec = (Delay % 1000000000);

    return NativeSleep(tv);
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


// determine if a path is a file or a directory
bool GetPathType(String const &Path, bool &IsFile, bool &IsDir)
{
    struct stat fs;
    bool rv = (stat(Path.c_str(), &fs) == 0);

    //    S_IFMT     0170000   bit mask for the file type bit field
    //
    //    S_IFSOCK   0140000   socket
    //    S_IFLNK    0120000   symbolic link
    //    S_IFREG    0100000   regular file
    //    S_IFBLK    0060000   block device
    //    S_IFDIR    0040000   directory
    //    S_IFCHR    0020000   character device
    //    S_IFIFO    0010000   FIFO

    if (rv)
    {
        IsFile = ((fs.st_mode & S_IFMT) == S_IFREG);
        IsDir = ((fs.st_mode & S_IFMT) == S_IFDIR);
    }
    else
    {
        IsFile = false;
        IsDir = false;
    }

    return rv;
}


// get file size
size_t GetFileSize(String const &Path)
{
    struct stat fs;
    size_t size = 0;

    if (stat(Path.c_str(), &fs) == 0)
    {
        size = fs.st_size;
    }

    return size;
}


// get file size
size_t GetFileSize(desc_t Descriptor)
{
    struct stat fs;
    size_t size = 0;

    if (fstat(Descriptor, &fs) == 0)
    {
        size = fs.st_size;
    }

    return size;
}


// get file attributes
uint32_t GetFileAttr(String const &Path)
{
    struct stat fs;
    uint32_t mode = 0;

    if (stat(Path.c_str(), &fs) == 0)
    {
        mode = fs.st_mode;
    }

    return mode;
}


// get file attributes
uint32_t GetFileAttr(desc_t Descriptor)
{
    struct stat fs;
    uint32_t mode = 0;

    if (fstat(Descriptor, &fs) == 0)
    {
        mode = fs.st_mode;
    }

    return mode;
}


// convert a numeric IPv4 address to a string
String Ipv4ToStr(uint32_t Addr)
{
    struct in_addr addr_in;
    char ipAddr[INET_ADDRSTRLEN + 1];

    memset(ipAddr, 0, sizeof(ipAddr));
    addr_in.s_addr = htonl(Addr);
    inet_ntop(AF_INET, &addr_in, ipAddr, sizeof(ipAddr));

    return ipAddr;
}


// convert a numeric IPv4 address to a string
String Ipv4ToStr(void const *pAddr)
{
    struct in_addr const *pAddr_in = (struct in_addr const *)pAddr;
    char ipAddr[INET_ADDRSTRLEN + 1];

    memset(ipAddr, 0, sizeof(ipAddr));
    inet_ntop(AF_INET, pAddr_in, ipAddr, sizeof(ipAddr));

    return ipAddr;
}


// convert a string to a numeric IPv4 address
uint32_t StrToIpv4(String const &Addr)
{
    uint32_t rv = 0;
    struct in_addr addr;

    if (inet_pton(AF_INET, Addr.c_str(), &addr) > 0)
    {
        rv = ntohl(addr.s_addr);
    }

    return rv;
}


// convert IPv6 address to string
String Ipv6ToStr(void const *pAddr)
{
    char ipAddr[INET6_ADDRSTRLEN + 1];

    memset(ipAddr, 0, sizeof(ipAddr));
    inet_ntop(AF_INET6, pAddr, ipAddr, sizeof(ipAddr));
    return ipAddr;
}


// convert a string to a numeric IPv6 address
Buffer StrToIpv6(String const &Addr)
{
    struct in6_addr addr;
    Buffer buf;

    if (inet_pton(AF_INET6, Addr.c_str(), &addr) > 0)
    {
        buf.CopyIn((uint8_t *)&addr.s6_addr, sizeof(in6_addr));
    }

    return buf;
}

}   // namespace cp
