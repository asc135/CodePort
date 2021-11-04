// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpPipe_I.cpp
//
//  Description:    Data Pipe Facility.
//
//  Platform:       posix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Added timeout parameter to SendData() and recvData().
// ----------------------------------------------------------------------------

#include "cpPipe.h"

namespace cp
{

// constructor
Pipe::Pipe(String const &Name, size_t BufSize) :
    IoDev(Name)
{
    int fd[2];

    // posix pipes' size is implementation specific
    (void)BufSize;

    // create the pipe with some default settings. 0 is success, -1 indicates an error
    if (pipe(fd) == 0)
    {
        // assign the pipe descriptors
        m_dRead  = fd[0];
        m_dWrite = fd[1];

        m_Valid = true;
    }
    else
    {
        LogErr << "Pipe::Pipe(): Data pipe could not be created: "
               << NameGet() << std::endl;
    }
}


// destructor
Pipe::~Pipe()
{
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
    (void)Timeout;
    return write(m_dWrite, pBuf + BytesWritten, SndLen - BytesWritten);
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
int Pipe::RecvData(char       *pBuf, size_t RcvLen, size_t BytesRead, uint32_t Timeout)
{
    (void)Timeout;
    return read(m_dRead, pBuf + BytesRead, RcvLen - BytesRead);
}


// ----------------------------------------------------------------------------
//  Function Name:  Complete
//
//  Description:    completes the pipe's data transfer
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        none
// ------------------------------------------------------------------------- */
void Pipe::Complete()
{
    if (m_dWrite != k_InvalidDescriptor)
    {
        close(m_dWrite);
        m_dWrite = k_InvalidDescriptor;
    }
}

}   // namespace cp
