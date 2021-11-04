// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIoDev_I.cpp
//
//  Description:    I/O Base Class.
//
//  Platform:       posix
//
//  History:
//  2011-06-30  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Added timeout parameter to SendData() and recvData().
//  2013-03-31  asc Added support for read and write descriptors being the same.
//  2013-05-08  asc Added SndLen and RcvLen to Buffer versions of Send() and Recv().
// ----------------------------------------------------------------------------

#include <sys/time.h>

#include "cpIoDev.h"
#include "cpBuffer.h"
#include "cpUtil.h"

namespace cp
{

// constructor
IoDev::IoDev(String const &Name) :
    Base(Name),
    m_FullRead(false),
    m_Retries(0),
    m_RetryDelay(100),
    m_dWrite(k_InvalidDescriptor),
    m_dRead(k_InvalidDescriptor)
{
}


// destructor
IoDev::~IoDev()
{
    // check if read and write descriptors are the same
    if (m_dWrite == m_dRead)
    {
        // set one of them to invalid since only one needs to be closed
        m_dRead = k_InvalidDescriptor;
    }

    // close the write descriptor
    if (m_dWrite != k_InvalidDescriptor)
    {
        if (close(m_dWrite) == k_Error)
        {
            LogErr << "IoDev::~IoDev(): Failed to close write descriptor for: "
                   << NameGet() << std::endl;
        }
    }

    // close the read descriptor
    if (m_dRead != k_InvalidDescriptor)
    {
        if (close(m_dRead) == k_Error)
        {
            LogErr << "IoDev::~IoDev(): Failed to close read descriptor for: "
                   << NameGet() << std::endl;
        }
    }
}


// ----------------------------------------------------------------------------
//  Function Name:  Send
//
//  Description:    send data to the device
//
//  Inputs:         Buf - reference to source buffer
//                  SndLen - number of bytes to send
//                  Timeout - maximum number of milliseconds to block
//
//  Outputs:        none
//
//  Returns:        number of characters written or < 0 if error
// ------------------------------------------------------------------------- */
int IoDev::Send(Buffer const &Buf, size_t SndLen, uint32_t Timeout)
{
    size_t length = Buf.LenGet();

    // make sure transmit length is always less than data length
    if (length > SndLen)
    {
        length = SndLen;
    }

    return Send(Buf.c_str(), length, Timeout);
}


// ----------------------------------------------------------------------------
//  Function Name:  Recv
//
//  Description:    receive data from the device
//
//  Inputs:         Buf - reference to target buffer
//                  RcvLen - number of bytes to receive
//                  Timeout - maximum number of milliseconds to block
//
//  Outputs:        none
//
//  Returns:        number of characters read or < 0 if error
// ------------------------------------------------------------------------- */
int IoDev::Recv(Buffer &Buf, size_t RcvLen, uint32_t Timeout)
{
    int rv = k_Error;

    // make sure there is enough buffer space to receive the number of octets requested
    if (Buf.Resize(RcvLen))
    {
        rv = Recv(Buf.c_str(), RcvLen, Timeout);
    }

    // if data was received, set the buffer's data length accordingly
    if (rv > 0)
    {
        Buf.LenSet(rv);
    }
    else
    {
        Buf.LenSet(0);
    }

    return rv;
}


// ----------------------------------------------------------------------------
//  Function Name:  Send
//
//  Description:    send data to the device
//
//  Inputs:         pBuf - pointer to source buffer
//                  SndLen - number of bytes to send
//                  Timeout - maximum number of milliseconds to block
//
//  Outputs:        none
//
//  Returns:        number of characters written or < 0 if error
// ------------------------------------------------------------------------- */
int IoDev::Send(char const *pBuf, size_t SndLen, uint32_t Timeout)
{
    int rv = k_Error;

    // abort if object not properly initialized or bad parameters are passed
    if (!m_Valid || (pBuf == NULL))
    {
        return rv;
    }

    // attempt to write data
    if (m_Valid)
    {
        size_t bytesWritten = 0;
        bool writeReady = true;
        bool exitFlag = false;
        uint32_t retries = m_Retries;

        // send the next message.  Stay in loop until error, all bytes have been written, or timeout
        while ((bytesWritten < SndLen) && !exitFlag)
        {
            // determine if call should block if descriptor not ready
            if (Timeout != k_InfiniteTimeout)
            {
                writeReady = SendReady(Timeout);
            }

            if (writeReady)
            {
                rv = SendData(pBuf, SndLen, bytesWritten, Timeout);

                // closed descriptor or error case... force exit from while loop
                if (rv <= 0)
                {
                    if (retries == 0)
                    {
                        // force exit condition
                        exitFlag = true;
                    }
                    else
                    {
                        --retries;
                        cp::MilliSleep(m_RetryDelay);
                    }
                }
                else
                {
                    bytesWritten += rv;
                }
            }
            else
            {
                exitFlag = true;
            }

            rv = bytesWritten;
        }
    }

    return rv;
}


// ----------------------------------------------------------------------------
//  Function Name:  Recv
//
//  Description:    receive data from the device
//
//  Inputs:         pBuf - pointer to target buffer
//                  RcvLen - number of bytes to receive
//                  Timeout - maximum number of milliseconds to block
//
//  Outputs:        none
//
//  Returns:        number of characters read or < 0 if error
// ------------------------------------------------------------------------- */
int IoDev::Recv(char *pBuf, size_t RcvLen, uint32_t Timeout)
{
    int rv = k_Error;

    // abort if object not properly initialized or bad parameters are passed
    if (!m_Valid || (pBuf == NULL))
    {
        return rv;
    }

    // attempt to read data
    if (m_Valid)
    {
        size_t bytesRead = 0;
        bool readReady = true;
        bool exitFlag = false;
        uint32_t retries = m_Retries;

        // receive the data.  Stay in loop until error, all bytes have been read, or timeout
        while ((bytesRead < RcvLen) && !exitFlag)
        {
            // determine if call should block if descriptor not ready
            if (Timeout != k_InfiniteTimeout)
            {
                readReady = RecvReady(Timeout);
            }

            if (readReady)
            {
                rv = RecvData(pBuf, RcvLen, bytesRead, Timeout);

                // closed descriptor or error case... force exit from while loop
                if (rv <= 0)
                {
                    if (retries == 0)
                    {
                        // force exit condition
                        exitFlag = true;
                    }
                    else
                    {
                        --retries;
                        cp::MilliSleep(m_RetryDelay);
                    }
                }
                else
                {
                    // if m_FullRead is true, don't exit with less than requested number of bytes
                    exitFlag = exitFlag || !m_FullRead;
                    bytesRead += rv;
                }
            }
            else
            {
                exitFlag = true;
            }
        }

        rv = bytesRead;
    }

    return rv;
}


// returns true if device ready for send
bool IoDev::SendReady(uint32_t Timeout)
{
    bool rv = true;
    fd_set writeFds;
    timeval tv;

    // watch the write file descriptor for readiness to write
    FD_ZERO(&writeFds);
    FD_SET(m_dWrite, &writeFds);

    // translate milliseconds into seconds + microseconds
    tv.tv_sec = Timeout / 1000;
    tv.tv_usec = (Timeout - (tv.tv_sec * 1000)) * 1000;

    // if descriptor can't be written, don't attempt a write
    if (select(m_dWrite + 1, NULL, &writeFds, NULL, &tv) <= 0)
    {
        rv = false;
    }

    return rv;
}


// returns true if device ready for receive
bool IoDev::RecvReady(uint32_t Timeout)
{
    bool rv = true;
    fd_set readFds;
    timeval tv;

    // watch the read file descriptor for readiness to read
    FD_ZERO(&readFds);
    FD_SET(m_dRead, &readFds);

    // translate milliseconds into seconds + microseconds
    tv.tv_sec = Timeout / 1000;
    tv.tv_usec = (Timeout - (tv.tv_sec * 1000)) * 1000;

    // if data is not available then don't attempt a read
    if (select(m_dRead + 1, &readFds, NULL, NULL, &tv) <= 0)
    {
        rv = false;
    }

    return rv;
}

}   // namespace cp
