// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpNamedPipe.h
//
//  Description:    Named Pipe Facility.
//
//  Platform:       common
//
//  History:
//  2011-06-26  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Added timeout parameter to SendData() and recvData().
// ----------------------------------------------------------------------------

#ifndef CP_NAMEDPIPE_H
#define CP_NAMEDPIPE_H

#include "cpIoDev.h"
#include "cpNamedPipe_I.h"

namespace cp
{

class NamedPipe : public IoDev
{
public:
    // enumerations
    enum PipeDir { k_Read, k_Write };

    // constructor
    NamedPipe(String const &Name,
             String const &Path,
             PipeDir Direction,
             bool Create = false,
             size_t BufSize = k_DefaultIoBufSize);

    // destructor
    virtual ~NamedPipe();

    // manipulators
    void Complete();                                        // terminate the data stream

protected:
    virtual int SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout);
    virtual int RecvData(char       *pBuf, size_t RcvLen, size_t BytesRead,    uint32_t Timeout);

private:
    bool CreateDevice();                                    // create the device file

    bool                m_Cleanup;                          // object that creates device needs to delete it
    NamedPipe_t         m_Pipe;                             // native data storage
    String              m_SysName;                          // native instance name
};

}   // namespace cp

#endif  // CP_NAMEDPIPE_H
