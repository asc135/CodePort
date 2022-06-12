// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2022 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSubProcess_I.cpp
//
//  Description:    Creates a subprocess with a pipe to parent.
//
//  Platform:       posix
//
//  History:
//  2022-02-02  asc Creation.
//  2022-02-04  asc Added Cancel method.
//  2022-03-02  asc Added StreamBufTransfer() method.
//  2022-06-02  asc Changed to ArrayWr() to avoid constructing a String.
//  2022-06-12  asc Added detection of subprocess termination and handling.
// ----------------------------------------------------------------------------

// (.)(.) 2022-02-03 asc Need to implement the k_FlowIn mode.

// (.)(.) 2022-02-03 asc This implementation is mostly "common" save for the popen()
// and related calls.  It can be re-implemented using fork() and using descriptors to
// make it truly posix and provide the ability to kill the subprocess if needed.  The
// portable portions (majority of the module) can be moved to src from platform/posix
// with the posix-specific bits kept in the implementation file.

#include <unistd.h>
#include <fcntl.h>

#include "cpUtil.h"
#include "cpBuffer.h"
#include "cpSubProcess.h"

namespace cp
{

// constructor
SubProcess::SubProcess(cp::String const &Command, SubProcIoDirection Dir) :
    Base("SubProcess: " + Command),
    m_Dir(Dir),
    m_PtrHandle(0),
    m_Descriptor(0),
    m_IoThread("SubProcess Thread: " + Command, ThreadFunction, this, Thread::opt_Suspended),
    m_Completed("SubProcess Semaphore", 0, 1)
{
    cp::String mode;

    if (Command.length() > 0)
    {
        switch (m_Dir)
        {
            case k_FlowIn:
                mode = "w";
                break;

            case k_FlowOut:
                mode = "r";
                break;

            default:
                mode = "x";
        };

        if (mode != "x")
        {
            m_PtrHandle = popen(Command.c_str(), mode.c_str());
        }

        if (m_PtrHandle)
        {
            // mark the underlying descriptor as non-blocking
            m_Descriptor = fileno(m_PtrHandle);
            fcntl(m_Descriptor, F_SETFL, O_NONBLOCK);

            // if success, mark instance valid and start the I/O thread
            m_Valid = true;
            m_IoThread.Resume();
        }
        else
        {
            // if failed, give the completed semaphore to signal it's not running
            m_Completed.Give();
        }
    }
}


// destructor
SubProcess::~SubProcess()
{
    if (m_Valid)
    {
        Cancel();
        WaitUntilDone();
        pclose(m_PtrHandle);
    }
}


// cancel execution
void SubProcess::Cancel()
{
    m_IoThread.ExitReq();
}


// check if subprocess currently running
bool SubProcess::IsRunning()
{
    bool rv = m_Completed.TryTake();

    if (rv)
    {
        m_Completed.Give();
    }

    return !rv;
}


// returns when subprocess completes
void SubProcess::WaitUntilDone()
{
    m_Completed.Take();
    m_Completed.Give();
}


// copy out the stream buffer
void SubProcess::BufferExtract(Buffer &Buf)
{
    m_SyncIo.Lock();
    // terminate the copied data as a C string
    m_RxBuffer.OctetInsert(0x00);
    // copy data to the supplied buffer
    Buf = m_RxBuffer;
    // clear the receive buffer
    m_RxBuffer.Clear();
    m_SyncIo.Unlock();
}


// transfer out the stream buffer
void SubProcess::StreamBufTransfer(StreamBuf &Dest)
{
    m_SyncIo.Lock();
    Dest.TransferBlocksFrom(m_RxBuffer);
    m_SyncIo.Unlock();
}


// subprocess I/O thread function
void SubProcess::IoThread()
{
    cp::Buffer buf(128);

    // loop while thread is active and handle is valid and not end of file
    while (m_IoThread.ThreadPoll())
    {
        ssize_t result;

        // attempt to read from handle
        result = read(m_Descriptor, buf.c_str(), buf.Size() - 1);

        if (result > 0)
        {
            // terminate the data
            buf.c_str()[result] = '\0';
            m_SyncIo.Lock();
            m_RxBuffer.ArrayWr(buf.c_str(), result + 1);
            m_SyncIo.Unlock();
        }
        else
        {
            // returns 0 if pipe is closed from other end (i.e. process terminated)
            if (result == 0)
            {
                m_IoThread.ExitReq();
            }
            else
            {
                // otherwise delay to give time for more data to arrive
                cp::MilliSleep(200);
            }
        }
    }

    // give semaphore to indicate subprocess has completed running
    m_Completed.Give();
}


// static thread trampoline function
void *SubProcess::ThreadFunction(cp::Thread *pThread)
{
    reinterpret_cast<SubProcess *>(pThread->ContextGet())->IoThread();
    return pThread;
}

}   // namespace cp
