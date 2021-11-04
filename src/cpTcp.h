// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpTcp.h
//
//  Description:    TCP Communications Facility.
//
//  Platform:       common
//
//  History:
//  2013-04-01  asc Creation.
//  2013-06-14  asc Added Shutdown() method and adjusted close detect logic.
//  2013-09-04  asc Added socket options enumeration and constructor param.
//  2014-12-03  asc Added OnOpen() and OnClose() methods.
// ----------------------------------------------------------------------------

#ifndef CP_TCP_H
#define CP_TCP_H

#include "cpIoDev.h"
#include "cpTcp_I.h"

namespace cp
{

class Tcp : public IoDev
{
public:
    // local enumerations
    enum SocketOptions
    {
        so_ReuseAddr = 1,
        so_TcpNoDelay = 2
    };

    // constructors
    Tcp(String const &Name, uint32_t RecvAddr = ntohl(INADDR_ANY), uint16_t RecvPort = 0, int ListenQueue = 0, uint32_t Options = 0);
    Tcp(String const &Name, desc_t Read, desc_t Write);

    // destructor
    virtual ~Tcp();

    // socket related methods
    uint16_t BindPortGet();                                 // get port bound to receive socket
    uint32_t BindAddrGet();                                 // get address bound to receive socket
    String BindAddrGetStr();                                // get address bound to receive socket
    void DestPortSet(uint16_t DestPort);                    // set the destination port
    void DestAddrSet(uint32_t DestAddr);                    // set the destination address
    void DestAddrSet(String const &DestAddr);               // set the destination address

    // accessor methods
    bool Closed() { return (m_dRead == k_InvalidSocket); }
    bool Connect();
    void AutoShutdown(bool Auto) { m_AutoShutdown = Auto; }
    bool Shutdown(int32_t Type);
    Tcp *WaitForConnection(uint32_t &Addr, uint16_t &Port, uint32_t Timeout = k_InfiniteTimeout);

protected:
    virtual int SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout);
    virtual int RecvData(char       *pBuf, size_t RcvLen, size_t BytesRead,    uint32_t Timeout);

private:
    void SetOptions(uint32_t Options);                      // set socket options
    bool Init();                                            // initialize stack
    bool Cleanup();                                         // cleanup stack
    bool BindGet(sockaddr_in &Addr);                        // get bound socket info
    void CloseSocket(desc_t Socket);                        // close a socket
    void OnOpen();                                          // settings to apply upon socket open
    void OnClose();                                         // settings to apply upon socket close

    bool                m_AutoShutdown;                     // shutdown before close when true
    sockaddr_in         m_DestAddr;                         // address of destination
};

}   // namespace cp

#endif  // CP_TCP_H
