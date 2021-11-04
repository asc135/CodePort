// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcRouter.cpp
//
//  Description:    IPC message router.
//
//  Platform:       common
//
//  History:
//  2013-01-28  asc Creation.
//  2013-04-24  asc Added ReleaseThread() method.
//  2013-05-15  asc Added broadcast support.
//  2013-05-16  asc Integrated router thread into main class.
//  2013-05-28  asc Changed NodeDel() return type to bool.
//  2013-06-17  asc Added pool allocator to vector.
//  2013-08-27  asc Starting with router thread suspended to allow Flush().
//  2014-05-29  asc Check if queue is valid after object creation.
// ----------------------------------------------------------------------------

#include "cpUtil.h"
#include "cpIpcRouter.h"
#include "cpIpcSegment.h"

namespace cp
{

// constructor
IpcRouter::IpcRouter() :
    m_NextNodeAddr(k_IpcNodeAddrMinVal),
    m_RecvDevice("00000000", IpcSegment::seg_MaxLen, k_IpcCommsQueueDepth),
    m_MtxMaps("Router Node Maps Mutex"),
    m_RtrThread("Router Thread", ThreadFunction, this, Thread::opt_Suspended)
{
    // flush the incoming message device
    m_RecvDevice.Flush();

    // create broadcast address
    m_MtxMaps.Lock();
    m_NodeAddresses[k_BroadcastNode] = static_cast<uint32_t>(~0);
    m_MtxMaps.Unlock();

    // start the router thread
    m_RtrThread.Resume();
}


// destructor
IpcRouter::~IpcRouter()
{
    IpcNodeQueueMap_t::iterator i = m_NodeQueues.begin();

    // signal router thread to exit
    m_RtrThread.ExitReq();

    // release the router thread
    ReleaseThread();

    // wait for router thread to exit
    m_RtrThread.WaitExit(cp::k_DefaultTimeout);

    // make sure all registered nodes' send device instances are destroyed
    while (i != m_NodeQueues.end())
    {
        if (i->second != NULL)
        {
            // flush the queue
            i->second->Flush();

            // delete the queue
            delete i->second;
            i->second = NULL;
        }

        ++i;
    }
}


uint32_t IpcRouter::NodeCreate(String const &NodeName, String &IoDevice)
{
    uint32_t nodeAddr = 0;

    // make sure node name doesn't already exist
    if (NodeFind(NodeName) == 0)
    {
        // create the map entry
        nodeAddr = NodeAdd(NodeName, IoDevice);
    }

    return nodeAddr;
}


// release the router thread
void IpcRouter::ReleaseThread()
{
    // send a four byte message which should be ignored by router thread
    Buffer buf(sizeof(uint32_t));
    buf.LenSet(sizeof(uint32_t));
    m_RecvDevice.Send(buf, buf, k_TransmitTimeout);
}


bool IpcRouter::Send(IpcSegment &Seg, uint32_t Timeout)
{
    bool rv = false;
    Queue *pQ = QueueFind(Seg.DstAddr());

    if (pQ != NULL)
    {
        rv = (pQ->Send(Seg.Buf(), Seg.Buf(), Timeout) > 0);
    }
    else if (Seg.DstAddr() == static_cast<uint32_t>(~0))
    {
        // broadcast
        rv = true;

        std::vector<Queue *, Alloc<Queue *> > nodes;
        IpcNodeQueueMap_t::iterator i;

        // build a vector of all nodes
        m_MtxMaps.Lock();

        i = m_NodeQueues.begin();

        while (i != m_NodeQueues.end())
        {
            nodes.push_back(i->second);
            ++i;
        }

        m_MtxMaps.Unlock();

        // send message to all nodes in the vector
        while (nodes.size())
        {
            if (!nodes.back()->Send(Seg.Buf(), Seg.Buf(), Timeout))
            {
                LogErr << "IpcRouter::Send(): Could not broadcast to node queue: " << nodes.back() << std::endl;
            }

            nodes.pop_back();
        }
    }
    else
    {
        LogErr << "IpcRouter::Send(): Could not locate transport for address: "
               << Seg.DstAddr() << ", instance: " << this << std::endl;
    }

    return rv;
}


bool IpcRouter::Recv(IpcSegment &Seg, uint32_t Timeout)
{
    return (m_RecvDevice.Recv(Seg.Buf(), Seg.Buf(), Timeout) >= IpcSegment::seg_Data);
}


uint32_t IpcRouter::NodeAdd(String const &NodeName, String &IoDevice)
{
    uint32_t nodeAddr = 0;
    Queue *pQueue = NULL;

    m_MtxMaps.Lock();

    // check if any new addresses are available
    if (m_NextNodeAddr < static_cast<uint32_t>(~0))
    {
        // acquire a new, unique node address number
        nodeAddr = m_NextNodeAddr++;
    }

    m_MtxMaps.Unlock();

    if (nodeAddr)
    {
        // create a device name based on that address
        IoDevice = UintToStr(nodeAddr);

        if (IoDevice.size() < k_IpcNodeDevNameMinLen)
        {
            IoDevice = GenStr(k_IpcNodeDevNameMinLen - IoDevice.size(), '0') + IoDevice;
        }

        // create a queue object using the new io device name
        pQueue = new (CP_NEW) Queue(IoDevice, IpcSegment::seg_MaxLen, k_IpcCommsQueueDepth);

        // store the record if queue is created successfully
        if (pQueue)
        {
            if (pQueue->IsValid())
            {
                pQueue->Flush();

                m_MtxMaps.Lock();
                m_NodeQueues[nodeAddr] = pQueue;
                m_NodeAddresses[NodeName] = nodeAddr;
                m_MtxMaps.Unlock();
            }
            else
            {
                LogErr << "IpcRouter::NodeAdd(): Device queue invalid, instance: "
                       << this << ", NodeName: " << NodeName << ", IoDevice: " << IoDevice << std::endl;
                delete pQueue;
                pQueue = NULL;
            }
        }
        else
        {
            LogErr << "IpcRouter::NodeAdd(): Failed to create a device Queue, instance: "
                   << this << ", NodeName: " << NodeName << ", IoDevice: " << IoDevice << std::endl;
        }
    }
    else
    {
        LogErr << "IpcRouter::NodeAdd(): Failed, out of node addresses." << std::endl;
    }

    // return node address or zero if error
    return (pQueue != NULL) ? nodeAddr : 0;
}


bool IpcRouter::NodeDel(uint32_t Address)
{
    bool rv = false;
    IpcNodeQueueMap_t::iterator i;
    IpcNodeAddrMap_t::iterator j;

    m_MtxMaps.Lock();

    // remove node to queue mapping and delete queue
    i = m_NodeQueues.find(Address);

    if (i != m_NodeQueues.end())
    {
        if (i->second != NULL)
        {
            delete i->second;
        }

        m_NodeQueues.erase(i);
        rv = true;
    }

    // locate and remove name to node address mapping
    j = m_NodeAddresses.begin();

    while (j != m_NodeAddresses.end())
    {
        if (j->second == Address)
        {
            m_NodeAddresses.erase(j);
            rv = true;
            break;
        }
        else
        {
            ++j;
        }
    }

    m_MtxMaps.Unlock();

    return rv;
}


uint32_t IpcRouter::NodeFind(String const &NodeName)
{
    uint32_t rv = 0;
    IpcNodeAddrMap_t::iterator i;

    m_MtxMaps.Lock();

    i = m_NodeAddresses.find(NodeName);

    if (i != m_NodeAddresses.end())
    {
        rv = i->second;
    }

    m_MtxMaps.Unlock();

    return rv;
}


Queue *IpcRouter::QueueFind(uint32_t Address)
{
    Queue *rv = NULL;
    IpcNodeQueueMap_t::iterator i;

    m_MtxMaps.Lock();

    i = m_NodeQueues.find(Address);

    if (i != m_NodeQueues.end())
    {
        rv = i->second;
    }

    m_MtxMaps.Unlock();

    return rv;
}


// router thread function
void IpcRouter::RtrThread()
{
    cp::IpcSegment seg;

    while (m_RtrThread.ThreadPoll())
    {
        // wait for a message to arrive
        if (Recv(seg, cp::k_ReceiveTimeout))
        {
            // send it to the next destination
            if (!Send(seg, cp::k_TransmitTimeout))
            {
                cp::LogErr << "IpcRouter::RtrThread(): Failed to forward a message from "
                           << seg.SrcAddr() << " to " << seg.DstAddr() << std::endl;
            }
        }
    }
}


// static thread trampoline function
void *IpcRouter::ThreadFunction(cp::Thread *pThread)
{
    reinterpret_cast<IpcRouter *>(pThread->ContextGet())->RtrThread();

    return pThread;
}

}   // namespace cp
