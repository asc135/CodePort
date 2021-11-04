// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcTransport.h
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

#ifndef CP_IPCTRANSPORT_H
#define CP_IPCTRANSPORT_H

#include <map>

#include "cpMutex.h"

namespace cp
{

// forward references
class IoDev;
class IpcSegment;
class IpcAccum;

// ----------------------------------------------------------------------------

class IpcTransport : public Base
{
public:
    // local types
    typedef std::map<uint32_t, IoDev *, std::less<uint32_t>, Alloc< std::pair<uint32_t const, IoDev *> > > DeviceMap_t;

    // constructor
    IpcTransport(String const &Name);

    // destructor
    virtual ~IpcTransport();

    // accessors
    bool Validate(uint32_t NodeAddr);

    // input/output functions
    virtual bool Send(IpcSegment *pSeg, uint32_t Timeout = cp::k_InfiniteTimeout);
    virtual bool Recv(IpcSegment *pSeg, uint32_t Timeout = cp::k_InfiniteTimeout);

    // manipulators
    bool DevicesSet(IoDev *pSendDevice, IoDev *pRecvDevice, uint32_t ValidateNode = 0);
    bool SendDeviceAdd(uint32_t NodeAddr, String const &DevName);
    bool SendDeviceDel(uint32_t NodeAddr);
    void ReleaseThread();                                   // release the receive thread

protected:
    virtual void SegmentDispose(IpcSegment *pSeg);          // properly dispose of a segment after transmission
    IoDev *SendDeviceGet(uint32_t NodeAddr);                // return appropriate send device

    IoDev              *m_PtrSendDevice;                    // device to send messages
    IoDev              *m_PtrRecvDevice;                    // device to receive messages
    Mutex               m_MutexSendDevs;                    // mutex to synchronize access to send device map
    DeviceMap_t         m_SendDevices;                      // map of send devices by node address
};

}   // namespace cp

#endif  // CP_IPCTRANSPORT_H
