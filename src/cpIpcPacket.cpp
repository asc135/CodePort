// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcPacket.cpp
//
//  Description:    IPC message packet.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
//  2013-08-22  asc Removed MoveMsgIn() to decouple from Dispatch class.
// ----------------------------------------------------------------------------

#include "cpIpcPacket.h"

namespace cp
{

// copy constructors
IpcPacket::IpcPacket(IpcPacket &rhs) :
    m_Rsp("Response")
{
    // invoke the assignment operator
    *this = rhs;
}


// operators
IpcPacket &IpcPacket::operator=(IpcPacket &rhs)
{
    // check for self assignment
    if (this == &rhs)
    {
        return *this;
    }

    // this is disabled (private), so nothing is copied
    return *this;
}


// store segment into the message packet and decode
bool IpcPacket::PtrSeg(IpcSegment *pSegment)
{
    // store the segment pointer
    // do not delete existing segments since they are owned
    // by the decoder and will be deleted in the next operation
    m_PtrSeg = pSegment;

    // load the segment stream in the decoder
    // this deletes any existing loaded segments
    return m_Decoder.LoadSegment(pSegment);
}


// send a response to message originator
bool IpcPacket::SendResponse()
{
    bool rv = false;

    if (MsgId() && SrcAddr() && m_PtrNode)
    {
        rv = (m_PtrNode->SendDat(SrcAddr(), m_Rsp, 0, MsgId()) != 0);
    }
    else
    {
        LogErr << "IpcPacket::SendResponse(): Could not send response, instance: "
               << this << std::endl;
    }

    return rv;
}

}   // namespace cp
