// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpUdp_I.h
//
//  Description:    UDP Communications Facility.
//
//  Platform:       mswin
//
//  History:
//  2012-12-10  asc Creation.
// ----------------------------------------------------------------------------

#ifndef CP_UDP_I_H
#define CP_UDP_I_H

#include <winsock2.h>
#include <ws2tcpip.h>

namespace cp
{

typedef SOCKET socket_t;

}   // namespace cp

#endif  // CP_UDP_I_H
