// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpStreamBuf.h
//
//  Description:    Stream Buffer Class.
//
//  Platform:       common
//
//  History:
//  2012-05-01  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-04-22  asc Added conversion assignment operator from Buffer.
//  2013-08-29  asc Refactored Clear() operation to eliminate inheritance pitfalls.
//  2022-03-02  asc Added TransferBlocks() method.
// ----------------------------------------------------------------------------
#ifndef CP_STREAMBUF_H
#define CP_STREAMBUF_H

#include "cpStreamBase.h"

namespace cp
{

// forward declarations
class Buffer;
class MemBlock;

// local custom types
typedef std::vector<MemBlock *, Alloc<MemBlock *> > StrmBufMem_t;

// ----------------------------------------------------------------------------

// the stream buffer class
class StreamBuf : public StreamBase
{
public:
    // constructor
    StreamBuf(size_t Size = 0);

    // copy constructor
    StreamBuf(StreamBuf &rhs);

    // destructor
    virtual ~StreamBuf();

    // operators
    StreamBuf &operator=(StreamBuf &rhs);                   // overridden assignment operator
    StreamBuf &operator=(Buffer const &rhs);                // conversion assignment operator

    // accessors
    void TransferBlocksFrom(StreamBuf &src);                // transfer memory blocks from src

    // manipulators

protected:
    virtual void MemoryFree();                              // free any allocated storage
    virtual bool MemoryAdd(size_t Size);                    // add memory to the stream
    virtual bool MemoryChk() const;                         // returns true if stream has some memory
    virtual bool ValidBlock(size_t Block) const;            // returns true if block is valid
    virtual char *BlockMemPtr(size_t Block) const;          // returns block's memory pointer
    virtual size_t BlockSize(size_t Block) const;           // returns specified memory block size

private:
    StrmBufMem_t        m_VecBlocks;                        // vector of memory blocks
};

}   // namespace cp

#endif // CP_STREAMBUF_H
