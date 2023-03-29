// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpMemMgr.cpp
//
//  Description:    Managed Memory Pool Facility.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-11-28  asc Eliminated MemManager destructor due to static initializer issue.
//  2023-03-28  asc Added mutex protection to CreatePool() so it can be used by application.
//  2023-03-28  asc Added parameter to control block level details in StatusLog().
// ----------------------------------------------------------------------------

#include "cpMemMgr.h"

namespace cp
{

// initialize static singleton instance
MemManager *MemManager::pInstance = NULL;


// ============================================================================
// MemBlock
// ============================================================================


// constructor
MemBlock::MemBlock(size_t BlockSize) :
    m_Guard(0),
    m_UseCount(0),
    m_BlockSize(BlockSize)
{
}


// destructor
MemBlock::~MemBlock()
{
}


// zero or fill the memory buffer
void MemBlock::Clear(int Val)
{
    memset(BuffGet(), Val, m_BlockSize);
}


// log block usage statistics
void MemBlock::StatusLog(std::ostream &Out) const
{
    // display any buffers that have been used and returned
    if (m_UseCount > 0)
    {
        Out << "   Block:  " << this << "    Deployed Count:  "
            << std::setfill(' ') << std::setw(6) << m_UseCount << std::endl;
    }
}


// ============================================================================
// MemSegment
// ============================================================================


// constructor
MemSegment::MemSegment() : m_Next(NULL)
{
}


// destructor
MemSegment::~MemSegment()
{
}


// ============================================================================
// MemPool
// ============================================================================


// constructor
MemPool::MemPool(size_t BlockSize, uint32_t InitCount, uint32_t Increment) :
    m_BlockSize(BlockSize),
    m_Increment(Increment),
    m_TotalBlocks(0),
    m_Inventory(0),
    m_PeakUsed(0),
    m_SegHead(NULL),
    m_BlkHead(NULL),
    m_BlkTail(NULL),
    m_Next(NULL)
{
   if (AllocateSegment(InitCount) == false)
   {
       LogErr << "MemPool::MemPool(): Failed to create initial memory segment for size: "
              << m_BlockSize << std::endl;
   }
}


// destructor
MemPool::~MemPool()
{
    MemSegment *pSeg = m_SegHead;

    // iterate through list and delete all segments
    while (pSeg != NULL)
    {
        m_SegHead = pSeg->NextGet();
        delete [] reinterpret_cast<char *>(pSeg);
        pSeg = m_SegHead;
    }
}


// get a block of memory from the pool
bool MemPool::MemBlockGet(MemBlock * &Mem)
{
    bool rv = true;

    // check block inventory
    if (m_BlkHead == NULL)
    {
        // need to allocate a new segment
        rv = AllocateSegment(m_Increment);

        if (rv == false)
        {
            LogErr << "MemPool::MemBlockGet(): Failed to create incremental memory segment for size: "
                   << m_BlockSize << std::endl;
        }
    }

    if (rv)
    {
        Mem = m_BlkHead;

        // if a block is found in the block list, use it and remove it from the list
        if (Mem != NULL)
        {
            m_BlkHead = Mem->NextGet();

            // make sure tail is cleared if last block is being removed
            if (m_BlkHead == NULL)
            {
                m_BlkTail = NULL;
            }

            // increment the block's usage count
            Mem->UseCountInc();

            // update inventory
            --m_Inventory;

            // update peak usage
            if ((m_TotalBlocks - m_Inventory) > m_PeakUsed)
            {
                m_PeakUsed = (m_TotalBlocks - m_Inventory);
            }
        }
        else
        {
            rv = false;
            LogErr << "MemPool::MemBlockGet(): Failed to locate a free block for size: "
                   << m_BlockSize << std::endl;
        }
    }

    return rv;
}


// return a block of memory to the pool
bool MemPool::MemBlockPut(MemBlock * &Mem)
{
    bool rv = true;

    // check that block is a valid instance
    if (Mem == NULL)
    {
        rv = false;
    }

    // check that block matches this pool's block size
    if (rv)
    {
        if (Mem->SizeGet() != m_BlockSize)
        {
            rv = false;
        }
    }

    // return block to the free block list
    if (rv)
    {
        if (m_BlkHead == NULL)
        {
            // set both tail and head point to the only block
            m_BlkHead = Mem;
            m_BlkTail = Mem;
        }
        else
        {
            // insert block at the tail
            m_BlkTail->NextSet(Mem);
            m_BlkTail = Mem;
        }

        // terminate final list entry
        Mem->NextSet(NULL);

        // clear the pointer reference
        Mem = NULL;

        // update inventory
        ++m_Inventory;
    }

    return rv;
}


// log block usage statistics
void MemPool::StatusLog(std::ostream &Out, bool Blocks) const
{
    Out << "\n";
    Out << "  Pool:  "   << std::setw(8) << m_BlockSize
        << "  Inc:  "    << std::setw(5) << m_Increment
        << "  Blocks:  " << std::setw(5) << m_TotalBlocks
        << "  Inv:  "    << std::setw(5) << m_Inventory
        << "  Peak:  "   << std::setw(5) << m_PeakUsed << std::endl;
    Out << "  ----------------------------------";
    Out << "-------------------------------------" << std::endl;

    if (Blocks)
    {
        MemBlock *pBlock = m_BlkHead;

        while (pBlock != NULL)
        {
            pBlock->StatusLog(Out);
            pBlock = pBlock->NextGet();
        }
    }
}


// allocate a segment that holds specified number of blocks
bool MemPool::AllocateSegment(uint32_t Blocks)
{
    bool rv = true;
    MemBlock *pBlk;
    MemSegment *pSeg;
    char *pRaw;
    size_t segSize = (Blocks * (m_BlockSize + sizeof(MemBlock))) + sizeof(MemSegment);

    if (Blocks == 0)
    {
        rv = false;
    }

    if (rv)
    {
        // allocate a new chunk of memory large enough
        // for all blocks plus the MemSegment object
        pRaw = new (CP_NEW) char[segSize];

        if (pRaw == NULL)
        {
            rv = false;
        }
    }

    if (rv)
    {
        // instantiate a MemSegment in the new chunk of memory
        pSeg = new (pRaw) MemSegment;

        // add the segment to the list
        pSeg->NextSet(m_SegHead);
        m_SegHead = pSeg;

        // index raw pointer past the MemSegment object
        pRaw += sizeof(MemSegment);

        // carve up remaining memory into blocks and put them in the block inventory
        for (uint32_t i = 0; i < Blocks; ++i)
        {
            pBlk = new (pRaw) MemBlock(m_BlockSize);

            rv = MemBlockPut(pBlk);

            if (rv)
            {
                ++m_TotalBlocks;
            }
            else
            {
                LogErr << "MemPool::AllocateSegment(): Failed adding new blocks to inventory for size: "
                       << m_BlockSize << std::endl;
                break;
            }

            // advance raw pointer to start of next block
            pRaw += (sizeof(MemBlock) + m_BlockSize);
        }
    }

    return rv;
}


// ============================================================================
// MemManager
// ============================================================================


// destructor
MemManager::~MemManager()
{
#if 0   // do not destruct -- static initializer race condition
    MemPool *pPool = m_PoolHead;

    // iterate through list and delete all pools
    while (pPool != NULL)
    {
        m_PoolHead = pPool->NextGet();
        delete pPool;
        pPool = m_PoolHead;
    }
#endif
}


// get a block of memory
bool MemManager::MemBlockGet(MemBlock * &Mem, size_t Size)
{
    bool rv = true;
    MemPool *pPool;

    m_Mutex.Lock();

    // find an appropriate pool
    pPool = PoolGet(Size);

    if (pPool != NULL)
    {
        // found a pool so attempt to get a block
        rv = pPool->MemBlockGet(Mem);
    }
    else
    {
        // no appropriate pool, so allocate custom size block
        char *pRaw = new (CP_NEW) char[Size + sizeof(MemBlock)];

        if (pRaw != NULL)
        {
            Mem = new (pRaw) MemBlock(Size);
        }
        else
        {
            rv = false;
            LogErr << "MemManager::MemBlockGet(): Failed to allocate block of custom size: "
                   << Size << std::endl;
        }
    }

    // update statistics
    if (rv)
    {
        ++m_DeployedCount;
        m_DeployedSize += Mem->SizeGet();
    }
    else
    {
        ++m_FailedGets;
    }

    m_Mutex.Unlock();

    // mark for consistency and protection against multiple returns
    Mem->GuardOn();

    return rv;
}


// return a block
bool MemManager::MemBlockPut(MemBlock * &Mem)
{
    bool rv = true;
    MemPool *pPool;
    size_t size = 0;

    // check validity of memory block
    if (Mem == NULL)
    {
        return rv;
    }
    else
    {
        size = Mem->SizeGet();
    }

    // check for integrity and prevent multiple puts
    if (Mem->Valid() == false)
    {
        LogErr << "MemManager::MemBlockPut(): Attempted return of invalid memory block: "
               << Mem << std::endl;
        return false;
    }

    // remove integrity guard to prevent multiple return
    Mem->GuardOff();

    m_Mutex.Lock();

    // find an appropriate pool
    pPool = PoolGet(size);

    if (pPool != NULL)
    {
        // found a pool so attempt to put the block
        rv = pPool->MemBlockPut(Mem);
    }
    else
    {
        // no appropriate pool, so delete the custom size block
        char *pRaw = reinterpret_cast<char *>(Mem);

        if (pRaw != NULL)
        {
            delete [] pRaw;
            Mem = NULL;
        }
    }

    // update statistics
    if (rv)
    {
        ++m_ReturnedCount;
        m_DeployedSize -= size;
    }
    else
    {
        ++m_FailedPuts;
    }

    m_Mutex.Unlock();

    return rv;
}


// return a block using its buffer pointer
bool MemManager::MemBlockPut(char * &Mem)
{
    bool rv;

    if (Mem == NULL)
    {
        return true;
    }

    // derive a pointer to where the header of the buffer should exist
    MemBlock *pBlock = reinterpret_cast<MemBlock *>(Mem - sizeof(MemBlock));

    // check for underflow
    rv = (reinterpret_cast<size_t>(Mem) > reinterpret_cast<size_t>(pBlock));

    // return the memory block
    if (rv)
    {
        rv = MemBlockPut(pBlock);

        if (rv)
        {
            Mem = NULL;
        }
    }

    return rv;
}


// log block usage statistics
void MemManager::StatusLog(std::ostream &Out, bool Blocks)
{
    MemPool *pPool;

    if (m_Mutex.Lock())
    {
        Out << "\nMemManager Status Log\n";
        Out << "-----------------------\n";
        Out << "Total Deployed:  " << m_DeployedCount << "  Returned:  "
            << m_ReturnedCount << "  Deployed Size:  " << m_DeployedSize << "\n";
        Out << "Failed Deploy Count:  " << m_FailedGets << "  Failed Return Count:  "
            << m_FailedPuts << std::endl;

        pPool = m_PoolHead;

        while (pPool != NULL)
        {
            pPool->StatusLog(Out, Blocks);
            pPool = pPool->NextGet();
        }

        m_Mutex.Unlock();
    }
}


// create a new memory pool
bool MemManager::CreatePool(size_t BlockSize, uint32_t Initial, uint32_t Increment)
{
    bool rv = true;

    m_Mutex.Lock();

    MemPool *pPool = m_PoolHead;
    MemPool *pPrevMatch = NULL;

    // enforce minimum block size
    if (BlockSize < k_MinMemBlockSize)
    {
        BlockSize = k_MinMemBlockSize;
    }

    while (pPool != NULL)
    {
        // check if pool of this size already exists
        if (pPool->SizeGet() == BlockSize)
        {
            rv = false;
            break;
        }

        // keep pointer of nearest smaller size pool
        if (pPool->SizeGet() < BlockSize)
        {
            pPrevMatch = pPool;
        }

        // go to next pool, if any
        pPool = pPool->NextGet();
    }

    if (rv)
    {
        // create a new pool
        pPool = new (CP_NEW) MemPool(BlockSize, Initial, Increment);

        if (pPool == NULL)
        {
            rv = false;
            LogErr << "MemManager::CreatePool(): Failed to create pool of size: "
                   << BlockSize << std::endl;
        }
    }

    if (rv)
    {
        // insert new pool into list
        if (pPrevMatch == NULL)
        {
            // this is the smallest pool
            pPool->NextSet(m_PoolHead);
            m_PoolHead = pPool;
        }
        else
        {
            // insert after nearest smaller pool
            pPool->NextSet(pPrevMatch->NextGet());
            pPrevMatch->NextSet(pPool);
        }
    }

    m_Mutex.Unlock();

    return rv;
}


// get singleton instance
MemManager *MemManager::InstanceGet()
{
    if (pInstance == NULL)
    {
        pInstance = new (CP_NEW) MemManager;

        if (pInstance == NULL)
        {
            LogErr << "MemManager::InstanceGet(): Failed to create MemManager instance."
                   << std::endl;
        }
    }

    return pInstance;
}


// return a pool of sufficient block size
MemPool *MemManager::PoolGet(size_t BlockSize)
{
    MemPool *pPool = m_PoolHead;
    MemPool *pBestMatch = NULL;

    while (pPool != NULL)
    {
        if (pPool->SizeGet() >= BlockSize)
        {
            pBestMatch = pPool;
            break;
        }

        // go to next pool, if any
        pPool = pPool->NextGet();
    }

    return pBestMatch;
}


// private constructor (for singleton pattern)
MemManager::MemManager() :
    m_DeployedCount(0),
    m_ReturnedCount(0),
    m_FailedGets(0),
    m_FailedPuts(0),
    m_DeployedSize(0),
    m_PoolHead(NULL)
{
    CreatePool(16,   256,  256);        //   4KB
    CreatePool(64,   128,  128);        //   8KB
    CreatePool(256,   64,   64);        //  16KB
    CreatePool(1024,  32,   32);        //  32KB
    CreatePool(4096,  16,   16);        //  64KB
    CreatePool(16384,  8,    8);        // 128KB
}

}   // namespace cp
