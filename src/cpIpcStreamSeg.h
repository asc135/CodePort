// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcStreamSeg.h
//
//  Description:    IPC message segment based stream buffer.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
//  2012-12-14  asc Added copy constructor and assignment operator.
//  2013-04-22  asc Added conversion assignment operator from Buffer.
//  2013-08-29  asc Refactored Clear() operation to eliminate inheritance pitfalls.
// ----------------------------------------------------------------------------

#ifndef CP_IPCSTREAMSEG_H
#define CP_IPCSTREAMSEG_H

#include "cpIpcSegment.h"
#include "cpStreamBase.h"

namespace cp
{

// ----------------------------------------------------------------------------

// the stream segment class
class IpcStreamSeg : public StreamBase
{
public:
    // constructor
    IpcStreamSeg(size_t Size = 0);

    // copy constructor
    IpcStreamSeg(IpcStreamSeg &rhs);

    // destructor
    ~IpcStreamSeg();

    // operators
    IpcStreamSeg &operator=(IpcStreamSeg &rhs);             // overridden assignment operator
    IpcStreamSeg &operator=(Buffer const &rhs);             // conversion assignment operator

    // accessors
    IpcSegment const *ListHead() { return m_PtrHead; }      // return pointer to the head segment

    // manipulators

    // custom methods
    void SegmentInject(IpcSegment *pSeg);                   // inject an external segment
    void SegmentExtract(IpcSegment *&pSeg);                 // extract the block storage
    void Finalize();                                        // prepare for transmit
    void Template(IpcSegment const &Seg)
        { m_Template = Seg; }                               // set the segment field template

protected:
    virtual void MemoryFree();                              // free any allocated storage
    virtual bool MemoryAdd(size_t Size);                    // add memory to the stream
    virtual bool MemoryChk() const;                         // returns true if stream has some memory
    virtual bool ValidBlock(size_t Block) const;            // returns true if block is valid
    virtual char *BlockMemPtr(size_t Block) const;          // returns block's memory pointer
    virtual size_t BlockSize(size_t Block) const;           // returns specified memory block size

private:
    size_t              m_BlockCount;                       // number of comm segments in list
    IpcSegment          m_Template;                         // template for segment field values
    IpcSegment         *m_PtrHead;                          // head to list of comm segments
    IpcSegment         *m_PtrTail;                          // tail to list of comm segments
};

}   // namespace cp

#endif  // CP_IPCSTREAMSEG_H
