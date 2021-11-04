// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpUdp.cpp
//
//  Description:    UDP Communications Facility.
//
//  Platform:       common
//
//  History:
//  2011-04-29  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-15  asc Redesigned client interface.
//  2012-12-11  asc Moved platform sensitive functions into platform module.
//  2013-03-31  asc Combined read/write sockets into one socket.
//  2013-04-01  asc Added constructor parameter to make bind optional.
//  2014-12-03  asc Added OnOpen() and OnClose() methods.
// ----------------------------------------------------------------------------

#include "cpUdp.h"
#include "cpUtil.h"
#include "cpBuffer.h"

namespace cp
{

Udp::Udp(String const &Name, uint32_t RecvAddr, uint16_t RecvPort, bool Bind) :
    IoDev(Name)
{
    // perform any platform specific network startup initialization
    m_Valid = Init();

    // create a socket for sending and receiving datagrams
    if (m_Valid)
    {
        m_dWrite = (desc_t)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        m_Valid = (m_dWrite != (desc_t)k_InvalidDescriptor);

        if (m_Valid)
        {
            m_dRead = m_dWrite;
            OnOpen();
        }
        else
        {
            LogErr << "Udp::Udp(): Error occurred while creating socket: "
                   << NameGet() << std::endl;
        }
    }
    else
    {
        LogErr << "Udp::Udp(): Error occurred while initializing socket library: "
               << NameGet() << std::endl;
    }

    // configure the address structures and bind receive socket
    if (m_Valid)
    {
        // initialize the send socket address
        memset(&m_DestAddr, 0, sizeof(m_DestAddr));
        m_DestAddr.sin_family      = AF_INET;
        m_DestAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        m_DestAddr.sin_port        = 0;

        if (Bind)
        {
            sockaddr_in recvAddr;

            // initialize the recv socket address
            memset(&recvAddr, 0, sizeof(recvAddr));
            recvAddr.sin_family      = AF_INET;
            recvAddr.sin_addr.s_addr = htonl(RecvAddr);
            recvAddr.sin_port        = htons(RecvPort);

            // bind the m_dRead to the recvAddr
            m_Valid = (bind((socket_t)m_dRead, (sockaddr *)&recvAddr, sizeof(recvAddr)) != k_Error);

            if (!m_Valid)
            {
                LogErr << "Udp::Udp(): Bind error on socket: "
                       << NameGet() << std::endl;
            }
        }
    }
}


Udp::~Udp()
{
    // perform any platform specific network shutdown cleanup
    OnClose();
    Cleanup();
}


// get port bound to receive socket
uint16_t Udp::BindPortGet()
{
    sockaddr_in addr_in;
    uint16_t port = 0;

    if (BindGet(addr_in))
    {
        // convert port to host order
        port = ntohs(addr_in.sin_port);
    }

    return port;
}


// get address bound to receive socket
uint32_t Udp::BindAddrGet()
{
    sockaddr_in addr_in;
    uint32_t addr = 0;

    if (BindGet(addr_in))
    {
        // convert IP address to host order
        addr = ntohl(addr_in.sin_addr.s_addr);
    }

    return addr;
}


// get address bound to receive socket
String Udp::BindAddrGetStr()
{
    return cp::Ipv4ToStr(BindAddrGet());
}


// set the destination port
void Udp::DestPortSet(uint16_t DestPort)
{
    m_DestAddr.sin_port = htons(DestPort);
}


// set the destination address
void Udp::DestAddrSet(uint32_t DestAddr)
{
    m_DestAddr.sin_addr.s_addr = htonl(DestAddr);
}


// set the destination address
void Udp::DestAddrSet(String const &DestAddr)
{
    DestAddrSet(cp::StrToIpv4(DestAddr));
}


// use instead of Recv() if client address and port are needed
int Udp::ReadDatagram(char *pBuf, size_t BufSize, uint32_t &Addr, uint16_t &Port, uint32_t Timeout)
{
    sockaddr_in srcAddr;
    socklen_t len = sizeof(srcAddr);
    int numRcvd = 0;

    if (pBuf == NULL)
    {
        return k_Error;
    }

    if (RecvReady(Timeout))
    {
        memset(&srcAddr, 0, sizeof(srcAddr));
        numRcvd = recvfrom((socket_t)m_dRead, pBuf, BufSize, 0, (sockaddr *)&srcAddr, &len);
        Addr = ntohl(srcAddr.sin_addr.s_addr);
        Port = ntohs(srcAddr.sin_port);
    }

    return numRcvd;
}


// ----------------------------------------------------------------------------
//  Function Name:  SendData
//
//  Description:    sends data to the write descriptor
//
//  Inputs:         pBuf - pointer to source buffer
//                  SndLen - number of bytes to send
//                  BytesWritten - number of bytes previously written
//                  Timeout - I/O timeout
//
//  Outputs:        none
//
//  Returns:        number of characters read or < 0 if error
// ------------------------------------------------------------------------- */
int Udp::SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout)
{
    int rv = k_Error;

    (void)Timeout;

    if ((SndLen != 0) || (SndLen <= k_UdpMaxMsgLen))
    {
        rv = sendto((socket_t)m_dWrite, pBuf + BytesWritten, SndLen - BytesWritten, 0, (sockaddr *)&m_DestAddr, sizeof(m_DestAddr));
    }
    else
    {
        LogErr << "Udp::SendData(): Msg length is not valid: "
               << NameGet() << std::endl;
    }

    return rv;
}


// ----------------------------------------------------------------------------
//  Function Name:  RecvData
//
//  Description:    receives data from the read descriptor
//
//  Inputs:         pBuf - pointer to target buffer
//                  RcvLen - number of bytes to receive
//                  BytesRead - number of bytes previously read
//                  Timeout - I/O timeout
//
//  Outputs:        none
//
//  Returns:        number of characters read or < 0 if error
// ------------------------------------------------------------------------- */
int Udp::RecvData(char       *pBuf, size_t RcvLen, size_t BytesRead, uint32_t Timeout)
{
    (void)Timeout;
    return recvfrom((socket_t)m_dRead, pBuf + BytesRead, RcvLen - BytesRead, 0, NULL, NULL);
}


// get bound socket info
bool Udp::BindGet(sockaddr_in &Addr)
{
    socklen_t len = sizeof(Addr);
    return (getsockname((socket_t)m_dRead, (sockaddr *)&Addr, &len) == 0);
}

}   // namespace cp
