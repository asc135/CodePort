// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpTcp.cpp
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
//  2023-01-20  asc Replaced AF_INET with PF_INET in socket() call.
// ----------------------------------------------------------------------------

#include "cpTcp.h"
#include "cpUtil.h"
#include "cpBuffer.h"

namespace cp
{

Tcp::Tcp(String const &Name, uint32_t RecvAddr, uint16_t RecvPort, int ListenQueue, uint32_t Options) :
    IoDev(Name),
    m_AutoShutdown(false)
{
    // perform any platform specific network startup initialization
    m_Valid = Init();

    // create a socket for sending and receiving stream data
    if (m_Valid)
    {
        m_dWrite = (desc_t)socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        m_Valid = (m_dWrite != k_InvalidSocket);

        if (m_Valid)
        {
            m_dRead = m_dWrite;

            OnOpen();
            SetOptions(Options);
        }
        else
        {
            LogErr << "Tcp::Tcp(): Error occurred while creating socket: "
                   << NameGet() << std::endl;
        }
    }
    else
    {
        LogErr << "Tcp::Tcp(): Error occurred while initializing socket library: "
               << NameGet() << std::endl;
    }

    // configure the address structures and bind receive
    // socket if this instance is going to be a server
    if (m_Valid)
    {
        // initialize the send socket address
        memset(&m_DestAddr, 0, sizeof(m_DestAddr));
        m_DestAddr.sin_family      = AF_INET;
        m_DestAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        m_DestAddr.sin_port        = 0;

        // bind receive socket and set it as passive if this is to be a server
        if (ListenQueue > 0)
        {
            sockaddr_in recvAddr;

            // initialize the recv socket address
            memset(&recvAddr, 0, sizeof(recvAddr));
            recvAddr.sin_family      = AF_INET;
            recvAddr.sin_addr.s_addr = htonl(RecvAddr);
            recvAddr.sin_port        = htons(RecvPort);

            // bind the m_dRead socket to the recvAddr
            m_Valid = (bind((socket_t)m_dRead, (sockaddr *)&recvAddr, sizeof(recvAddr)) != k_Error);

            if (m_Valid)
            {
                // set it as a passive socket accepting connection requests
                m_Valid = (listen((socket_t)m_dRead, ListenQueue) != k_Error);

                if (!m_Valid)
                {
                    LogErr << "Tcp::Tcp(): Listen error on socket: "
                           << NameGet() << std::endl;
                }
            }
            else
            {
                LogErr << "Tcp::Tcp(): Bind error on socket: "
                       << NameGet() << std::endl;
            }
        }
    }
}


Tcp::Tcp(String const &Name, desc_t Read, desc_t Write) :
    IoDev(Name),
    m_AutoShutdown(false)
{
    m_dRead = Read;
    m_dWrite = Write;
    m_Valid = Init();
}


Tcp::~Tcp()
{
    // need to close them here before the IoDev destructor
    // to allow Cleanup() to work properly on some platforms
    CloseSocket(m_dRead);
    m_dRead = k_InvalidSocket;
    m_dWrite = m_dRead;

    // perform any platform specific network shutdown cleanup
    Cleanup();
}


// get port bound to receive socket
uint16_t Tcp::BindPortGet()
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
uint32_t Tcp::BindAddrGet()
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
String Tcp::BindAddrGetStr()
{
    return cp::Ipv4ToStr(BindAddrGet());
}


// set the destination port
void Tcp::DestPortSet(uint16_t DestPort)
{
    m_DestAddr.sin_port = htons(DestPort);
}


// set the destination address
void Tcp::DestAddrSet(uint32_t DestAddr)
{
    m_DestAddr.sin_addr.s_addr = htonl(DestAddr);
}


// set the destination address
void Tcp::DestAddrSet(String const &DestAddr)
{
    DestAddrSet(cp::StrToIpv4(DestAddr));
}


// connect to a host
bool Tcp::Connect()
{
    bool rv = false;

    rv = (connect((socket_t)m_dWrite, (sockaddr *)&m_DestAddr, sizeof(m_DestAddr)) != k_Error);

    return rv;
}


bool Tcp::Shutdown(int32_t Type)
{
    return (shutdown((socket_t)m_dRead, Type) == 0);
}


// wait for an incoming connection
Tcp *Tcp::WaitForConnection(uint32_t &Addr, uint16_t &Port, uint32_t Timeout)
{
    bool status = false;
    sockaddr_in srcAddr;
    socklen_t len = sizeof(srcAddr);
    desc_t newSocket = k_InvalidSocket;
    Tcp *pTcp = NULL;

    if (RecvReady(Timeout))
    {
        memset(&srcAddr, 0, sizeof(srcAddr));
        newSocket = (desc_t)accept((socket_t)m_dRead, (sockaddr *)&srcAddr, &len);
        status = (newSocket != k_InvalidSocket);
    }

    if (status)
    {
        Addr = ntohl(srcAddr.sin_addr.s_addr);
        Port = ntohs(srcAddr.sin_port);
        pTcp = new (CP_NEW) Tcp(NameGet() + " - port: " + UintToStr(Port), newSocket, newSocket);

        // close new socket if new instance construction fails
        if (pTcp == NULL)
        {
            CloseSocket(newSocket);
        }
    }

    return pTcp;
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
int Tcp::SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout)
{
    int rv = k_Error;

    (void)Timeout;

    if (SndLen != 0)
    {
        rv = send((socket_t)m_dWrite, pBuf + BytesWritten, SndLen - BytesWritten, 0);

        if (rv <= 0)
        {
            CloseSocket(m_dWrite);

            m_dWrite = k_InvalidSocket;
            m_dRead = m_dWrite;
        }
    }
    else
    {
        LogErr << "Tcp::SendData(): Msg length is not valid: "
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
int Tcp::RecvData(char       *pBuf, size_t RcvLen, size_t BytesRead, uint32_t Timeout)
{
    int rv = k_Error;

    (void)Timeout;

    if (RcvLen != 0)
    {
        rv = recv((socket_t)m_dRead, pBuf + BytesRead, RcvLen - BytesRead, 0);

        if (rv <= 0)
        {
            CloseSocket(m_dRead);

            m_dRead = k_InvalidSocket;
            m_dWrite = m_dRead;
        }
    }
    else
    {
        LogErr << "Tcp::RecvData(): Receive length is not valid: "
               << NameGet() << std::endl;
    }

    return rv;
}


// get bound socket info
bool Tcp::BindGet(sockaddr_in &Addr)
{
    socklen_t len = sizeof(Addr);
    return (getsockname((socket_t)m_dRead, (sockaddr *)&Addr, &len) == 0);
}

}   // namespace cp
