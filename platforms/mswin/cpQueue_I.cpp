// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpQueue_I.cpp
//
//  Description:    Inter-Process Message Queue Facility.
//
//  Platform:       mswin
//
//  History:
//  2012-12-11  asc Creation.
// ----------------------------------------------------------------------------

#include "cpQueue.h"
#include "cpBuffer.h"

namespace cp
{

// constructor, owner
Queue::Queue(String const &Name, size_t MsgSize, uint32_t MaxMsgs) :
    IoDev(Name),
    m_Cleanup(true),
    m_MsgSize(MsgSize),
    m_MaxMsgs(MaxMsgs)
{
    // create or open the queue
    if (Name.size() > 0)
    {
        // assign the fixed Windows path name
        m_SysName = "\\\\.\\pipe\\";
        m_SysName += Name;

        // create the queue (a named pipe in message mode)

        m_dWrite = CreateNamedPipe(m_SysName.c_str(),
                                   PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                   PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, // | PIPE_NOWAIT,
                                   PIPE_UNLIMITED_INSTANCES,
                                   MsgSize * MaxMsgs,
                                   MsgSize * MaxMsgs,
                                   k_DefaultTimeout, NULL);

        if (m_dWrite != INVALID_HANDLE_VALUE)
        {
            m_dRead = CreateFile(m_SysName.c_str(),
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_OVERLAPPED,
                                 NULL);

            if (m_dRead != INVALID_HANDLE_VALUE)
            {
                DWORD mode = PIPE_READMODE_MESSAGE;

                // set the read descriptor to message mode
                if (SetNamedPipeHandleState(m_dRead, &mode, NULL, NULL))
                {
                    m_Valid = true;
                }
            }
            else
            {
                LogErr << "Queue::Queue(): Error opening queue for read: "
                       << NameGet() << std::endl;
                PrintGetLastError(GetLastError());
            }
        }
        else
        {
            LogErr << "Queue::Queue(): Error opening queue for write: "
                   << NameGet() << std::endl;
        }
    }
    else
    {
        LogErr << "Queue::Queue(): Empty string passed for queue name."
               << std::endl;
    }
}


// constructor, user
Queue::Queue(String const &Name) :
    IoDev(Name),
    m_Cleanup(false),
    m_MsgSize(0),
    m_MaxMsgs(0)
{
    // create or open the queue
    if (Name.size() > 0)
    {
        // assign the fixed Windows path name
        m_SysName = "\\\\.\\pipe\\";
        m_SysName += Name;

        // create the queue (a named pipe in message mode)

        m_dWrite = CreateFile(m_SysName.c_str(),
                              GENERIC_WRITE | GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_FLAG_OVERLAPPED,
                              NULL);

        if (m_dWrite != INVALID_HANDLE_VALUE)
        {
            m_dRead = CreateFile(m_SysName.c_str(),
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_OVERLAPPED,
                                 NULL);

            if (m_dRead != INVALID_HANDLE_VALUE)
            {
                DWORD mode = PIPE_READMODE_MESSAGE;

                // set the read descriptor to message mode
                if (SetNamedPipeHandleState(m_dWrite, &mode, NULL, NULL))
                {
                    if (SetNamedPipeHandleState(m_dRead, &mode, NULL, NULL))
                    {
                        m_Valid = true;
                    }
                }
            }
            else
            {
                LogErr << "Queue::Queue(): Error opening queue for read: "
                       << NameGet() << std::endl;
            }
        }
        else
        {
            LogErr << "Queue::Queue(): Error opening queue for write: "
                   << NameGet() << std::endl;
        }

    }
    else
    {
        LogErr << "Queue::Queue(): Empty string passed for queue name."
               << std::endl;
    }
}


// destructor
Queue::~Queue()
{
    // make sure device descriptor is closed
    if (m_dRead != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_dRead);
        m_dRead = INVALID_HANDLE_VALUE;
    }

    if (m_dWrite != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_dWrite);
        m_dWrite = INVALID_HANDLE_VALUE;
    }
}


// ----------------------------------------------------------------------------
//  Function Name:  SendReady
//
//  Description:    test if queue is ready to accept data
//
//  Inputs:         Timeout - time to wait in milliseconds for ready state
//
//  Outputs:        none
//
//  Returns:        true if ready, false if not ready
// ----------------------------------------------------------------------------
bool Queue::SendReady(uint32_t Timeout)
{
    bool rv = true;

    (void)Timeout;

    // win32 does this using asynchronous I/O

    return rv;
}


// ----------------------------------------------------------------------------
//  Function Name:  RecvReady
//
//  Description:    test if queue has data ready to read
//
//  Inputs:         Timeout - time to wait in milliseconds for ready state
//
//  Outputs:        none
//
//  Returns:        true if ready, false if not ready
// ----------------------------------------------------------------------------
bool Queue::RecvReady(uint32_t Timeout)
{
    bool rv = true;

    (void)Timeout;

    // win32 does this using asynchronous I/O

    return rv;
}


// ----------------------------------------------------------------------------
//  Function Name:  SendData
//
//  Description:    sends data to the write descriptor
//
//  Inputs:         pBuf - pointer to source buffer
//                  SndLen - number of bytes to send
//                  BytesWritten - number of bytes previously written
//                  Timeout - I/O timeout
//
//  Outputs:        none
//
//  Returns:        number of characters read or < 0 if error
// ------------------------------------------------------------------------- */
int Queue::SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout)
{
    int rv = k_Error;
    IoResults results(m_dWrite);
    bool exitFlag = false;
    uint32_t waiting = Timeout / k_IoPendingInterval;

    while (!exitFlag)
    {
        if (WriteFileEx(m_dWrite,
                        pBuf + BytesWritten,
                        SndLen - BytesWritten,
                        (LPOVERLAPPED)&results,
                        IoCallback))
        {
            DWORD bytes = 0;

            // wait for operation to complete
            results.WaitComplete(Timeout);

            if (GetOverlappedResult(m_dWrite, (LPOVERLAPPED)&results, &bytes, false))
            {
                rv = bytes;
            }

            exitFlag = true;
        }
        else
        {
            exitFlag = (--waiting == 0);

            if (!exitFlag)
            {
                ::Sleep(k_IoPendingInterval);
            }
        }
    }

    return rv;
}


// ----------------------------------------------------------------------------
//  Function Name:  RecvData
//
//  Description:    receives data from the read descriptor
//
//  Inputs:         pBuf - pointer to target buffer
//                  RcvLen - number of bytes to receive
//                  BytesRead - number of bytes previously read
//                  Timeout - I/O timeout
//
//  Outputs:        none
//
//  Returns:        number of characters read or < 0 if error
// ------------------------------------------------------------------------- */
int Queue::RecvData(char *pBuf, size_t RcvLen, size_t BytesRead, uint32_t Timeout)
{
    int rv = k_Error;
    IoResults results(m_dRead);
    bool exitFlag = false;
    uint32_t waiting = Timeout / k_IoPendingInterval;

    while (!exitFlag)
    {
        if (ReadFileEx(m_dRead,
                       pBuf + BytesRead,
                       RcvLen - BytesRead,
                       (LPOVERLAPPED)&results,
                       IoCallback))
        {
            DWORD bytes = 0;

            // wait for operation to complete
            results.WaitComplete(Timeout);

            if (GetOverlappedResult(m_dRead, (LPOVERLAPPED)&results, &bytes, false))
            {
                rv = bytes;
            }

            exitFlag = true;
        }
        else
        {
            exitFlag = (--waiting == 0);

            if (!exitFlag)
            {
                ::Sleep(k_IoPendingInterval);
            }
        }
    }

    return rv;
}


// ----------------------------------------------------------------------------
//  Function Name:  Flush
//
//  Description:    force the queue to empty
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        none
// ----------------------------------------------------------------------------
void Queue::Flush()
{
    Buffer buf(m_MsgSize);

    // abort if object not properly initialized
    if (!m_Valid)
    {
        return;
    }

    // pull message off queue while messages are available.  Don't use "wait"
    // mode because you don't want to block if the queue is empty
    while (!IsEmpty())
    {
        Recv(buf, buf);
    }
}


// ----------------------------------------------------------------------------
//  Function Name:  IsEmpty
//
//  Description:    tests if the queue is empty
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        true if the queue is empty
// ----------------------------------------------------------------------------
bool Queue::IsEmpty() const
{
    return (NumUsed() == 0);
}


// ----------------------------------------------------------------------------
//  Function Name:  IsFull
//
//  Description:    tests if the queue is full
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        true if the queue is full
// ----------------------------------------------------------------------------
bool Queue::IsFull() const
{
    return (NumUsed() == m_MaxMsgs);
}


// ----------------------------------------------------------------------------
//  Function Name:  NumFree
//
//  Description:    retrieve number of free positions in queue
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        number of free positions in the queue
// ----------------------------------------------------------------------------
uint32_t Queue::NumFree() const
{
    uint32_t rv = 0;
    // (.)(.)
    return rv;
}


// ----------------------------------------------------------------------------
//  Function Name:  NumUsed
//
//  Description:    retrieve number of used positions in queue
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        number of used positions in the queue
// ----------------------------------------------------------------------------
uint32_t Queue::NumUsed() const
{
    uint32_t rv = 0;
    // (.)(.)
    return rv;
}


// ----------------------------------------------------------------------------
//  Function Name:  MaxMsgsGet
//
//  Description:    retrieve maximum number of messages possible in queue
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        maximum number of positions in queue
// ----------------------------------------------------------------------------
uint32_t Queue::MaxMsgsGet() const
{
    return m_MaxMsgs;
}

}   // namespace cp
