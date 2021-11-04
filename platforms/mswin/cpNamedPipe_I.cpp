// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpNamedPipe_I.cpp
//
//  Description:    Named Pipe Facility.
//
//  Platform:       mswin
//
//  History:
//  2012-12-11  asc Creation.
// ----------------------------------------------------------------------------

#include "cpNamedPipe.h"

namespace cp
{

// constructor
NamedPipe::NamedPipe(String const &Name, String const &Path, PipeDir Direction, bool Create, size_t BufSize) :
    IoDev(Name),
    m_Cleanup(Create)
{
    // create or open the named pipe
    if (Name.size() > 0)
    {
        HANDLE pipe;

        // assign the fixed Windows path name
        m_SysName = "\\\\.\\pipe\\";
        m_SysName += Path;

        // create the named pipe
        if (Create)
        {
            pipe = CreateNamedPipe(m_SysName.c_str(),
                                   PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                   PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, // | PIPE_NOWAIT,
                                   PIPE_UNLIMITED_INSTANCES,
                                   BufSize,
                                   BufSize,
                                   k_DefaultTimeout, NULL);
        }
        else
        {
            pipe = CreateFile(m_SysName.c_str(),
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_FLAG_OVERLAPPED,
                              NULL);
        }

        if (Direction == k_Read)
        {
            // assign the pipe file for read
            m_dRead = pipe;

            if (m_dRead != INVALID_HANDLE_VALUE)
            {
                m_Valid = true;
            }
            else
            {
                LogErr << "NamedPipe::NamedPipe(): Failed to open named pipe for read: "
                       << NameGet() << std::endl;
            }
        }
        else
        {
            // assign the pipe file for write
            m_dWrite = pipe;

            if (m_dWrite != INVALID_HANDLE_VALUE)
            {
                m_Valid = true;
            }
            else
            {
                LogErr << "NamedPipe::NamedPipe(): Failed to open named pipe for write: "
                       << NameGet() << std::endl;
            }
        }
    }
    else
    {
        LogErr << "NamedPipe::NamedPipe(): Empty string passed for named pipe name."
               << std::endl;
    }

    // if not successful, clear the descriptors
    if (!m_Valid)
    {
        m_dWrite = INVALID_HANDLE_VALUE;
        m_dRead  = INVALID_HANDLE_VALUE;
    }
}


// destructor
NamedPipe::~NamedPipe()
{
    // make sure device descriptor is closed
    Complete();
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
int NamedPipe::SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout)
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
int NamedPipe::RecvData(char *pBuf, size_t RcvLen, size_t BytesRead, uint32_t Timeout)
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
//  Function Name:  Complete
//
//  Description:    Completes the pipe's data transfer.
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        none
// ------------------------------------------------------------------------- */
void NamedPipe::Complete()
{
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


// create the fifo file
bool NamedPipe::CreateDevice()
{
    return true;
}

}   // namespace cp
