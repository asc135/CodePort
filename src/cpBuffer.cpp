// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpBuffer.cpp
//
//  Description:    Managed Buffer Class.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-08-31  asc Added copy constructors to match conversion assignment operators.
//  2013-02-08  asc Changed block resize ratio from 3.0 to 2.0.
//  2013-03-08  asc Fixed a length criterion in the CopyOut method.
//  2013-04-22  asc Removed duplicated code from StreamBase conversion assignment.
//  2013-06-08  asc Added m_PtrBlock validity check before CopyOut() references it.
// ----------------------------------------------------------------------------

#include "cpStreamBase.h"
#include "cpBuffer.h"
#include "cpMemMgr.h"
#include "cpUtil.h"

namespace cp
{

// constructor
Buffer::Buffer(size_t Size) :
    m_PtrBlock(NULL),
    m_DataLen(0)
{
    Resize(Size);
}


// copy constructors
Buffer::Buffer(Buffer const &rhs) :
    m_PtrBlock(NULL),
    m_DataLen(0)
{
    // invoke assignment operator
    *this = rhs;
}


Buffer::Buffer(StreamBase &rhs) :
    m_PtrBlock(NULL),
    m_DataLen(0)
{
    // invoke assignment operator
    *this = rhs;
}


Buffer::Buffer(String const &rhs) :
    m_PtrBlock(NULL),
    m_DataLen(0)
{
    // invoke assignment operator
    *this = rhs;
}


Buffer::Buffer(char const *rhs) :
    m_PtrBlock(NULL),
    m_DataLen(0)
{
    // invoke assignment operator
    *this = rhs;
}


Buffer::Buffer(uint8_t const *rhs) :
    m_PtrBlock(NULL),
    m_DataLen(0)
{
    // invoke assignment operator
    *this = rhs;
}


// destructor
Buffer::~Buffer()
{
    Resize(0);
}


// return the data length
size_t Buffer::LenGet() const
{
    return m_DataLen;
}


// return memory buffer size
size_t Buffer::Size() const
{
    size_t rv = 0;

    if (m_PtrBlock != NULL)
    {
        rv = m_PtrBlock->SizeGet();
    }

    return rv;
}


// calculate the CRC-32 of the buffer contents
uint32_t Buffer::Crc32Get()
{
    uint32_t crc = 0;

    if (m_PtrBlock != NULL)
    {
        crc = cp::CalcCrc32(u_str(), m_DataLen);
    }

    return crc;
}


// return a pointer to the memory buffer
char *Buffer::c_str(size_t Offset)
{
    char *rv = NULL;

    if (m_PtrBlock != NULL)
    {
        rv = (m_PtrBlock->BuffGet() + Offset);
    }

    return rv;
}


// return a pointer to the memory buffer
char const *Buffer::c_str(size_t Offset) const
{
    char *rv = NULL;

    if (m_PtrBlock != NULL)
    {
        rv = (m_PtrBlock->BuffGet() + Offset);
    }

    return rv;
}


// return a pointer to the memory buffer
uint8_t *Buffer::u_str(size_t Offset)
{
    uint8_t *rv = NULL;

    if (m_PtrBlock != NULL)
    {
        rv = reinterpret_cast<uint8_t *>(m_PtrBlock->BuffGet() + Offset);
    }

    return rv;
}


// return a pointer to the memory buffer
uint8_t const *Buffer::u_str(size_t Offset) const
{
    uint8_t *rv = NULL;

    if (m_PtrBlock != NULL)
    {
        rv = reinterpret_cast<uint8_t *>(m_PtrBlock->BuffGet() + Offset);
    }

    return rv;
}


// copy data out of the buffer
bool Buffer::CopyOut(uint8_t *pBuf, size_t Size) const
{
    bool rv = false;
    size_t length = 0;

    if ((pBuf != NULL) && (m_PtrBlock != NULL))
    {
        length = m_PtrBlock->SizeGet();

        // copy buffer size or amount requested, whichever is less
        if (length > Size)
        {
            length = Size;
        }

        memcpy(pBuf, m_PtrBlock->BuffGet(), length);
        rv = true;
    }

    return rv;
}


// zero or fill the memory buffer
void Buffer::Clear(int Val)
{
    if (m_PtrBlock != NULL)
    {
        m_PtrBlock->Clear(Val);
    }

    m_DataLen = 0;
}


// set the data length
void Buffer::LenSet(size_t Len)
{
    if (m_PtrBlock != NULL)
    {
        if (Len < m_PtrBlock->SizeGet())
        {
            m_DataLen = Len;
        }
        else
        {
            m_DataLen = m_PtrBlock->SizeGet();
        }
    }
    else
    {
        m_DataLen = 0;
    }
}


// acquire a new memory block
bool Buffer::Resize(size_t NewSize)
{
    // don't take any action if sizes are the same or only slightly larger
    if ((m_PtrBlock != NULL) && (m_PtrBlock->SizeGet() >= NewSize)
        && (NewSize > 0) && ((m_PtrBlock->SizeGet() / NewSize) < 2.0))
    {
        return true;
    }

    // return any existing memory buffer
    if (m_PtrBlock != NULL)
    {
        if (MemManager::InstanceGet()->MemBlockPut(m_PtrBlock) == false)
        {
            LogErr << "Buffer::Resize(): Failed to return a MemBlock of size: "
                   << m_PtrBlock->SizeGet() << ", block address: " << m_PtrBlock
                   << ", cp::Buffer instance: " << this << std::endl;
        }

        m_PtrBlock = NULL;
    }

    // acquire another memory buffer
    if (NewSize > 0)
    {
        if (MemManager::InstanceGet()->MemBlockGet(m_PtrBlock, NewSize) == false)
        {
            LogErr << "Buffer::Resize(): Failed to acquire a MemBlock of size: "
                   << NewSize << ", cp::Buffer instance: " << this << std::endl;
        }
    }

    // clear the memory buffer
    Clear();

    return ((m_PtrBlock != NULL) || (NewSize == 0));
}


// copy data into the buffer
bool Buffer::CopyIn(uint8_t const *pBuf, size_t Len)
{
    if ((pBuf != NULL) && Resize(Len))
    {
        memcpy(m_PtrBlock->BuffGet(), pBuf, Len);
        m_DataLen = Len;
        return true;
    }
    else
    {
        return false;
    }
}


// move memory block to destination Buffer
void Buffer::XferMemBlk(Buffer &Dest)
{
    Dest.Resize(0);
    Dest.m_PtrBlock = m_PtrBlock;
    Dest.m_DataLen = m_DataLen;
    m_PtrBlock = NULL;
    m_DataLen = 0;
}


// extract memory block from Buffer
MemBlock *Buffer::GetMemBlk()
{
    MemBlock *p = m_PtrBlock;

    m_PtrBlock = NULL;
    m_DataLen = 0;

    return p;
}


// insert memory block into Buffer
void Buffer::SetMemBlk(MemBlock *pBlock)
{
    if (pBlock != NULL)
    {
        Resize(0);
        m_PtrBlock = pBlock;
    }
}


// assignment operator -- any binary content
Buffer &Buffer::operator=(Buffer const &rhs)
{
    // avoid self-assignment
    if (this == &rhs)
    {
        return *this;
    }

    size_t size = rhs.m_DataLen;

    // acquire a buffer of sufficient size
    if (Resize(size) == false)
    {
        LogErr << "Buffer::operator=(Buffer): Failed to resize Buffer to size: "
               << size << ", cp::Buffer instance: " << this << std::endl;
    }
    else
    {
        if ((size > 0) && rhs.m_PtrBlock && (rhs.m_PtrBlock->SizeGet() >= size))
        {
            // copy the buffer data
            memcpy(m_PtrBlock->BuffGet(), rhs.m_PtrBlock->BuffGet(), size);
        }

        // copy the data length
        m_DataLen = size;
    }

    return *this;
}


// assignment conversion operator -- any binary content
Buffer &Buffer::operator=(StreamBase &rhs)
{
    size_t size = rhs.LenGet();

    if (size > 0)
    {
        // copy the buffer data
        rhs.Seek(0);
        size = rhs.Read(*this, size);
    }

    return *this;
}


// assignment conversion operator -- terminated strings only
Buffer &Buffer::operator=(String const &rhs)
{
    return operator=(rhs.c_str());
}


// assignment conversion operator -- terminated strings only
Buffer &Buffer::operator=(char const *rhs)
{
    // check for invalid pointer
    if (rhs == NULL)
    {
        return *this;
    }

    // string length plus terminating zero
    size_t size = strlen(rhs) + sizeof(char);

    // acquire a buffer of sufficient size
    if (Resize(size) == false)
    {
        LogErr << "Buffer::operator=(char *): Failed to resize Buffer to size: "
               << size << ", cp::Buffer instance: " << this << std::endl;
    }
    else
    {
        if (size > 0)
        {
            // copy the buffer data (strcpy() copies the terminating zero)
            strcpy(m_PtrBlock->BuffGet(), rhs);
        }

        // copy the data length
        m_DataLen = size;
    }

    return *this;
}


// assignment conversion operator -- terminated strings only
Buffer &Buffer::operator=(uint8_t const *rhs)
{
    return operator=(reinterpret_cast<char const *>(rhs));
}


// convert instance to uint32_t pointer
Buffer::operator uint32_t *()
{
    uint32_t *rv = NULL;

    if (m_PtrBlock != NULL)
    {
        rv = reinterpret_cast<uint32_t *>(m_PtrBlock->BuffGet());
    }

    return rv;
}


// convert instance to uint16_t pointer
Buffer::operator uint16_t *()
{
    uint16_t *rv = NULL;

    if (m_PtrBlock != NULL)
    {
        rv = reinterpret_cast<uint16_t *>(m_PtrBlock->BuffGet());
    }

    return rv;
}


// convert instance to uint8_t pointer
Buffer::operator uint8_t *()
{
    uint8_t *rv = NULL;

    if (m_PtrBlock != NULL)
    {
        rv = reinterpret_cast<uint8_t *>(m_PtrBlock->BuffGet());
    }

    return rv;
}


// convert instance to char pointer
Buffer::operator char *()
{
    char *rv = NULL;

    if (m_PtrBlock != NULL)
    {
        rv = reinterpret_cast<char *>(m_PtrBlock->BuffGet());
    }

    return rv;
}


// convert instance to buffer size
Buffer::operator size_t()
{
    size_t rv = 0;

    if (m_PtrBlock != NULL)
    {
        rv = m_PtrBlock->SizeGet();
    }

    return rv;
}

}   // namespace cp
