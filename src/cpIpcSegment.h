// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcSegment.h
//
//  Description:    IPC message segment.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
//  2013-08-20  asc Redesigned control code mechanism.
// ----------------------------------------------------------------------------

#ifndef CP_IPCSEGMENT_H
#define CP_IPCSEGMENT_H

#include "cpMutex.h"
#include "cpBuffer.h"
#include "cpPooledBase.h"

namespace cp
{

// this is diagnostic instrumentation
extern uint32_t g_Created;
extern uint32_t g_Deleted;
extern Mutex g_UsageMutex;

// ----------------------------------------------------------------------------

class IpcSegment : public PooledBase
{
public:
    enum IpcSegOptions
    {
        opt_Priority0   = 0x01,
        opt_Priority1   = 0x02,
        opt_Unused0     = 0x04,
        opt_Unused1     = 0x08,
        opt_Unused2     = 0x10,
        opt_Multipart   = 0x20,
        opt_Initial     = 0x40,
        opt_Control     = 0x80
    };

    // all multi-octet fields are in big-endian format, MSB at lowest address
    enum IpcSegFields
    { // name             offset   field-len   description
        seg_Version     = 0x0000,  // 1 - this represents the segment structure version
        seg_Options     = 0x0001,  // 1 - option flags (see enum above)
        seg_FragNum     = 0x0002,  // 2 - fragment number if multipart
        seg_SrcAddr     = 0x0004,  // 4 - source node address
        seg_DstAddr     = 0x0008,  // 4 - destination node address
        seg_MsgId       = 0x000c,  // 4 - node-unique message ID
        seg_Context     = 0x0010,  // 4 - message ID of original if this is a reply, 0 if unsolicited
        seg_MsgType     = 0x0014,  // 1 - message type code
        seg_CtlCode     = 0x0015,  // 1 - message control code
        seg_DataLen     = 0x0016,  // 2 - payload length in octets
        seg_Data        = 0x0018,  // start of payload
        seg_MaxLen      = 0x0400   // 1024 octets maximum segment size including fields and payload
    };

    // IPC segment communications priority
    enum IpcSegPriority
    {
        pri_Uber = 0,
        pri_High = 1,
        pri_Med  = 2,
        pri_Low  = 3
    };

    // message type codes
    enum IpcMsgTypes { msg_Raw, msg_Datum, msg_Control };

    // message control codes
    enum IpcControlCodes
    {
        ctl_NoOp        = 0x00,
        ctl_Shutdown    = 0x01,
        ctl_Cancel      = 0x02,
        ctl_Reset       = 0x03,
        ctl_Suspend     = 0x04,
        ctl_Resume      = 0x05,
        ctl_TraceOn     = 0x06,
        ctl_TraceOff    = 0x07,
        ctl_WatchDog    = 0x08,
        ctl_FlushAddr   = 0x09,
        ctl_StartSync   = 0x0a,
        ctl_Extended    = 0xff
    };

    // constructor
    IpcSegment();

    // copy constructor
    IpcSegment(IpcSegment const &rhs);

    // destructor
    ~IpcSegment();

    // accessors
    uint8_t Ver() const;                                    // returns the segment version
    uint8_t Options() const;                                // returns the options field
    uint8_t Priority() const;                               // returns the transmit priority field
    uint16_t FragNum() const;                               // returns the fragment number
    uint32_t SrcAddr() const;                               // returns the source address
    uint32_t DstAddr() const;                               // returns the destination address
    uint32_t MsgId() const;                                 // returns the message identification code
    uint32_t Context() const;                               // returns the message context value
    uint8_t MsgType() const;                                // returns the message type code
    uint8_t CtlCode() const;                                // returns the message control code
    size_t DataLen() const;                                 // returns the payload data length

    uint32_t DataGet() const;                               // returns the first long word in the payload buffer
    bool DataGet(Buffer &Data) const;                       // returns the payload data
    Buffer &Buf() { return m_Buffer; }                      // returns the internal buffer
    Buffer const &Buf() const { return m_Buffer; }          // returns the internal buffer as const

    size_t SegLen() const { return seg_Data + DataLen(); }  // returns the segment length
    uint64_t Guid() const;                                  // returns the globally unique ID (src addr + msg id)
    void Dump(std::ostream &Stream) const;                  // hex dump the segment contents to output stream
    static size_t Capacity()
        { return (seg_MaxLen - seg_Data); }                 // returns the maximum payload capacity

    // manipulators
    void Options(uint8_t Data);                             // sets the options field
    void Priority(uint8_t Data);                            // sets the transmit priority field
    void FragNum(uint16_t Val);                             // sets the fragment number
    void SrcAddr(uint32_t Addr);                            // sets the source address
    void DstAddr(uint32_t Addr);                            // sets the destination address
    void MsgId(uint32_t Val);                               // sets the message identification code
    void Context(uint32_t Val);                             // sets the message context value
    void MsgType(uint8_t Val);                              // sets the message type code
    void CtlCode(uint8_t Val);                              // sets the message control code
    void DataLen(size_t Len);                               // sets the payload data length

    bool DataSet(uint32_t Value);                           // stores a long word as the payload
    bool DataSet(Buffer const &Data);                       // stores the payload data
    bool DataSet(char const *pBuf, size_t Len);             // stores the payload data

    void Clear();                                           // sets the segment contents to default values

    // operators
    IpcSegment &operator=(IpcSegment const &rhs);           // overridden assignment operator

    // list accessors
    IpcSegment *NextGet() { return m_NextSeg; }
    IpcSegment *NextGet() const { return m_NextSeg; }
    void NextSet(IpcSegment *pSeg) { m_NextSeg = pSeg; }
    void PurgeList();                                       // purge the linked list of attached segments

    // instrumentation
    static void Stats()
    {
        g_UsageMutex.Lock();
        LogMsg << "*** IpcSegments Created = " << g_Created << " Deleted = " << g_Deleted << std::endl;
        g_UsageMutex.Unlock();
    }

private:

    void IncCreate()
    {
        g_UsageMutex.Lock();
        ++g_Created;
        g_UsageMutex.Unlock();
    }

    void IncDelete()
    {
        g_UsageMutex.Lock();
        ++g_Deleted;
        g_UsageMutex.Unlock();
    }

    Buffer              m_Buffer;                           // the message segment buffer
    IpcSegment         *m_NextSeg;                          // the next segment in a linked set
};

}   // namespace cp

#endif  // CP_IPCSEGMENT_H

