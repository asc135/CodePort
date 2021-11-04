// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpTcp_I.h
//
//  Description:    TCP Communications Facility.
//
//  Platform:       posix
//
//  History:
//  2013-04-01  asc Creation.
// ----------------------------------------------------------------------------

#ifndef CP_TCP_I_H
#define CP_TCP_I_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>

namespace cp
{

typedef int socket_t;

extern desc_t const k_InvalidSocket;

}   // namespace cp

#endif  // CP_TCP_I_H
