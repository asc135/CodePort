// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpNamedPipe_I.cpp
//
//  Description:    Named Pipe Facility.
//
//  Platform:       posix
//
//  History:
//  2011-06-26  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Added timeout parameter to SendData() and recvData().
// ----------------------------------------------------------------------------

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cpNamedPipe.h"

namespace cp
{

// constructor
NamedPipe::NamedPipe(String const &Name, String const &Path, PipeDir Direction, bool Create, size_t BufSize) :
    IoDev(Name),
    m_Cleanup(Create)
{
    // posix pipes' size is implementation specific
    (void)BufSize;

    // open the pipe with some default settings. 0 is success, -1 indicates an error
    if (Path.size() > 0)
    {
        // save the path name
        m_SysName = Path;

        // create the fifo file
        if (Create)
        {
            if (CreateDevice() == false)
            {
                LogErr << "NamedPipe::NamedPipe(): Failed to create device file: "
                       << NameGet() << std::endl;
            }
        }

        if (Direction == k_Read)
        {
            // open the FIFO file for read
            m_dRead = open(Path.c_str(), O_RDONLY);

            if (m_dRead != k_InvalidDescriptor)
            {
                m_Valid = true;
            }
            else
            {
                LogErr << "NamedPipe::NamedPipe(): Failed to open FIFO for read: "
                       << NameGet() << std::endl;
            }
        }
        else
        {

            // open the FIFO file for write
            m_dWrite = open(Path.c_str(), O_WRONLY);

            if (m_dWrite != k_InvalidDescriptor)
            {
                m_Valid = true;
            }
            else
            {
                LogErr << "NamedPipe::NamedPipe(): Failed to open FIFO for write: "
                       << NameGet() << std::endl;
            }
        }
    }
    else
    {
        LogErr << "NamedPipe::NamedPipe(): Empty string passed for FIFO path: "
               << NameGet() << std::endl;
    }

    // if not successful, clear the descriptors
    if (!m_Valid)
    {
        m_dWrite = k_InvalidDescriptor;
        m_dRead  = k_InvalidDescriptor;
    }
}


// destructor
NamedPipe::~NamedPipe()
{
    // make sure device descriptor is closed
    Complete();

    if (m_Cleanup)
    {
        unlink(m_SysName.c_str());
    }
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
    (void)Timeout;
    return ::write(m_dWrite, pBuf + BytesWritten, SndLen - BytesWritten);
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
    (void)Timeout;
    return ::read(m_dRead, pBuf + BytesRead, RcvLen - BytesRead);
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
    if (m_dRead != k_InvalidDescriptor)
    {
        close(m_dRead);
        m_dRead = k_InvalidDescriptor;
    }

    if (m_dWrite != k_InvalidDescriptor)
    {
        close(m_dWrite);
        m_dWrite = k_InvalidDescriptor;
    }
}


// create the fifo file
bool NamedPipe::CreateDevice()
{
    return (mknod(m_SysName.c_str(), S_IFIFO | 0644 , 0) == 0);
}

}   // namespace cp
