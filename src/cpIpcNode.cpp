// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012-2014 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcNode.cpp
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
//  2013-02-15  asc Added ThreadExitFunc() to synchronize shutdown.
//  2013-03-11  asc Added Connect() and Disconnect() methods.
//  2013-03-21  asc Changed Transport object to a pointer to enable inheritance.
//  2013-03-22  asc Added support for accumulator timeout handling.
//  2013-04-24  asc Added StopNode() method.
//  2013-08-27  asc Refactored name resolver management.
//  2013-09-30  asc Added support for node startup sync.
//  2013-10-14  asc Added support for watchdog control message.
//  2014-03-30  asc Testing for valid before any send.
// ----------------------------------------------------------------------------

#include "cpUtil.h"
#include "cpDatum.h"
#include "cpIpcNode.h"
#include "cpIpcPacket.h"
#include "cpIpcTransport.h"
#include "cpIpcStreamSeg.h"

namespace cp
{

// global data

// ----------------------------------------------------------------------------

// control code event handler function
void *ControlEventHandler(DispatchEvent *pDispEvent)
{
    IpcPacket *pPacket = NULL;
    IpcNode *pNode = NULL;

    // check that the event is valid
    if (pDispEvent)
    {
        pPacket = reinterpret_cast<IpcPacket *>(pDispEvent->pEvent);
        pNode = reinterpret_cast<IpcNode *>(pDispEvent->pContext);
    }

    // handle control events
    if (pPacket)
    {
        // put the node pointer into the message packet
        pPacket->PtrNode(pNode);

        if (pPacket->MsgType() == IpcSegment::msg_Control)
        {
            switch (pPacket->CtlCode())
            {
            case IpcSegment::ctl_NoOp:
                break;

            case IpcSegment::ctl_Shutdown:
                LogMsg << "IpcNode::ControlEventHandler(): Received exit signal: " << pNode->NameGet() << std::endl;
                pNode->SignalExit();
                break;

            case IpcSegment::ctl_Cancel:
                break;

            case IpcSegment::ctl_Reset:
                break;

            case IpcSegment::ctl_Suspend:
                break;

            case IpcSegment::ctl_Resume:
                break;

            case IpcSegment::ctl_TraceOn:
                break;

            case IpcSegment::ctl_TraceOff:
                break;

            case IpcSegment::ctl_WatchDog:
                pNode->WatchDog();
                break;

            case IpcSegment::ctl_FlushAddr:
                pNode->FlushAddrCache();
                break;

            case IpcSegment::ctl_StartSync:
                pNode->StartSync();
                break;

            case IpcSegment::ctl_Extended:
                break;

            default:
                break;
            }
        }
    }

    return pDispEvent;
}

// ----------------------------------------------------------------------------

// constructor
IpcNode::IpcNode(String const &NodeName) :
    Base(NodeName),
    m_NodeAddr(0),
    m_SemStart("IpcNode Start Sync Semaphore", 0, 1),
    m_SemExit("IpcNode Exit Control Semaphore", 0, 1),
    m_RecvThread("IpcNode Receive Thread",  ThreadFunction, this, Thread::opt_Suspended, IpcReceive),
    m_XmitThread("IpcNode Transmit Thread", ThreadFunction, this, Thread::opt_Suspended, IpcTransmit),
    m_PtrTransport(NULL),
    m_AccumMap(this),
    m_PtrWatchDogFunc(NULL),
    m_PtrWatchDogParam(NULL)
{
    // register the control code handler
    RegisterHandler(ControlEventHandler, 0, 4, this);
}


// copy constructor (disabled)
IpcNode::IpcNode(IpcNode const &rhs) :
    Base(rhs.NameGet()),
    m_NodeAddr(0),
    m_SemExit("dummy Semaphore", 0, 1),
    m_RecvThread("dummy thread", ThreadFunction, this, Thread::opt_Suspended, -1),
    m_XmitThread("dummy thread", ThreadFunction, this, Thread::opt_Suspended, -1),
    m_PtrTransport(NULL),
    m_AccumMap(NULL)
{
    (void)rhs;
}


// destructor
IpcNode::~IpcNode()
{
    if (m_Valid)
    {
        // send exit request signal to all the threads
        m_RecvThread.ExitReq();
        m_XmitThread.ExitReq();

        // release the receive thread
        if (m_PtrTransport)
        {
            m_PtrTransport->ReleaseThread();
        }

        // release the transmit thread
        m_TransmitQueue.ReleaseThread();

        // wait for threads to exit
        m_RecvThread.WaitExit(k_ReceiveTimeout);
        m_XmitThread.WaitExit(k_ReceiveTimeout);
    }

    LogMsg << "Shutting down comm interface for node: "
           << NameGet() << std::endl;

    // delete the transport object
    if (m_PtrTransport)
    {
        delete m_PtrTransport;
    }

    IpcSegment::Stats();
}


// assignment operator (disabled)
IpcNode &IpcNode::operator=(IpcNode const &rhs)
{
    (void)rhs;
    return *this;
}


// forward a message to the router
uint32_t IpcNode::SendCtl(uint32_t Address, uint8_t CtlCode, uint32_t Context, uint8_t Priority)
{
    uint32_t rv = 0;
    bool status = true;
    IpcSegment *pSeg = NULL;

    // create the segment and set its fields
    status = m_Valid && CreateSegment(pSeg, Address, Priority, Context, IpcSegment::msg_Control, CtlCode);

    // send the message
    if (status)
    {
        // set the control option flag
        pSeg->Options(IpcSegment::opt_Control);

        rv = TransmitMessage(pSeg);
    }

    return rv;
}


// forward a message to the router
uint32_t IpcNode::SendBuf(uint32_t Address, char const *pBuf, size_t MsgLen, uint8_t MsgType, uint8_t CtlCode, uint32_t Context, uint8_t Priority)
{
    uint32_t rv = 0;
    bool status = true;
    IpcSegment *pSeg = NULL;
    IpcStreamSeg strm;

    if ((pBuf == NULL) || (MsgLen == 0))
    {
        return 0;
    }

    // create the segment and set its fields
    status = m_Valid && CreateSegment(pSeg, Address, Priority, Context, MsgType, CtlCode);

    // encode payload into one or more segments
    if (status)
    {
        // load the segment template into the stream object
        strm.Template(*pSeg);
        delete pSeg;

        // copy payload into segment stream
        status = (strm.ArrayWr(pBuf, MsgLen) == MsgLen);

        if (status)
        {
            // prepare for transmit
            strm.Finalize();
            strm.SegmentExtract(pSeg);
        }
        else
        {
            LogErr << "IpcNode::SendMsg(): Failed to encode data buffer: "
                   << NameGet() << std::endl;
        }
    }

    // send the message
    if (status)
    {
        rv = TransmitMessage(pSeg);
    }

    return rv;
}


// forward a message to the router
uint32_t IpcNode::SendDat(uint32_t Address, Datum &Dat, uint8_t CtlCode, uint32_t Context, uint8_t Priority)
{
    uint32_t rv = 0;
    bool status = true;
    IpcSegment *pSeg = NULL;
    IpcStreamSeg strm;

    // create the segment and set its fields
    status = m_Valid && CreateSegment(pSeg, Address, Priority, Context, IpcSegment::msg_Datum, CtlCode);

    // encode payload into one or more segments
    if (status)
    {
        // load the segment template into the stream object
        strm.Template(*pSeg);
        delete pSeg;

        // copy payload into segment stream
        status = Dat.Encode(strm);

        if (status)
        {
            // prepare for transmit
            strm.Finalize();
            strm.SegmentExtract(pSeg);
        }
        else
        {
            LogErr << "IpcNode::SendMsg(): Failed to encode Datum instance, Node: "
                   << NameGet() << ", Datum: " << Dat.NameGet() << std::endl;
        }
    }

    // send the message
    if (status)
    {
        rv = TransmitMessage(pSeg);
    }

    return rv;
}


// forward a message to the router
uint32_t IpcNode::SendSeg(IpcSegment *pSeg)
{
    uint32_t rv = 0;
    bool status = m_Valid && (pSeg != NULL);

    // send the message
    if (status)
    {
        rv = TransmitMessage(pSeg);
    }

    return rv;
}


// get a response
bool IpcNode::GetResponse(uint32_t MsgId, IpcSegment *&pResponse, uint32_t Timeout)
{
    return m_AccumMap.GetResponse(MsgId, pResponse, Timeout);
}


// sets up a persistent IPC channel
bool IpcNode::Connect(uint32_t Address)
{
    bool rv = false;
    (void)Address;
    return rv;
}


// tears down a persistent IPC channel
bool IpcNode::Disconnect(uint32_t Address)
{
    bool rv = false;
    (void)Address;
    return rv;
}


// configure the watchdog function
void IpcNode::WatchDogSet(WatchDogFunc_t pFunc, void *pContext)
{
    m_PtrWatchDogFunc = pFunc;
    m_PtrWatchDogParam = pContext;
}


// invoke the watch dog operation
void IpcNode::WatchDog()
{
    if (m_PtrWatchDogFunc)
    {
        (*m_PtrWatchDogFunc)(m_PtrWatchDogParam);
    }
}


// sets the transport device
void IpcNode::TransportSet(IpcTransport *pTransport)
{
    if (!m_Valid)
    {
        m_PtrTransport = pTransport;
    }
}


// set send and receive device
bool IpcNode::DevicesSet(IoDev *pSendDevice, IoDev *pRecvDevice, uint32_t ValidateNode)
{
    m_Valid = m_PtrTransport && m_PtrTransport->DevicesSet(pSendDevice, pRecvDevice, ValidateNode);
    return m_Valid;
}


// start I/O operations
bool IpcNode::StartNode()
{
    bool rv = m_Valid && m_PtrTransport && m_PtrTransport->IsValid();

    // transport must be valid before starting threads
    if (rv)
    {
        m_RecvThread.Resume();
        m_XmitThread.Resume();
    }
    else
    {
        LogErr << "IpcNode::StartNode(): Transport not functional: "
               << NameGet() << std::endl;
    }

    return rv;
}


// stop I/O operations
void IpcNode::StopNode()
{
    m_RecvThread.Suspend();
    m_XmitThread.Suspend();
}


// register a handler function to process received messages
bool IpcNode::RegisterHandler(DispatchHandler_t pHandler, uint32_t MsgId, uint32_t NumThreads, void *pContext)
{
    return m_AccumMap.RegisterHandler(pHandler, MsgId, NumThreads, pContext);
}


// remove a handler function
bool IpcNode::RemoveHandler(DispatchHandler_t pHandler, uint32_t MsgId)
{
    return m_AccumMap.RemoveHandler(pHandler, MsgId);
}


// notify node of an expired accumulator
void IpcNode::ExpiredAccumNotify(IpcSegment const *pSeg)
{
    (void)pSeg;
    return;
}


// notify node of a valid message
void IpcNode::ValidMessageNotify(IpcSegment const *pSeg)
{
    (void)pSeg;
    return;
}


// flush the address cache
void IpcNode::FlushAddrCache()
{
    // clear the address cache
    m_Resolver.Clear();

    // manually add the entry for resolver node so addresses can be resolved
    m_Resolver.AddressAdd(m_ResolverNodeName, m_ResolverNodeAddr);
}


// create a segment and populate fields
bool IpcNode::CreateSegment(IpcSegment *&pSeg, uint32_t DestAddr, uint8_t Priority, uint32_t Context, uint8_t MsgType, uint8_t CtlCode)
{
    bool rv = (DestAddr != 0);

    if (rv)
    {
        // create a segment object
        pSeg = new (CP_NEW) IpcSegment;
        rv = (pSeg != NULL);

        if (!rv)
        {
            LogErr << "IpcNode::CreateSegment(): Failed to create an IpcSegment: "
                   << NameGet() << std::endl;
        }
    }

    // set the segment fields
    if (rv)
    {
        pSeg->Priority(Priority);
        pSeg->SrcAddr(m_NodeAddr);
        pSeg->DstAddr(DestAddr);
        pSeg->Context(Context);
        pSeg->MsgType(MsgType);
        pSeg->CtlCode(CtlCode);
    }

    return rv;
}


// process timeouts and cleanup stale resources
void IpcNode::ProcessTimeouts()
{
    // (.)(.) implement
    LogMsg << "IpcNode::ProcessTimeouts(): Processing timeouts: "
           << NameGet() << std::endl;
}


// communications receive thread function
void IpcNode::RecvThread()
{
    IpcSegment *pSeg = NULL;

    while (m_RecvThread.ThreadPoll())
    {
        // check if a valid segment is pointed to
        if (pSeg == NULL)
        {
            // if not, create a new segment object
            pSeg = new (CP_NEW) IpcSegment;
        }

        // check if a valid segment now exists
        if (pSeg == NULL)
        {
            // if a segment can't be created suspend the thread
            LogErr << "IpcNode::RecvThread: Failed to create a receive segment: "
                   << NameGet() << std::endl;
            m_RecvThread.Suspend();
            continue;
        }

        // block and wait for an incoming message
        if (m_PtrTransport && m_PtrTransport->Recv(pSeg, k_ReceiveTimeout))
        {
            // pass it to accumulator thread to process the message
            if (m_AccumMap.SubmitSegment(pSeg) == false)
            {
                LogErr << "IpcNode::RecvThread(): Cannot submit to accumulator. Discarding received segment: "
                       << NameGet() << std::endl;

                // delete segment if it can't be delivered to accumulator
                if (pSeg)
                {
                    delete pSeg;
                }
            }

            // clear the pointer
            pSeg = NULL;
        }
    }

    // clean up before exit
    if (pSeg)
    {
        delete pSeg;
    }
}


// communications transmit thread function
void IpcNode::XmitThread()
{
    IpcSegment *pSeg = NULL;
    uint32_t count = 0;
    uint32_t const k_MaxTransmitCount = 4;

    while (m_XmitThread.ThreadPoll())
    {
        // wait for segments here
        if (m_TransmitQueue.SegmentGet(pSeg, k_ReceiveTimeout))
        {
            if (pSeg == NULL)
            {
                LogErr <<"IpcNode::XmitThread(): Received a NULL segment to transmit: "
                       << NameGet() << std::endl;
            }
            else
            {
                if (m_PtrTransport && m_PtrTransport->Send(pSeg) == false)
                {
                    // delete the segment
                    delete pSeg;

                    LogErr << "!!! IpcNode::XmitThread(): Failed to send a segment: "
                           << NameGet() << std::endl;
                }

                // segment has either been handed over or deleted
                pSeg = NULL;

                // increment transmit counter
                ++count;
            }

            // yield the CPU if a burst is completed
            if (count > k_MaxTransmitCount)
            {
                count = 0;
                ThreadYield();
            }
        }
    }
}


// static thread trampoline function
void *IpcNode::ThreadFunction(Thread *pThread)
{
    IpcNode *pNode = reinterpret_cast<IpcNode *>(pThread->ContextGet());

    switch (pThread->SelectorGet())
    {
    case IpcReceive:
        pNode->RecvThread();
        break;

    case IpcTransmit:
        pNode->XmitThread();
        break;

    default:
        break;
    }

    return NULL;
}

}   // namespace cp
