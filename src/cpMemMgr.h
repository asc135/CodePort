// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpMemMgr.h
//
//  Description:    Managed Memory Pool Facility.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef  CP_MEMMGR_H
#define  CP_MEMMGR_H

#include "cpPlatform.h"
#include "cpMemMutex_I.h"

// ----------------------------------------------------------------------------
// In this design, a pool manager provides an access API to clients that need
// memory buffers.  When a memory request is received, it looks in its
// inventory of pools to find one that manages blocks that are the smallest
// size that can still satisfy the request.  The request is delegated to the
// appropriate pool which looks in its inventory of memory segments to find
// one that has a free block available.  If no free blocks are found, a new
// segment is allocated and a memory block is returned.
// ----------------------------------------------------------------------------

namespace cp
{

// Memory Block - This represents a unit of memory delivered to the client.
//
class MemBlock
{
public:
    // constructor
    MemBlock(size_t BlockSize);

    // destructor
    ~MemBlock();

    // accessor methods
    bool Valid() const                                      // determine if block has a valid header
    {
        return (m_Guard == k_MemSentinel);
    }

    char *BuffGet() const                                   // accessor for data buffer
    {
        return ((char *)this) + sizeof(MemBlock);
    }

    size_t SizeGet() const                                  // accessor for data buffer's size
    {
        return m_BlockSize;
    }

    void StatusLog(std::ostream &Out) const;                // log block usage statistics

    // manipulators
    void Clear(int Val = 0);                                // zero or fill the memory buffer

    void GuardOn() { m_Guard = k_MemSentinel; }             // activate sentinel

    void GuardOff() { m_Guard = 0; }                        // deactivate sentinel

    void UseCountInc()                                      // increment the usage counter
    {
        if (++m_UseCount == 0)
        {
            --m_UseCount;
        }
    }

    // embedded list accessors
    void NextSet(MemBlock *Blk)                             // put pointer to next block in the list
    {
        *(MemBlock **)(BuffGet()) = Blk;
    }

    MemBlock *NextGet()                                     // get pointer to next block in the list
    {
        return *(MemBlock **)(BuffGet());
    }

private:
    uint16_t            m_Guard;                            // guard sentinel to detect memory corruption
    uint16_t            m_UseCount;                         // counts number of times this block has been deployed
    size_t              m_BlockSize;                        // the size of the data buffer
};

//-----------------------------------------------------------------------------

// Memory Segment - This represents a list of allocations of memory from which
//                  memory blocks are created.  Each element is the minimum amount of
//                  memory that a pool allocates from the heap.  This is instantiated
//                  at the start a memory area allocated for dividing into blocks.
class MemSegment
{
public:
    MemSegment();
    ~MemSegment();

    // embedded list accessors
    void NextSet(MemSegment *Seg)                           // set pointer to next segment in the list
    {
        m_Next = Seg;
    }

    MemSegment *NextGet()                                   // get pointer to next segment in the list
    {
        return m_Next;
    }

private:
    MemSegment         *m_Next;                             // pointer to next segment in list
};

//-----------------------------------------------------------------------------

// Memory Pool - This represents a collection of one or more segments from which memory blocks are
//               distributed to a client.  A given pool is characterized by a uniform memory
//               block size and is owned by the memory manager.  The manager delegates block
//               requests to a pool and returns blocks back to the pool when the client
//               returns them.
class MemPool
{
public:
    // constructor
    MemPool(size_t BlockSize,
              uint32_t InitCount,
              uint32_t Increment);

    // destructor
    ~MemPool();

    // accessor methods
    bool MemBlockGet(MemBlock * &Mem);                      // get a block of memory from the pool
    bool MemBlockPut(MemBlock * &Mem);                      // return a block of memory to the pool

    size_t SizeGet()                                        // get the block size managed by this pool
    {
        return m_BlockSize;
    }

    void StatusLog(std::ostream &Out) const;                // log block usage statistics

    // embedded list accessors
    void NextSet(MemPool *Pool)                             // put pointer to next pool in the list
    {
        m_Next = Pool;
    }

    MemPool *NextGet()                                      // get pointer to next pool in the list
    {
        return m_Next;
    }

private:
    bool AllocateSegment(uint32_t NumBlocks);               // allocate another memory segment

    size_t              m_BlockSize;                        // the size of the pooled memory blocks
    uint32_t            m_Increment;                        // number of blocks a subsequently allocated segment must hold
    uint32_t            m_TotalBlocks;                      // total number of blocks managed by this pool
    uint32_t            m_Inventory;                        // total number of unused blocks current in inventory
    uint32_t            m_PeakUsed;                         // highest number of blocks ever deployed
    MemSegment         *m_SegHead;                          // list of allocated segments
    MemBlock           *m_BlkHead;                          // first entry in block inventory
    MemBlock           *m_BlkTail;                          // last entry in block inventory
    MemPool            *m_Next;                             // pointer to next pool in list
};

//-----------------------------------------------------------------------------

// Memory Manager - This is the external interface to the application.  All memory requests are sent
//                  to the memory manager.  One or more memory pools, each representing a different
//                  block size, are owned by the manager.  The memory manager directs a memory request
//                  to an appropriate pool.  Typically the pool is selected based on the smallest standard
//                  size that meets the memory requested.  Returned blocks are directed back to their
//                  size-matched pool.
//
//                  In the case of a block size that is larger than any managed by the existing pools, a
//                  standard heap allocation will result.  The return of such a block will result in its
//                  deallocatin back to the heap.
class MemManager
{
public:
    // destructor
    ~MemManager();

    // accessors
    bool MemBlockGet(MemBlock * &Mem, size_t Size);         // get a block of memory
    bool MemBlockPut(MemBlock * &Mem);                      // return a block
    bool MemBlockPut(char * &Mem);                          // return a block using its buffer pointer

    void StatusLog(std::ostream &Out);                      // get block usage statistics

    // manipulators
    bool CreatePool(size_t BlockSize,
                    uint32_t InitCount,
                    uint32_t Increment);                    // create a new memory pool

    // static accessors
    static MemManager *InstanceGet();                       // static accessor of singleton

private:
    // private constructor (for singleton pattern)
    MemManager();
    MemPool *PoolGet(size_t BlockSize);                     // return a pool of sufficient block size

    uint32_t            m_DeployedCount;                    // cumulative number of buffers deployed
    uint32_t            m_ReturnedCount;                    // cumulative number of buffers returned
    uint32_t            m_FailedGets;                       // cumulative number of failed get requests
    uint32_t            m_FailedPuts;                       // cumulative number of failed put requests
    uint32_t            m_DeployedSize;                     // number of bytes currently deployed
    MemPool            *m_PoolHead;                         // list of memory pools
    MemMutex            m_Mutex;                            // thread protection mutex

    // static member data
    static MemManager *pInstance;                           // static singleton instance
};

//-----------------------------------------------------------------------------

}   // namespace cp

#endif  // CP_MEMMGR_H
