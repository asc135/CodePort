// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIoDev_I.h
//
//  Description:    I/O Base Class.
//
//  Platform:       mswin
//
//  History:
//  2012-12-10  asc Creation.
// ----------------------------------------------------------------------------

#ifndef CP_IODEV_I_H
#define CP_IODEV_I_H

namespace cp
{

typedef HANDLE desc_t;

// callback routine for asynchronous I/O
void WINAPI IoCallback(DWORD ErrorCode, DWORD BytesTransferred, LPOVERLAPPED pOlio);

// used to return results from I/O APC
class IoResults
{
public:
    // constructor
    IoResults(HANDLE Hdl);

    // destructor
    ~IoResults();

    bool WaitComplete(uint32_t Timeout);

    // member data
    OVERLAPPED  m_Olio;                 // pass-through pointer, must be first member of structure
    HANDLE      m_Handle;
    DWORD       m_ErrorCode;
    DWORD       m_BytesTransferred;
};

}   // namespace cp

#endif  // CP_IODEV_I_H
