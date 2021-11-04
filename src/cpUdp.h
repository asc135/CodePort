// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpUdp.h
//
//  Description:    UDP Communications Facility.
//
//  Platform:       common
//
//  History:
//  2011-04-29  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-15  asc Redesigned client interface.
//  2012-12-11  asc Added timeout parameter to SendData() and recvData().
//  2013-04-01  asc Added constructor parameter to make bind optional.
//  2014-12-03  asc Added OnOpen() and OnClose() methods.
// ----------------------------------------------------------------------------

#ifndef CP_UDP_H
#define CP_UDP_H

#include "cpIoDev.h"
#include "cpUdp_I.h"

namespace cp
{

class Udp : public IoDev
{
public:
    // constructor
    Udp(String const &Name, uint32_t RecvAddr = ntohl(INADDR_ANY), uint16_t RecvPort = 0, bool Bind = true);

    // destructor
    virtual ~Udp();

    // socket related methods
    uint16_t BindPortGet();                                 // get port bound to receive socket
    uint32_t BindAddrGet();                                 // get address bound to receive socket
    String BindAddrGetStr();                                // get address bound to receive socket
    void DestPortSet(uint16_t DestPort);                    // set the destination port
    void DestAddrSet(uint32_t DestAddr);                    // set the destination address
    void DestAddrSet(String const &DestAddr);               // set the destination address

    // accessor methods
    int ReadDatagram(char *pBuf, size_t BufSize,            // use instead of Recv() if client address and port are needed
                     uint32_t &Addr, uint16_t &Port,
                     uint32_t Timeout = k_InfiniteTimeout);

protected:
    virtual int SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout);
    virtual int RecvData(char       *pBuf, size_t RcvLen, size_t BytesRead,    uint32_t Timeout);

private:
    bool Init();                                            // initialize stack
    bool Cleanup();                                         // cleanup stack
    bool BindGet(sockaddr_in &Addr);                        // get bound socket info
    void OnOpen();                                          // settings to apply upon socket open
    void OnClose();                                         // settings to apply upon socket close

    sockaddr_in         m_DestAddr;                         // address of destination
};

}   // namespace cp

#endif  // CP_UDP_H
