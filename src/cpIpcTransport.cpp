// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcTransport.cpp
//
//  Description:    IPC transport base class.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
//  2013-01-31  asc Changed validation mechanism in SetDevices().
//  2013-03-13  asc Added device map to support direct node routing.
//  2013-03-22  asc Added support for accumulator timeout handling.
//  2013-04-19  asc Removed RTL support mechanisms.
//  2013-04-24  asc Added ReleaseThread() method.
// ----------------------------------------------------------------------------

#include "cpIpcTransport.h"
#include "cpIpcSegment.h"
#include "cpQueue.h"
#include "cpUtil.h"

namespace cp
{

// constructor
IpcTransport::IpcTransport(String const &Name) :
    Base(Name),
    m_PtrSendDevice(NULL),
    m_PtrRecvDevice(NULL),
    m_MutexSendDevs("Send Device Map Mutex")
{
}


// destructor
IpcTransport::~IpcTransport()
{
    if (m_PtrSendDevice)
    {
        delete m_PtrSendDevice;
    }

    if (m_PtrRecvDevice && (m_PtrSendDevice != m_PtrRecvDevice))
    {
        delete m_PtrRecvDevice;
    }
}


// return true if comm bus connection is functioning
bool IpcTransport::Validate(uint32_t NodeAddr)
{
    bool rv = false;
    IpcSegment *pSeg = NULL;
    IpcSegment seg;
    uint32_t time = Time32();

    // create a segment
    pSeg = new (CP_NEW) IpcSegment;

    if (pSeg)
    {
        // set up for a loopback transmission
        pSeg->SrcAddr(NodeAddr);
        pSeg->DstAddr(NodeAddr);
        pSeg->DataSet(time);

        // send the message
        rv = Send(pSeg);
    }
    else
    {
        LogErr << "IpcTransport::Validate(): Failed to create an IPC segment." << std::endl;
    }

    if (rv)
    {
        // wait for response
        rv = Recv(&seg, k_ReceiveTimeout);
    }

    // check if result is correct
    rv = rv && (seg.DataGet() == time);

    return rv;
}


bool IpcTransport::Send(IpcSegment *pSeg, uint32_t Timeout)
{
    bool rv = false;
    IoDev *sendDevice = NULL;
    int sent = 0;

    if (pSeg == NULL)
    {
        return false;
    }

    sendDevice = SendDeviceGet(pSeg->DstAddr());

    if (sendDevice != NULL)
    {
        sent = sendDevice->Send(pSeg->Buf(), pSeg->Buf(), Timeout);

        rv = ((sent >= 0) && (sent == static_cast<ssize_t>(pSeg->SegLen())));
    }
    else
    {
        LogErr << "IpcTransport::Send(): Invalid send device: "
               << NameGet() << std::endl;
    }

    if (!rv)
    {
        LogErr << "IpcTransport::Send(): Failed to send a message: "
               << NameGet() << std::endl;
    }

    // properly dispose of the segment
    SegmentDispose(pSeg);

    return rv;
}


bool IpcTransport::Recv(IpcSegment *pSeg, uint32_t Timeout)
{
    bool rv = (m_PtrRecvDevice != NULL);
    int rcvd = 0;

    if (pSeg == NULL)
    {
        return false;
    }

    if (rv)
    {
        rv = false;

        rcvd = m_PtrRecvDevice->Recv(pSeg->Buf(), pSeg->Buf(), Timeout);

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
    else
    {
        LogErr << "IpcTransport::Recv(): Invalid receive device: "
               << NameGet() << std::endl;
    }

    return rv;
}


// install default send and receive devices
bool IpcTransport::DevicesSet(IoDev *pSendDevice, IoDev *pRecvDevice, uint32_t ValidateNode)
{
    bool rv = pSendDevice && pRecvDevice;

    if (rv)
    {
        // delete any existing send device
        if (m_PtrSendDevice)
        {
            delete m_PtrSendDevice;
        }

        // delete any existing receive device
        if (m_PtrRecvDevice)
        {
            delete m_PtrRecvDevice;
        }

        // assign new devices
        m_PtrSendDevice = pSendDevice;
        m_PtrRecvDevice = pRecvDevice;

        // set the validity flag
        if (ValidateNode)
        {
            // validate if a node number is provided
            m_Valid = Validate(ValidateNode);
        }
        else
        {
            // otherwise, just set validity flag to true (typically for point-to-point connections)
            m_Valid = true;
        }
    }

    return rv;
}


// associate a node address with a direct send device
bool IpcTransport::SendDeviceAdd(uint32_t NodeAddr, String const &DevName)
{
    IoDev *pSend = new (CP_NEW) cp::Queue(DevName);
    bool rv = (pSend != NULL);

    if (rv)
    {
        // delete any existing device instance
        SendDeviceDel(NodeAddr);

        m_MutexSendDevs.Lock();

        // add new device
        m_SendDevices[NodeAddr] = pSend;

        m_MutexSendDevs.Unlock();
    }

    return rv;
}


// disassociate a node address from a direct send device
bool IpcTransport::SendDeviceDel(uint32_t NodeAddr)
{
    bool rv = false;
    DeviceMap_t::iterator i;

    m_MutexSendDevs.Lock();

    // locate any existing map entry
    i = m_SendDevices.find(NodeAddr);

    // delete any existing device instance
    if (i != m_SendDevices.end())
    {
        if (i->second)
        {
            delete i->second;
        }

        m_SendDevices.erase(i);
        rv = true;
    }

    m_MutexSendDevs.Unlock();

    return rv;
}


// properly dispose of a segment after transmission
void IpcTransport::SegmentDispose(IpcSegment *pSeg)
{
    // in standard transport, we don't keep a history so delete the segment
    if (pSeg)
    {
        delete pSeg;
    }
}


// return appropriate send device
IoDev *IpcTransport::SendDeviceGet(uint32_t NodeAddr)
{
    IoDev *rv = NULL;
    DeviceMap_t::iterator i;

    m_MutexSendDevs.Lock();

    // locate any existing map entry
    i = m_SendDevices.find(NodeAddr);

    // return the found device otherwise
    // return the device to the router
    if (i != m_SendDevices.end())
    {
        rv = i->second;
    }
    else
    {
        rv = m_PtrSendDevice;
    }

    m_MutexSendDevs.Unlock();

    return rv;
}


// release the receive thread
void IpcTransport::ReleaseThread()
{
    if (m_PtrRecvDevice)
    {
        // send a four byte message which should be ignored by receive thread
        Buffer buf(sizeof(uint32_t));
        buf.LenSet(sizeof(uint32_t));
        m_PtrRecvDevice->Send(buf, buf, k_TransmitTimeout);
    }
}

}   // namespace cp
