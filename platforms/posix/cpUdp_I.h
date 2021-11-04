// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpUdp_I.h
//
//  Description:    UDP Communications Facility.
//
//  Platform:       posix
//
//  History:
//  2011-04-29  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_UDP_I_H
#define CP_UDP_I_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>

namespace cp
{

typedef int socket_t;

}   // namespace cp

#endif  // CP_UDP_I_H
