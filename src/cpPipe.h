// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpPipe.h
//
//  Description:    Data Pipe Facility.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Added timeout parameter to SendData() and recvData().
// ----------------------------------------------------------------------------

#ifndef CP_PIPE_H
#define CP_PIPE_H

#include "cpIoDev.h"
#include "cpPipe_I.h"

namespace cp
{

class Pipe : public IoDev
{
public:
    // constructor
    Pipe(String const &Name = "Anonymous Pipe",
         size_t BufSize = k_DefaultIoBufSize);

    // destructor
    virtual ~Pipe();

    // manipulators
    void Complete();                                        // terminate the data stream

protected:
    virtual int SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout);
    virtual int RecvData(char       *pBuf, size_t RcvLen, size_t BytesRead,    uint32_t Timeout);

private:
    Pipe_t              m_Pipe;                             // native data storage
};

}   // namespace cp

#endif  // CP_PIPE_H
