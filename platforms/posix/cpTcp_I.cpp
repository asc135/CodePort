// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpTcp_I.cpp
//
//  Description:    TCP Communications Facility.
//
//  Platform:       posix
//
//  History:
//  2013-04-01  asc Creation.
//  2013-09-05  asc Added socket options function.
//  2014-12-03  asc Added OnOpen() and OnClose() methods.  Added FD_CLOEXEC option.
// ----------------------------------------------------------------------------

// Use fcntl to set FD option so that FD isn't passed to child processes.
#include <fcntl.h>

#include "cpTcp.h"
#include "cpBuffer.h"

namespace cp
{

desc_t const k_InvalidSocket = k_InvalidDescriptor;


void Tcp::SetOptions(uint32_t Options)
{
    if (Options & so_ReuseAddr)
    {
        int optval = 1;

        // set SO_REUSEADDR on a socket to true (1)
        setsockopt(m_dWrite, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    }

    if (Options & so_TcpNoDelay)
    {
        int optval = 1;

        // set TCP_NODELAY on a socket to true (1)
        setsockopt(m_dWrite, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
    }

}


void Tcp::CloseSocket(desc_t Socket)
{
    if (Socket != k_InvalidSocket)
    {
        OnClose();

        if (m_AutoShutdown)
        {
            Shutdown(2);
        }

        close(Socket);
    }
}


bool Tcp::Init()
{
    return true;
}


bool Tcp::Cleanup()
{
    return true;
}


// settings to apply upon socket open
void Tcp::OnOpen()
{
    // Use fcntl to set FD option so that FD isn't passed to child processes.
    fcntl(m_dWrite, F_SETFD, fcntl(m_dWrite, F_GETFD) | FD_CLOEXEC);
}


// settings to apply upon socket close
void Tcp::OnClose()
{
}

}   // namespace cp
