// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpStreamBuf.cpp
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

#include "cpStreamBuf.h"
#include "cpBuffer.h"
#include "cpMemMgr.h"
#include "cpUtil.h"

namespace cp
{

// constructor
StreamBuf::StreamBuf(size_t Size)
{
    MemoryAdd(Size);
}


// copy constructor
StreamBuf::StreamBuf(StreamBuf &rhs)
{
    // invoke the assignment operator
    *this = rhs;
}


// destructor
StreamBuf::~StreamBuf()
{
    // deallocates buffers
    MemoryFree();
}


// overridden assignment operator
StreamBuf &StreamBuf::operator=(StreamBuf &rhs)
{
    // check for self assignment
    if (this == &rhs)
    {
        return *this;
    }

    Buffer buf;

    buf = rhs;
    *this = buf;

    return *this;
}


// conversion assignment operator
StreamBuf &StreamBuf::operator=(Buffer const &rhs)
{
    Clear();
    Write(rhs, rhs.LenGet());

    return *this;
}


// transfer memory blocks from src
void StreamBuf::TransferBlocksFrom(StreamBuf &src)
{
    Clear();
    m_VecBlocks = src.m_VecBlocks;
    // empty src vector otherwise src.Clear() will deallocate the blocks
    src.m_VecBlocks.clear();
    m_CurBlock = src.m_CurBlock;
    m_CurPos = src.m_CurPos;
    m_LastBlock = src.m_LastBlock;
    m_LastPos = src.m_LastPos;
    src.Clear();
}


// free any allocated storage
void StreamBuf::MemoryFree()
{
    MemBlock *pMem = NULL;

    // empty the buffer collection
    while (m_VecBlocks.size())
    {
        pMem = m_VecBlocks.back();

        // return the block to the pool
        if (MemManager::InstanceGet()->MemBlockPut(pMem) == false)
        {
            LogErr << "StreamBuf::MemoryFree(): Failed to return a MemBlock: "
                   << pMem << ", instance: " << this << std::endl;
        }

        // remove it from the vector
        m_VecBlocks.pop_back();
    }
}


// add memory to the stream
bool StreamBuf::MemoryAdd(size_t Size)
{
    bool rv = false;
    MemBlock *pMem = NULL;
    size_t blkSize = k_MinStreamBlockSize;

    if (Size > blkSize)
    {
        blkSize = Size;
    }

    rv = MemManager::InstanceGet()->MemBlockGet(pMem, blkSize);

    if (rv)
    {
        m_VecBlocks.push_back(pMem);
    }
    else
    {
        LogErr << "StreamBuf::MemBlkAdd(): Failed to acquire a memory block of size: "
               << blkSize << ", instance: " << this << std::endl;
    }

    return rv;
}


// returns true if stream has some memory
bool StreamBuf::MemoryChk() const
{
    return (m_VecBlocks.size() > 0);
}


// returns true if block is valid
bool StreamBuf::ValidBlock(size_t Block) const
{
    return (Block < m_VecBlocks.size());
}


// returns block's memory pointer
char *StreamBuf::BlockMemPtr(size_t Block) const
{
    char *ptr = NULL;

    if (ValidBlock(Block))
    {
        ptr = m_VecBlocks[Block]->BuffGet();
    }

    return ptr;
}


// returns specified memory block size
size_t StreamBuf::BlockSize(size_t Block) const
{
    size_t rv = 0;

    if (Block < m_VecBlocks.size())
    {
        rv = m_VecBlocks[Block]->SizeGet();
    }

    return rv;
}

}   // namespace cp
