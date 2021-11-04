// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcPacket.h
//
//  Description:    IPC message packet.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
//  2013-07-17  asc Removed unused member variables.
//  2013-08-22  asc Removed MoveMsgIn() to decouple from Dispatch class.
// ----------------------------------------------------------------------------

#ifndef CP_IPCPACKET_H
#define CP_IPCPACKET_H

#include "cpDatum.h"
#include "cpIpcStreamSeg.h"
#include "cpIpcDecoder.h"
#include "cpIpcNode.h"
#include "cpUtil.h"

namespace cp
{

class IpcPacket : public PooledBase
{
public:
    // constructor
    IpcPacket() :
        m_PtrNode(NULL),
        m_PtrSeg(NULL),
        m_PtrCur(NULL),
        m_Rsp("Response")
    {
        CurrentSet();
    }

    // destructor
    ~IpcPacket()
    {
        // do not delete m_PtrNode since it is not owned by this object

        // do not delete m_PtrSeg since it is owned by m_Decoder

        // do not delete m_PtrCur since it points to a Datum owned by m_Decoder
    }

    // accessors
    uint32_t MsgType() const { return m_PtrSeg ? m_PtrSeg->MsgType() : 0; }
    uint32_t MsgId()   const { return m_PtrSeg ? m_PtrSeg->MsgId()   : 0; }
    uint32_t SrcAddr() const { return m_PtrSeg ? m_PtrSeg->SrcAddr() : 0; }
    uint32_t Context() const { return m_PtrSeg ? m_PtrSeg->Context() : 0; }
    uint32_t CtlCode() const { return m_PtrSeg ? m_PtrSeg->CtlCode() : 0; }

    IpcNode *PtrNode()       { return  m_PtrNode;       }
    IpcSegment *PtrSeg()     { return  m_PtrSeg;        }
    Datum &Cur()             { return *m_PtrCur;        }
    Datum &Rsp()             { return  m_Rsp;           }
    Datum &Msg()             { return  m_Decoder.Msg(); }
    Buffer &Buf()            { return  m_Decoder.Buf(); }

    bool ParamSelect(cp::String const Key) { return Cur().Select(Key); }
    Variant const &ParamGet() { return Cur().Get().Val(); }

    // mutators
    void PtrNode(IpcNode *pNode) { m_PtrNode = pNode; }     // store node into the message packet
    bool PtrSeg(IpcSegment *pSegment);                      // store segment into the message packet and decode
    void CurrentSet() { m_PtrCur = &(m_Decoder.Msg().Get()); } // set current msg to default subdatum in m_Decoder.Msg()
    bool SendResponse();                                    // send a response to message originator

private:
    // copy constructor
    IpcPacket(IpcPacket &rhs);

    // operators
    IpcPacket &operator=(IpcPacket &rhs);

    IpcNode            *m_PtrNode;                          // IPC Node used for communications to remote client
    IpcSegment         *m_PtrSeg;                           // segment list comprising incoming message
    Datum              *m_PtrCur;                           // pointer to Datum containing current message
    Datum               m_Rsp;                              // Datum containing response
    IpcDecoder          m_Decoder;                          // message decoder object
};

}   // namespace cp

#endif  // CP_IPCPACKET_H
