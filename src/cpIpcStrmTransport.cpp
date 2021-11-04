// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcStrmTransport.cpp
//
//  Description:    IPC stream transport.
//
//  Platform:       common
//
//  History:
//  2013-04-06  asc Creation.
// ----------------------------------------------------------------------------

#include "cpIpcStrmTransport.h"
#include "cpIpcSegment.h"
#include "cpIoDev.h"
#include "cpUtil.h"

namespace cp
{

// constructor
IpcStrmTransport::IpcStrmTransport(String const &Name) :
    IpcTransport(Name)
{
}


// destructor
IpcStrmTransport::~IpcStrmTransport()
{
}


bool IpcStrmTransport::Send(IpcSegment *pSeg, uint32_t Timeout)
{
    bool rv = false;
    IoDev *sendDevice = NULL;
    int sent = 0;
    cp::Buffer header(k_IpcStrmHeaderLen);

    if (pSeg == NULL)
    {
        return false;
    }

    // generate the stream header
    rv = EncodeHeader(header, OpCode_IpcMsg, pSeg->SegLen());

    // acquire a send device
    if (rv)
    {
        sendDevice = SendDeviceGet(pSeg->DstAddr());
        rv = (sendDevice != NULL);

        if (!rv)
        {
            LogErr << "IpcStrmTransport::Send(): Invalid send device: "
                   << NameGet() << std::endl;
        }
    }

    // send the stream header
    if (rv)
    {
        sent = sendDevice->Send(header, header, Timeout);
        rv = ((sent >= 0) && (sent == static_cast<ssize_t>(header.LenGet())));

        if (!rv)
        {
            LogErr << "IpcStrmTransport::Send(): Failed to send a stream header: "
                   << NameGet() << std::endl;
        }
    }

    // send the stream payload
    if (rv)
    {
        sent = sendDevice->Send(pSeg->Buf(), pSeg->Buf(), Timeout);
        rv = ((sent >= 0) && (sent == static_cast<ssize_t>(pSeg->SegLen())));

        if (!rv)
        {
            LogErr << "IpcStrmTransport::Send(): Failed to send a stream payload: "
                   << NameGet() << std::endl;
        }
    }

    // properly dispose of the segment
    SegmentDispose(pSeg);

    return rv;
}


bool IpcStrmTransport::Recv(IpcSegment *pSeg, uint32_t Timeout)
{
    bool rv = (m_PtrRecvDevice != NULL);
    int rcvd = 0;
    cp::Buffer header(k_IpcStrmHeaderLen);
    OpCodes opCode = OpCode_None;
    size_t rcvLen = 0;

    if (pSeg == NULL)
    {
        return false;
    }

    // make sure device is in full read mode
    if (rv)
    {
        m_PtrRecvDevice->FullRead(true);
    }

    // receive stream header
    if (rv)
    {
        rcvd = m_PtrRecvDevice->Recv(header, k_IpcStrmHeaderLen, Timeout);
        rv = (rcvd == k_IpcStrmHeaderLen);
    }
    else
    {
        LogErr << "IpcStrmTransport::Recv(): Invalid receive device: "
               << NameGet() << std::endl;
    }

    // decode the stream header
    if (rv)
    {
        rv = DecodeHeader(header, opCode, rcvLen);

        if (!rv)
        {
            LogErr << "IpcStrmTransport::Recv(): Invalid stream header: "
                   << NameGet() << std::endl;
        }
    }

    // determine if it is an IPC message
    if (rv)
    {
        // validate header values
        rv = ((opCode == OpCode_IpcMsg) &&
              (rcvLen > 0) &&
              (rcvLen <= pSeg->Buf().Size()));

        // not an IPC message so just read and discard
        if (!rv)
        {
            cp::Buffer tmpBuf;
            rcvd = m_PtrRecvDevice->Recv(tmpBuf, rcvLen, Timeout);
            rv = (rcvd != static_cast<ssize_t>(rcvLen));

            if (!rv)
            {
                LogErr << "IpcStrmTransport::Recv(): Failed to read non-IPC message: "
                       << NameGet() << std::endl;
            }
        }
    }

    // receive stream payload
    if (rv)
    {
        rv = false;

        rcvd = m_PtrRecvDevice->Recv(pSeg->Buf(), rcvLen, Timeout);

        if (rcvd > 0)
        {
            // determine if in-band or out of band message
            if ((rcvd >= IpcSegment::seg_Data) && (rcvd <= IpcSegment::seg_MaxLen))
            {
                // in-band data message
                rv = true;
            }
            else
            {
                // something else, discard it
                pSeg->Clear();
            }
        }
    }

    return rv;
}


bool IpcStrmTransport::EncodeHeader(cp::Buffer &Header, OpCodes OpCode, size_t Length)
{
    uint8_t *ptr = NULL;

    if (Header.Resize(k_IpcStrmHeaderLen))
    {
        ptr = Header.u_str();

        // insert the header id
        cp::WriteUint16B(k_IpcStrmHeaderId, ptr);
        ++ptr;
        ++ptr;

        // insert the opcode
        cp::WriteUint16B(OpCode, ptr);
        ++ptr;
        ++ptr;

        // insert the payload length
        cp::WriteUint32B(Length, ptr);
        Header.LenSet(k_IpcStrmHeaderLen);
    }

    return (ptr != NULL);
}


bool IpcStrmTransport::DecodeHeader(cp::Buffer const &Header, OpCodes &OpCode, size_t &Length)
{
    bool rv = false;
    uint8_t const *ptr = NULL;

    if (Header.LenGet() >= k_IpcStrmHeaderLen)
    {
        ptr = Header.u_str();

        // confirm the header id
        rv = (cp::ReadUint16B(ptr) == k_IpcStrmHeaderId);
        ++ptr;
        ++ptr;

        if (rv)
        {
            // extract the opcode
            OpCode = static_cast<OpCodes>(cp::ReadUint16B(ptr));
            ++ptr;
            ++ptr;

            // extract the payload length
            Length = cp::ReadUint32B(ptr);
        }
    }

    return rv;
}

}   // namespace cp
