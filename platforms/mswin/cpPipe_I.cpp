// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpPipe_I.cpp
//
//  Description:    Data Pipe Facility.
//
//  Platform:       mswin
//
//  History:
//  2012-12-11  asc Creation.
// ----------------------------------------------------------------------------

#include "cpPipe.h"

namespace cp
{

// constructor
Pipe::Pipe(String const &Name, size_t BufSize) :
    IoDev(Name)
{
    // create the named pipe
    if (CreatePipe(&m_dRead, &m_dWrite, NULL, BufSize))
    {
        m_Valid = true;
    }
    else
    {
        LogErr << "Pipe::Pipe(): Failed to create pipe: "
               << NameGet() << std::endl;
    }

    // if not successful, clear the descriptors
    if (!m_Valid)
    {
        m_dWrite = INVALID_HANDLE_VALUE;
        m_dRead  = INVALID_HANDLE_VALUE;
    }
}


// destructor
Pipe::~Pipe()
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
int Pipe::SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout)
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
int Pipe::RecvData(char *pBuf, size_t RcvLen, size_t BytesRead, uint32_t Timeout)
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
void Pipe::Complete()
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

}   // namespace cp
