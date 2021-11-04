// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpUdp_I.cpp
//
//  Description:    UDP Communications Facility.
//
//  Platform:       posix
//
//  History:
//  2011-04-29  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Moved platform dependent functions into platform module.
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
    return true;
}


bool Udp::Cleanup()
{
    return true;
}


// settings to apply upon socket open
void Udp::OnOpen()
{
    // Use fcntl to set FD option so that FD isn't passed to child processes.
    fcntl(m_dWrite, F_SETFD, fcntl(m_dWrite, F_GETFD) | FD_CLOEXEC);
}


// settings to apply upon socket close
void Udp::OnClose()
{
}

}   // namespace cp
