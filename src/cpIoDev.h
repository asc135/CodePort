// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIoDev.h
//
//  Description:    I/O Base Class.
//
//  Platform:       common
//
//  History:
//  2011-04-30  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Added timeout parameter to SendData() and recvData().
//  2013-05-08  asc Added SndLen and RcvLen to Buffer versions of Send() and Recv().
// ----------------------------------------------------------------------------

#ifndef CP_IODEV_H
#define CP_IODEV_H

#include "cpBase.h"
#include "cpIoDev_I.h"

namespace cp
{

class Buffer;

// ----------------------------------------------------------------------------

class IoDev : public Base
{
public:
    // constructor
    IoDev(String const &Name);

    // destructor
    virtual ~IoDev();

    // input/output operations
    int Send(Buffer const &Buf, size_t SndLen, uint32_t Timeout = k_InfiniteTimeout);
    int Recv(Buffer       &Buf, size_t RcvLen, uint32_t Timeout = k_InfiniteTimeout);
    int Send(char const  *pBuf, size_t SndLen, uint32_t Timeout = k_InfiniteTimeout);
    int Recv(char        *pBuf, size_t RcvLen, uint32_t Timeout = k_InfiniteTimeout);

    // manipulators
    void FullRead(bool Val)       { m_FullRead = Val;   }   // set the full read flag
    void Retries(uint8_t Val)     { m_Retries = Val;    }   // set the numer of I/O retry attempts
    void RetryDelay(uint16_t Val) { m_RetryDelay = Val; }   // set the delay before a retry
    virtual void Flush()  {}                                // flush device I/O buffers
    virtual void Cancel() {}                                // cancel pended I/O operations

protected:
    virtual bool SendReady(uint32_t Timeout);               // returns true if device ready for send
    virtual bool RecvReady(uint32_t Timeout);               // returns true if device ready for receive
    virtual int SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout) = 0;
    virtual int RecvData(char       *pBuf, size_t RcvLen, size_t BytesRead,    uint32_t Timeout) = 0;

    bool                m_FullRead;                         // don't return with less than requested bytes when true
    uint8_t             m_Retries;                          // number of I/O retry attempts
    uint16_t            m_RetryDelay;                       // delay before a retry
    desc_t              m_dWrite;                           // the write descriptor
    desc_t              m_dRead;                            // the read descriptor
};

}   // namespace cp

#endif  // CP_IODEV_H
