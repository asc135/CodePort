// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcSegment.cpp
//
//  Description:    IPC message segment.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
// ----------------------------------------------------------------------------

#include "cpIpcSegment.h"
#include "cpUtil.h"

namespace cp
{

// performance instrumentation
uint32_t g_Created = 0;
uint32_t g_Deleted = 0;
Mutex g_UsageMutex("Usage Counter Mutex");


// constructor
IpcSegment::IpcSegment() :
    m_Buffer(seg_MaxLen),
    m_NextSeg(NULL)
{
    // initialize the fields
    Clear();

    // update the debug instrumentation
    IncCreate();
}


// copy constructor
IpcSegment::IpcSegment(IpcSegment const &rhs) :
    m_Buffer(seg_MaxLen),
    m_NextSeg(NULL)
{
    // invoke assignment operator
    *this = rhs;

    // update the debug instrumentation
    IncCreate();
}


// destructor
IpcSegment::~IpcSegment()
{
    // protected from recursion
    if (m_NextSeg)
    {
        PurgeList();
    }

    // udpate the debug instrumentation
    IncDelete();
}


// returns the segment version
uint8_t IpcSegment::Ver() const
{
    return *m_Buffer.u_str(seg_Version);
}


// returns the options field
uint8_t IpcSegment::Options() const
{
    return *m_Buffer.u_str(seg_Options) & 0xfc;
}


// returns the transmit priority field
uint8_t IpcSegment::Priority() const
{
    return *m_Buffer.u_str(seg_Options) & 0x03;
}


// returns the fragment number
uint16_t IpcSegment::FragNum() const
{
    return ReadUint16B(m_Buffer.u_str(seg_FragNum));
}


// returns the source address
uint32_t IpcSegment::SrcAddr() const
{
    return ReadUint32B(m_Buffer.u_str(seg_SrcAddr));
}


// returns the destination address
uint32_t IpcSegment::DstAddr() const
{
    return ReadUint32B(m_Buffer.u_str(seg_DstAddr));
}


// returns the message identification code
uint32_t IpcSegment::MsgId() const
{
    return ReadUint32B(m_Buffer.u_str(seg_MsgId));
}


// returns the message context value
uint32_t IpcSegment::Context() const
{
    return ReadUint32B(m_Buffer.u_str(seg_Context));
}


// returns the message type code
uint8_t IpcSegment::MsgType() const
{
    return *m_Buffer.u_str(seg_MsgType);
}


// returns the message control code value
uint8_t IpcSegment::CtlCode() const
{
    return *m_Buffer.u_str(seg_CtlCode);
}


// returns the payload data length
size_t IpcSegment::DataLen() const
{
    return ReadUint16B(m_Buffer.u_str(seg_DataLen));
}


// returns the first long word in the payload buffer
uint32_t IpcSegment::DataGet() const
{
    return ReadUint32B(m_Buffer.u_str(seg_Data));
}


// returns the payload data
bool IpcSegment::DataGet(Buffer &Buf) const
{
    return Buf.CopyIn(m_Buffer.u_str(seg_Data), DataLen());
}


// returns the globally unique ID (src addr + msg id)
uint64_t IpcSegment::Guid() const
{
    uint64_t guid;

    guid = SrcAddr();
    guid <<= 32;
    guid |= MsgId();

    return guid;
}


// hex dump the segment contents to output stream
void IpcSegment::Dump(std::ostream &Stream) const
{
    IpcSegment const *pSeg = this;

    while (pSeg)
    {
        LogMsg << "IpcSegment: " << this << std::endl;
        HexDump(Stream, pSeg->m_Buffer);
        pSeg = pSeg->NextGet();
    }
}


// sets the options field
void IpcSegment::Options(uint8_t Data)
{
    uint8_t opt = *m_Buffer.u_str(seg_Options);

    // preserve the priority bits
    opt &= 0x03;

    // apply the options bits
    opt |= (Data & 0xfc);

    *m_Buffer.u_str(seg_Options) = opt;
}


// sets the transmit priority field
void IpcSegment::Priority(uint8_t Data)
{
    uint8_t pri = *m_Buffer.u_str(seg_Options);

    // preserve the option bits
    pri &= 0xfc;

    // apply the priority bits
    if (Data > k_IpcMinimumPriority)
    {
        Data = k_IpcMinimumPriority;
    }

    pri |= (Data & 0x03);

    *m_Buffer.u_str(seg_Options) = pri;
}


// sets the fragment number
void IpcSegment::FragNum(uint16_t Val)
{
    WriteUint16B(Val, m_Buffer.u_str(seg_FragNum));
}


// sets the source address
void IpcSegment::SrcAddr(uint32_t Addr)
{
    WriteUint32B(Addr, m_Buffer.u_str(seg_SrcAddr));
}


// sets the destination address
void IpcSegment::DstAddr(uint32_t Addr)
{
    WriteUint32B(Addr, m_Buffer.u_str(seg_DstAddr));
}


// sets the message identification code
void IpcSegment::MsgId(uint32_t Val)
{
    WriteUint32B(Val, m_Buffer.u_str(seg_MsgId));
}


// sets the message context value
void IpcSegment::Context(uint32_t Val)
{
    WriteUint32B(Val, m_Buffer.u_str(seg_Context));
}


// sets the message type code
void IpcSegment::MsgType(uint8_t Val)
{
    *m_Buffer.u_str(seg_MsgType) = Val;
}


// sets the message control code value
void IpcSegment::CtlCode(uint8_t Val)
{
    *m_Buffer.u_str(seg_CtlCode) = Val;
}


// sets the payload data length
void IpcSegment::DataLen(size_t Len)
{
    size_t val = (Len < Capacity()) ? Len : Capacity();
    WriteUint16B(val, m_Buffer.u_str(seg_DataLen));
}


// stores a long word as the payload
bool IpcSegment::DataSet(uint32_t Value)
{
    char val[sizeof(uint32_t)];

    // do this to make sure it is in network byte order
    WriteUint32B(Value, val);
    return DataSet(val, sizeof(val));
}


// stores the payload data
bool IpcSegment::DataSet(Buffer const &Data)
{
    return DataSet(Data.c_str(), Data.LenGet());
}


// stores the payload data
bool IpcSegment::DataSet(char const *pBuf, size_t Len)
{
    // copy only up to maximum segment capacity
    size_t copyLen = (Len < Capacity() ? Len : Capacity());

    // copy the data
    memcpy(m_Buffer.c_str(seg_Data), pBuf, copyLen);

    // update the header data length field
    DataLen(copyLen);

    // update the segment buffer length
    m_Buffer.LenSet(seg_Data + copyLen);

    // return true if data fit in the segment payload area
    return (Len == copyLen);
}


// sets the segment contents to default values
void IpcSegment::Clear()
{
    m_Buffer.Clear();

    // set the header length in the buffer
    m_Buffer.LenSet(seg_Data);

    // set the header version information
    *m_Buffer.u_str(seg_Version) = 0;

    // set the default communications priority
    Priority(k_IpcDefaultPriority);
}


// overridden assignment operator
IpcSegment &IpcSegment::operator=(IpcSegment const &rhs)
{
    // check for self assignment
    if (this == &rhs)
    {
        return *this;
    }

    // resize the buffer storage
    m_Buffer.Resize(seg_MaxLen);

    // make sure there is a valid source buffer
    if (rhs.m_Buffer.Size() >= rhs.m_Buffer.LenGet())
    {
        // copy the header data fields
        m_Buffer.LenSet(rhs.m_Buffer.LenGet());
        memcpy(m_Buffer.c_str(), rhs.m_Buffer.c_str(), m_Buffer.LenGet());
    }

    return *this;
}


// purge the linked list of attached segments
void IpcSegment::PurgeList()
{
    IpcSegment *pSeg = NextGet();
    IpcSegment *pNext;

    // delete all linked headers
    while (pSeg != NULL)
    {
        // get next list item
        pNext = pSeg->NextGet();

        // prevent destructor recursion
        pSeg->NextSet(NULL);

        // delete the object
        delete pSeg;

        // iterate
        pSeg = pNext;
    }

    // clear the next pointer
    NextSet(NULL);
}

}   // namespace cp
