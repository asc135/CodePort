// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcNode.h
//
//  Description:    IPC endpoint node.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
//  2012-12-13  asc Added multiple dispatch thread capability.
//  2013-01-17  asc Added exit control semaphore.
//  2013-02-01  asc Changed validation mechanism.
//  2013-02-13  asc Added SendCtl() and adjusted message field usage.
//  2013-03-11  asc Added Connect() and Disconnect() methods.
//  2013-03-21  asc Changed Transport object to a pointer to enable inheritance.
//  2013-03-22  asc Added support for accumulator timeout handling.
//  2013-04-24  asc Added StopNode() method.
//  2013-05-16  asc Added CheckForExit() method.
//  2013-08-27  asc Refactored name resolver management.
//  2013-09-30  asc Added support for node startup sync.
//  2013-10-14  asc Added support for watchdog control message.
// ----------------------------------------------------------------------------

#ifndef CP_IPCNODE_H
#define CP_IPCNODE_H

#include "cpIpcNodeUtil.h"
#include "cpIpcResolver.h"

namespace cp
{

// forward references
class Buffer;
class Datum;
class IoDev;
class IpcSegment;
class IpcTransport;

// ----------------------------------------------------------------------------

// the IPC node interface
class IpcNode : public Base
{
public:
    // local types
    typedef void *(*WatchDogFunc_t)(void *pContext);

    // thread enumerations
    enum Threads { IpcReceive, IpcTransmit };

    // constructor
    IpcNode(String const &NodeName);

    // destructor
    ~IpcNode();

    // accessors
    uint32_t NodeAddr()     { return m_NodeAddr; }          // return the current node address
    IpcResolver &Resolver() { return m_Resolver; }          // return address resolver instance
    bool CheckForExit()     { return m_Flag1;    }          // return true if exit has been signalled

    bool WaitForSync(uint32_t Timeout = k_InfiniteTimeout)
        { return m_SemStart.Take(Timeout); }                // block until start sync condition

    bool WaitForExit(uint32_t Timeout = k_InfiniteTimeout)
        { return m_SemExit.Take(Timeout); }                 // block until node exit condition

    // send and receive methods
    uint32_t SendCtl(uint32_t Address,                      // forward a message to the router
                     uint8_t CtlCode,
                     uint32_t Context = 0,
                     uint8_t Priority = k_IpcDefaultPriority);

    uint32_t SendBuf(uint32_t Address,                      // forward a message to the router
                     char const *pBuf,
                     size_t MsgLen,
                     uint8_t MsgType,
                     uint8_t CtlCode = 0,
                     uint32_t Context = 0,
                     uint8_t Priority = k_IpcDefaultPriority);

    uint32_t SendDat(uint32_t Address,                      // forward a message to the router
                     Datum &Dat,
                     uint8_t CtlCode = 0,
                     uint32_t Context = 0,
                     uint8_t Priority = k_IpcDefaultPriority);

    uint32_t SendSeg(IpcSegment *pSeg);                     // forward a message to the router

    bool GetResponse(uint32_t MsgId,                        // get a response
                     IpcSegment *&pResponse,
                     uint32_t Timeout = k_ResponseTimeout);

    // manipulators
    bool Connect(uint32_t Address);                         // sets up a persistent IPC channel
    bool Disconnect(uint32_t Address);                      // tears down a persistent IPC channel
    void WatchDogSet(WatchDogFunc_t pFunc, void *pContext); // configure the watchdog function
    void WatchDog();                                        // invoke the watch dog operation
    void StartSync()  { m_SemStart.Give(); }                // give the start sync semaphore
    void SignalExit() { m_Flag1 = m_SemExit.Give(); }       // signal node to exit
    void NodeAddrSet(uint32_t Addr) { m_NodeAddr = Addr; }  // set the node address
    void TransportSet(IpcTransport *pTransport);            // sets the transport device
    bool DevicesSet(IoDev *pSendDevice,
                    IoDev *pRecvDevice,
                    uint32_t ValidateNode = 0);             // set send and receive device

    void ResolverNameSet(String const &Name)                // set the resolver node name
    {
        m_ResolverNodeName = Name;
    }

    void ResolverAddrSet(uint32_t Address)                  // set the resolver node address
    {
        m_ResolverNodeAddr = Address;
    }

    bool StartNode();                                       // start I/O operations
    void StopNode();                                        // stop I/O operations

    bool RegisterHandler(DispatchHandler_t pHandler,
                         uint32_t MsgId,
                         uint32_t NumThreads,
                         void *pContext = NULL);            // register a handler function to process received messages

    bool RemoveHandler(DispatchHandler_t pHandler,
                       uint32_t MsgId);                     // remove a handler function

    // system management methods
    void ExpiredAccumNotify(IpcSegment const *pSeg);        // notify node of an expired accumulator
    void ValidMessageNotify(IpcSegment const *pSeg);        // notify node of a valid message
    void FlushAddrCache();                                  // flush the address cache

private:
    IpcNode(IpcNode const &rhs);                            // copy constructor (disabled)
    IpcNode &operator=(IpcNode const &rhs);                 // assignment operator (disabled)

    bool CreateSegment(IpcSegment *&pSeg,
                       uint32_t DestAddr,
                       uint8_t Priority,
                       uint32_t Context,
                       uint8_t MsgType,
                       uint8_t OpCode);                     // create a segment and populate fields

    uint32_t TransmitMessage(IpcSegment *pSeg)              // queue up a message for transmission
    {
        return m_TransmitQueue.TransmitMessage(pSeg);
    }

    void ProcessTimeouts();                                 // process timeouts and cleanup stale resources
    void RecvThread();                                      // communications receive thread function
    void XmitThread();                                      // communications transmit thread function
    static void *ThreadFunction(Thread *pThread);           // static thread trampoline function

    uint32_t            m_NodeAddr;                         // the address of the node using this interface
    uint32_t            m_ResolverNodeAddr;                 // address of the resolver node
    String              m_ResolverNodeName;                 // name of the resolver node
    SemLite             m_SemStart;                         // start sync semaphore
    SemLite             m_SemExit;                          // exit control semaphore
    Thread              m_RecvThread;                       // receive thread object
    Thread              m_XmitThread;                       // transmit thread object
    IpcTransport       *m_PtrTransport;                     // IPC transport layer object
    IpcAccumMap         m_AccumMap;                         // map of IPC message segment accumulators
    IpcTransmitQueue    m_TransmitQueue;                    // queue of outgoing messages
    IpcResolver         m_Resolver;                         // address resolver and cache
    WatchDogFunc_t      m_PtrWatchDogFunc;                  // pointer to the watchdog function
    void               *m_PtrWatchDogParam;                 // parameter to the watchdog function
};

}   // namespace cp

#endif  // CP_IPCNODE_H
