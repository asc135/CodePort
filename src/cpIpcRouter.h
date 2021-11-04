// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcRouter.h
//
//  Description:    IPC message router.
//
//  Platform:       common
//
//  History:
//  2013-01-28  asc Creation.
//  2013-04-24  asc Added ReleaseThread() method.
//  2013-05-16  asc Integrated router thread into main class.
//  2013-05-28  asc Changed NodeDel() return type to bool.
// ----------------------------------------------------------------------------

#ifndef CP_IPCROUTER_H
#define CP_IPCROUTER_H

#include <map>

#include "cpQueue.h"
#include "cpThread.h"

namespace cp
{

class IpcSegment;

// ----------------------------------------------------------------------------

// custom type definitions
typedef std::map<uint32_t, Queue *, std::less<uint32_t const>, Alloc< std::pair<uint32_t, Queue *> > > IpcNodeQueueMap_t;
typedef std::map<String, uint32_t,  std::less<String const>,   Alloc< std::pair<String, uint32_t > > > IpcNodeAddrMap_t;

// ----------------------------------------------------------------------------

class IpcRouter
{
public:
    // constructor
    IpcRouter();

    // destructor
    ~IpcRouter();

    // accessors
    bool IsValid();
    uint32_t NodeFind(String const &NodeName);

    // manipulators
    uint32_t NodeCreate(String const &NodeName, String &IoDevice);
    bool NodeDel(uint32_t Address);
    void ReleaseThread();                                   // release the router thread

    // input/output functions
    bool Send(IpcSegment &Seg, uint32_t Timeout = k_InfiniteTimeout);
    bool Recv(IpcSegment &Seg, uint32_t Timeout = k_InfiniteTimeout);

private:
    uint32_t NodeAdd(String const &NodeName, String &IoDevice);
    Queue *QueueFind(uint32_t Address);
    void RtrThread();                                       // router thread function
    static void *ThreadFunction(cp::Thread *pThread);       // static thread trampoline function

    uint32_t            m_NextNodeAddr;                     // next available node address
    Queue               m_RecvDevice;                       // device to receive incoming messages
    Mutex               m_MtxMaps;                          // mutex to synchronize access to node maps
    Thread              m_RtrThread;                        // router thread object
    IpcNodeQueueMap_t   m_NodeQueues;                       // maps numeric addresses to send queues
    IpcNodeAddrMap_t    m_NodeAddresses;                    // maps string names to numeric addresses
};

}   // namespace cp

#endif  // CP_IPCROUTER_H
