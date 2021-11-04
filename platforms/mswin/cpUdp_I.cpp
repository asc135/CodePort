// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpUdp_I.cpp
//
//  Description:    UDP Communications Facility.
//
//  Platform:       mswin
//
//  History:
//  2012-12-10  asc Creation.
//  2013-04-03  asc Moved Winsock initialization and cleanup into platform file.
//  2014-12-03  asc Added OnOpen() and OnClose() methods.  Added FD_CLOEXEC option.
// ----------------------------------------------------------------------------

// Use fcntl to set FD option so that FD isn't passed to child processes.
#include <fcntl.h>

#include "cpUdp.h"
#include "cpBuffer.h"

namespace cp
{

bool Udp::Init()
{
    return WinSockGet();
}


bool Udp::Cleanup()
{
    return WinSockPut();
}


// settings to apply upon socket open
void Udp::OnOpen()
{
    #if 0
    // Use fcntl to set FD option so that FD isn't passed to child processes.
    fcntl(m_dWrite, F_SETFD, fcntl(m_dWrite, F_GETFD) | FD_CLOEXEC);
    #endif
}


// settings to apply upon socket close
void Udp::OnClose()
{
}

}   // namespace cp
