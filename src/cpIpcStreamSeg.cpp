// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcStreamSeg.cpp
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

#include "cpIpcStreamSeg.h"

namespace cp
{

// constructor
IpcStreamSeg::IpcStreamSeg(size_t Size) :
    m_BlockCount(0),
    m_PtrHead(NULL),
    m_PtrTail(NULL)
{
    MemoryAdd(Size);
}


// copy constructor
IpcStreamSeg::IpcStreamSeg(IpcStreamSeg &rhs) :
    m_BlockCount(0),
    m_PtrHead(NULL),
    m_PtrTail(NULL)
{
    // invoke the assignment operator
    *this = rhs;
}


// destructor
IpcStreamSeg::~IpcStreamSeg()
{
    // deallocates segments
    MemoryFree();
}


// overridden assignment operator
IpcStreamSeg &IpcStreamSeg::operator=(IpcStreamSeg &rhs)
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
IpcStreamSeg &IpcStreamSeg::operator=(Buffer const &rhs)
{
    Clear();
    Write(rhs, rhs.LenGet());

    return *this;
}


// inject an external segment
void IpcStreamSeg::SegmentInject(IpcSegment *pSeg)
{
    // remove any existing memory blocks
    Clear();

    if (pSeg != NULL)
    {
        IpcSegment *ptr = NULL;

        // assign the head and tail
        m_PtrHead = pSeg;
        ++m_BlockCount;

        // locate the tail
        m_PtrTail = m_PtrHead;
        ptr = m_PtrHead->NextGet();

        while (ptr)
        {
            ++m_LastBlock;
            ++m_BlockCount;
            m_PtrTail = ptr;
            ptr = ptr->NextGet();
        }

        // set the position in the final block
        m_LastPos = m_PtrTail->DataLen();
    }
}


// extract the block storage
void IpcStreamSeg::SegmentExtract(IpcSegment *&pSeg)
{
    pSeg = m_PtrHead;

    m_PtrHead = NULL;
    m_PtrTail = NULL;

    Clear();
}


// prepare for transmit
void IpcStreamSeg::Finalize()
{
    if (m_PtrTail != NULL)
    {
        // set the actual length of the last segment
        m_PtrTail->DataLen(m_LastPos);
        m_PtrTail->Buf().LenSet(m_LastPos + IpcSegment::seg_Data);

        // set the opt_Initial flag on the highest numbered segment
        // if more than one segment was needed to hold the payload
        // this informs the receive accumulator how many packets
        // it should expect for a complete transmission
        if (m_PtrHead != m_PtrTail)
        {
            m_PtrTail->Options(m_PtrTail->Options() | IpcSegment::opt_Initial);
        }
    }
}


// free any allocated storage
void IpcStreamSeg::MemoryFree()
{
    // empty the buffer collection
    if (m_PtrHead)
    {
        delete m_PtrHead;
    }

    // clear the block storage
    m_BlockCount = 0;
    m_PtrHead = NULL;
    m_PtrTail = NULL;
}


// add memory to the stream
bool IpcStreamSeg::MemoryAdd(size_t Size)
{
    bool rv = false;
    IpcSegment *ptr = NULL;
    size_t i = Size / BlockSize(0);

    if (Size % BlockSize(0))
    {
        ++i;
    }

    while (i)
    {
        ptr = new (CP_NEW) IpcSegment;
        rv = (ptr != NULL);

        if (ptr)
        {
            // update the block count
            ++m_BlockCount;

            if (m_PtrHead == NULL)
            {
                // add first segment to list
                m_PtrHead = ptr;
                m_PtrTail = ptr;

                // copy template segment fields to new segment
                *ptr = m_Template;
            }
            else
            {
                // set the multi-part flag when collection transitions past one segment
                if (m_PtrHead == m_PtrTail)
                {
                    m_PtrHead->Options(m_PtrHead->Options() | IpcSegment::opt_Multipart);
                }

                // copy segment fields to new segment
                *ptr = *m_PtrTail;

                // mark previous tail segment as full of data
                m_PtrTail->DataLen(BlockSize(0));
                m_PtrTail->Buf().LenSet(BlockSize(0) + IpcSegment::seg_Data);

                // add new segment to the list
                m_PtrTail->NextSet(ptr);
                m_PtrTail = ptr;
            }

            // set the block number
            ptr->FragNum(m_BlockCount);
        }
        else
        {
            cp::LogErr << "IpcStreamSeg::MemoryAdd(): Failed to create an IpcSegment, instance: "
                       << this << std::endl;
            break;
        }

        --i;
    }

    return rv;
}


// returns true if stream has some memory
bool IpcStreamSeg::MemoryChk() const
{
    return (m_PtrHead != NULL);
}


// returns true if block is valid
bool IpcStreamSeg::ValidBlock(size_t Block) const
{
    return (Block < m_BlockCount);
}


// returns block's memory pointer
char *IpcStreamSeg::BlockMemPtr(size_t Block) const
{
    char *pBuf = NULL;
    IpcSegment *pSeg = m_PtrHead;
    size_t seg = Block;

    // seek to the current block
    while (seg && pSeg)
    {
        pSeg = pSeg->NextGet();
        --seg;
    }

    // get the buffer
    if (pSeg && (seg == 0))
    {
        pBuf = pSeg->Buf().c_str(IpcSegment::seg_Data);
    }

    return pBuf;
}


// returns specified memory block size
size_t IpcStreamSeg::BlockSize(size_t Block) const
{
    (void)Block;
    return IpcSegment::Capacity();
}

}   // namespace cp
