// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcHandle.cpp
//
//  Description:    IPC transaction handle.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
//  2013-06-17  asc Added check to make sure msg ID is valid in WaitReply().
//  2013-07-17  asc Added Response() accessor method.
// ----------------------------------------------------------------------------

#include "cpIpcNode.h"
#include "cpIpcHandle.h"

namespace cp
{

// constructor
IpcHandle::IpcHandle(IpcNode *pNode) :
    m_MsgId(0),
    m_PtrNode(pNode),
    m_PtrDecoder(NULL)
{
}


// destructor
IpcHandle::~IpcHandle()
{
    // delete the decoder object if one was created
    if (m_PtrDecoder)
    {
        delete m_PtrDecoder;
    }
}


// assignment operator
IpcHandle &IpcHandle::operator=(IpcHandle const &rhs)
{
    // check for self assignment
    if (this != &rhs)
    {
        m_MsgId = rhs.m_MsgId;
        m_PtrNode = rhs.m_PtrNode;
    }

    return *this;
}


// wait for a reply
bool IpcHandle::WaitReply(uint32_t Timeout)
{
    bool rv = false;
    IpcSegment *pSeg = NULL;

    if (m_PtrNode && m_MsgId)
    {
        // wait for a response to arrive
        rv = m_PtrNode->GetResponse(m_MsgId, pSeg, Timeout);
    }

    if (rv)
    {
        // create a decoder object if one doesn't already exist
        if (m_PtrDecoder == NULL)
        {
            m_PtrDecoder = new (CP_NEW) IpcDecoder;
            rv = (m_PtrDecoder != NULL);
        }
    }

    if (rv)
    {
        // load the received segment list into the decoder object
        rv = m_PtrDecoder->LoadSegment(pSeg);
    }

    return rv;
}


// register a reply handler function
bool IpcHandle::RegisterHandler(DispatchHandler_t pHandler)
{
    bool rv = false;

    if (m_PtrNode && m_MsgId)
    {
        rv = m_PtrNode->RegisterHandler(pHandler, m_MsgId, 1, m_PtrNode);
    }

    return rv;
}


// return the received response
Datum &IpcHandle::Response()
{
    if (m_PtrDecoder)
    {
        return m_PtrDecoder->Msg();
    }
    else
    {
        return Datum::Inert();
    }
}

}   // namespace cp
