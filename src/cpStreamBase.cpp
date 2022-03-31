// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpStreamBase.cpp
//
//  Description:    Stream I/O Base Class.
//
//  Platform:       common
//
//  History:
//  2012-05-01  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-02-08  asc Modified Read() to only increase buffer size, not reduce it.
//  2013-03-17  asc Tweaked resize algorithm for Read() method.
//  2013-06-03  asc Added primitive data serializers.
//  2013-08-07  asc Replaced byte order state with separate B/L insertion methods.
//  2013-08-29  asc Refactored Clear() operation to eliminate pitfalls.
//  2013-11-15  asc Implemented CRC calculation.
//  2022-03-15  asc Added flag to return terminator, if present, with ReadLine().
// ----------------------------------------------------------------------------

#include "cpStreamBase.h"
#include "cpBuffer.h"
#include "cpUtil.h"

namespace cp
{

// constructor
StreamBase::StreamBase() :
    m_CurBlock(0),
    m_CurPos(0),
    m_LastBlock(0),
    m_LastPos(0)
{
}


// destructor
StreamBase::~StreamBase()
{
}


// observe next character available to read
char StreamBase::Peek()
{
    char *pch = NULL;
    char ch = '\0';

    if (Readable())
    {
        pch = BlockMemPtr(m_CurBlock);

        if (pch)
        {
            ch = *(pch + m_CurPos);
        }
    }

    return ch;
}


// true when at least one character can be read
bool StreamBase::Readable()
{
    bool rv = false;

    // determine if there is some data that can be read
    if (MemoryChk())
    {
        // check if current position is located somewhere before end of stream
        rv = ((m_CurBlock < m_LastBlock) || (m_CurPos < m_LastPos));
    }

    return rv;
}


// return the current stream position
size_t StreamBase::Pos() const
{
    uint32_t i = 0;
    size_t pos = 0;

    while (i < m_CurBlock)
    {
        pos += BlockSize(i);
        ++i;
    }

    pos += m_CurPos;

    return pos;
}


// return the stream data size
size_t StreamBase::LenGet() const
{
    uint32_t i = 0;
    size_t len = 0;

    while (i < m_LastBlock)
    {
        len += BlockSize(i);
        ++i;
    }

    len += m_LastPos;

    return len;
}


// return the stream buffer size
size_t StreamBase::BufSize() const
{
    uint32_t i = 0;
    size_t size = 0;

    while (ValidBlock(i))
    {
        size += BlockSize(i);
        ++i;
    }

    return size;
}


// read up to Len octets from stream into array
size_t StreamBase::ArrayRd(char *pBuf, size_t Len)
{
    size_t numRead = 0;
    size_t leftToRead = Len;

    if ((pBuf == NULL) || (Len == 0))
    {
        return 0;
    }

    // if data is available, read it
    if (Readable())
    {
        // loop until read request fulfilled or data runs out
        while (leftToRead > 0)
        {
            size_t readSize = 0;
            size_t blockSize = BlockSize(m_CurBlock);

            // determine how much to read in this pass
            if (m_CurBlock < m_LastBlock)
            {
                if (m_CurPos < blockSize)
                {
                    readSize = blockSize - m_CurPos;
                }
                else
                {
                    // index to next block
                    ++m_CurBlock;
                    m_CurPos = 0;
                }
            }
            else
            {
                if (m_CurPos < m_LastPos)
                {
                    readSize = m_LastPos - m_CurPos;
                }
                else
                {
                    // no more data available to read so exit loop
                    leftToRead = 0;
                }
            }

            // adjust read size if greater than requested
            if (readSize > leftToRead)
            {
                readSize = leftToRead;
            }

            if (readSize > 0)
            {
                char *addr = BlockMemPtr(m_CurBlock);

                // read data
                if (addr)
                {
                    memcpy(pBuf + numRead, addr + m_CurPos, readSize);

                    // adjust pointers
                    leftToRead -= readSize;
                    m_CurPos += readSize;
                    numRead += readSize;
                }
                else
                {
                    LogErr << "StreamBase()::ArrayRd(): Invalid Address, instance: "
                           << this << std::endl;
                    leftToRead = 0;
                }
            }
        }
    }

    return numRead;
}


// write up to Len octets to stream from array
size_t StreamBase::ArrayWr(char const *pBuf, size_t Len)
{
    size_t numWritten = 0;
    size_t leftToWrite = Len;

    if ((pBuf == NULL) || (Len == 0))
    {
        return 0;
    }

    // make sure at least one block is allocated
    if (MemoryChk() == false)
    {
        if (MemoryAdd(Len) == false)
        {
            return 0;
        }
    }

    // loop until write request fulfilled
    while (leftToWrite > 0)
    {
        size_t writeSize = 0;
        size_t blockSize = BlockSize(m_CurBlock);

        // determine how much to write in this pass
        if (m_CurPos < blockSize)
        {
            writeSize = blockSize - m_CurPos;
        }
        else
        {
            // index to next block position
            ++m_CurBlock;
            m_CurPos = 0;

            // check if a block needs to be allocated
            if (ValidBlock(m_CurBlock) == false)
            {
                // allocate a block
                if (MemoryAdd(leftToWrite) == false)
                {
                    // abort if failed to acquire a block
                    leftToWrite = 0;
                }
            }
        }

        // adjust write size if greater than requested
        if (writeSize > leftToWrite)
        {
            writeSize = leftToWrite;
        }

        if (writeSize > 0)
        {
            char *addr = BlockMemPtr(m_CurBlock);

            // write data
            if (addr)
            {
                memcpy(addr + m_CurPos, pBuf + numWritten, writeSize);

                // adjust pointers
                leftToWrite -= writeSize;
                m_CurPos += writeSize;
                numWritten += writeSize;
            }
            else
            {
                LogErr << "StreamBase()::ArrayWr(): Invalid Address, instance: "
                       << this << std::endl;
                leftToWrite = 0;
            }
        }
    }

    // adjust pointers
    if (m_LastBlock < m_CurBlock)
    {
        m_LastBlock = m_CurBlock;
        m_LastPos = m_CurPos;
    }
    else if (m_LastBlock == m_CurBlock)
    {
        if (m_LastPos < m_CurPos)
        {
            m_LastPos = m_CurPos;
        }
    }

    return numWritten;
}


// read up to Len octets from stream into array
size_t StreamBase::ArrayRd(uint8_t *pBuf, size_t Len)
{
    return ArrayRd((char *)pBuf, Len);
}


// write up to Len octets to stream from array
size_t StreamBase::ArrayWr(uint8_t const *pBuf, size_t Len)
{
    return ArrayWr((char const *)pBuf, Len);
}


// read up to Len octets from stream
size_t StreamBase::Read(Buffer &Buf, size_t Len)
{
    bool status = true;
    size_t length = (Len < LenGet()) ? Len : LenGet();

    // make sure there is enough target storage, but don't
    // resize unless more storage is required for this transfer
    // since user may have purposely allocated a larger buffer
    if (Buf.Size() < length)
    {
        status = Buf.Resize(length);
    }

    if (status)
    {
        length = ArrayRd(Buf.c_str(), length);
        Buf.LenSet(length);
    }
    else
    {
        LogErr << "StreamBase::Read(): Failed to resize buffer to size required for copy, instance: "
               << this << std::endl;
    }

    return length;
}


// write up to Len octets to stream
size_t StreamBase::Write(Buffer const &Buf, size_t Len)
{
    size_t length = Buf.LenGet();

    if (length > Len)
    {
        length = Len;
    }

    return ArrayWr(Buf.c_str(), length);
}


// read one octet from stream
bool StreamBase::Read(uint8_t &Ch)
{
    return (ArrayRd((char *)&Ch, sizeof(uint8_t)) == sizeof(uint8_t));
}


// write one octet to stream
bool StreamBase::Write(uint8_t Ch)
{
    return (ArrayWr((char *)&Ch, sizeof(uint8_t)) == sizeof(uint8_t));
}


// read one octet from stream
bool StreamBase::Read(char &Ch)
{
    return (ArrayRd(&Ch, sizeof(char)) == sizeof(char));
}


// write one octet to stream
bool StreamBase::Write(char Ch)
{
    return (ArrayWr(&Ch, sizeof(char)) == sizeof(char));
}


// read until terminator or buf size
bool StreamBase::ReadLine(String &Line, char Term, bool DiscardTerm)
{
    char ch;
    bool rv = true;
    bool exitFlag = false;

    // clear supplied target string
    Line.clear();

    // search for terminator
    while (rv && !exitFlag)
    {
        rv = Read(ch);

        if (rv)
        {
            if (ch == Term)
            {
                if (!DiscardTerm)
                {
                    Line += ch;
                }

                exitFlag = true;
            }
            else
            {
                if (ch != '\0')
                {
                    Line += ch;
                }
            }
        }
    }

    return (rv || (Line.size() > 0));
}


// binary dump contents to an ostream object
bool StreamBase::BinDump(std::ostream &Out)
{
    bool rv = true;
    bool exitFlag = false;
    size_t len = 0;
    size_t size = 1024;
    Buffer buf(size);

    // rewind the stream
    rv = rv && Seek(0);

    // write the data out
    while (rv && !exitFlag)
    {
        len = Read(buf, size);
        exitFlag = (len == 0);

        if (!exitFlag)
        {
            Out.write(buf.c_str(), len);

            if (!Out)
            {
                rv = false;
            }
        }
    }

    return rv;
}


// hex dump contents to an ostream object
bool StreamBase::HexDump(std::ostream &Out)
{
    bool rv = true;
    bool exitFlag = false;
    size_t len = 0;
    size_t size = 1024;
    Buffer buf(size);

    // rewind the stream
    rv = rv && Seek(0);

    // write the data out
    while (rv && !exitFlag)
    {
        len = Read(buf, size);
        exitFlag = (len == 0);

        if (!exitFlag)
        {
            rv = cp::HexDump(Out, buf);
        }
    }

    return rv;
}


// binary load contents from an ostream object
bool StreamBase::BinLoad(std::istream &In)
{
    bool rv = true;
    bool exitFlag = false;
    size_t len = 0;
    size_t size = 1024;
    Buffer buf(size);

    // clear the stream
    Clear();

    // read the data in
    while (rv && !exitFlag)
    {
        In.read(buf.c_str(), size);
        len = In.gcount();
        buf.LenSet(len);
        exitFlag = (len == 0);

        if (!exitFlag)
        {
            rv = (Write(buf, len) == len);
        }
    }

    return rv;
}


// hex load contents from an ostream object
bool StreamBase::HexLoad(std::istream &In)
{
    // (.)(.) need to implement HexLoad()
    (void)In;
    return false;
}


// calculate the CRC-32 of the buffer contents
uint32_t StreamBase::Crc32Get(size_t Len)
{
    uint32_t i = 0;
    size_t size = 0;
    uint32_t cascade = 0xffffffff;

    // length of 0 means use the entire stream
    if (Len == 0)
    {
        Len = LenGet();
    }

    while (ValidBlock(i) && (Len > 0))
    {
        size = BlockSize(i);

        // trim if less than full stream was requested
        if (size > Len)
        {
            size = Len;
        }

        cascade = cp::CalcCrc32(BlockMemPtr(i), size, cascade);
        ++i;
        Len -= size;
    }

    return cascade;
}


// list the currently allocated blocks
void StreamBase::BlockList(std::ostream &Out)
{
    uint32_t i = 0;

    Out << "\nCurrent Buffer Inventory" << std::endl;
    Out << "------------------------" << std::endl;

    while (ValidBlock(i))
    {
        Out << "- Block of size: " << BlockSize(i) << std::endl;
        ++i;
    }

    Out << "------------------------\n" << std::endl;
}


// display contents of currently allocated blocks
void StreamBase::BlockDump(std::ostream &Out)
{
    uint32_t i = 0;
    size_t size = 0;

    Out << "\nCurrent Buffer Inventory" << std::endl;
    Out << "------------------------" << std::endl;

    while (ValidBlock(i))
    {
        size = BlockSize(i);
        Out << "- Block of size: " << size << std::endl;
        cp::HexDump(Out, BlockMemPtr(i), size);
        ++i;
    }

    Out << "------------------------\n" << std::endl;
}


// clears the stream and seeks to zero
void StreamBase::Clear()
{
    // free any allocated storage
    MemoryFree();

    // reset state data
    m_LastBlock = 0;
    m_LastPos = 0;
    m_CurBlock = 0;
    m_CurPos = 0;
}


// seeks to position specified
bool StreamBase::Seek(size_t Pos)
{
    size_t loc = 0;
    size_t size = 0;
    uint32_t i = 0;
    bool rv = false;

    while (ValidBlock(i) && !rv)
    {
        // if this is not the last written block
        if (i < m_LastBlock)
        {
            size = BlockSize(i);
        }
        else if (i == m_LastBlock)
        {
            size = m_LastPos;
        }
        else
        {
            size = 0;
            break;
        }

        // check if seek position has been found or overshot
        if ((loc + size) >= Pos)
        {
            m_CurPos = Pos - loc;
            rv = true;
        }
        else
        {
            loc += size;
            m_CurPos = size;
        }

        // update the current position pointers
        m_CurBlock = i;

        // iterate
        ++i;
    }

    return rv;
}


// skips forward by specified count
bool StreamBase::Skip(size_t Num)
{
    bool rv = false;
    size_t pos = Pos();

    // make sure size_t doesn't overflow
    if ((pos + Num) > pos)
    {
        rv = Seek(pos + Num);
    }
    else
    {
        rv = Seek(-1);
    }

    return rv;
}


// skips backward by specified count
bool StreamBase::Back(size_t Num)
{
    bool rv = false;
    size_t pos = Pos();

    // make sure size_t doesn't underflow
    if (pos >= Num)
    {
        rv = Seek(pos - Num);
    }
    else
    {
        Seek(0);
    }

    return rv;
}


// encode a C string into the output stream
bool StreamBase::CStringInsert(cp::String const &Str)
{
    bool rv = true;
    size_t size = Str.size();

    rv = rv && (ArrayWr(Str.c_str(), size) == size);
    rv = rv && OctetInsert(0x00);

    return rv;
}


// encode a string into the output stream
bool StreamBase::StringInsert(cp::String const &Str)
{
    bool rv = true;
    size_t size = Str.size();

    rv = rv && (ArrayWr(Str.c_str(), size) == size);

    return rv;
}


// encode a BLOB into the output stream
bool StreamBase::BlobInsert(cp::Buffer const &Buf)
{
    bool rv = true;
    size_t size = Buf.LenGet();

    rv = rv && (Write(Buf, size) == size);

    return rv;
}


// encode a short into the output stream
bool StreamBase::Uint16Insert(uint16_t Val, bool NetworkOrder)
{
    bool rv = true;
    size_t i = 0;
    uint8_t *pSrc = reinterpret_cast<uint8_t *>(&Val);
    bool bswap = (cp::HostBigEndian() != NetworkOrder);

    if (bswap)
    {
        for (i = sizeof(Val); i > 0; --i)
        {
            rv = rv && OctetInsert(pSrc[i-1]);
        }
    }
    else
    {
        for (i = 0; i < sizeof(Val); ++i)
        {
            rv = rv && OctetInsert(pSrc[i]);
        }
    }

    return rv;
}


// encode a long into the output stream
bool StreamBase::Uint32Insert(uint32_t Val, bool NetworkOrder)
{
    bool rv = true;
    size_t i = 0;
    uint8_t *pSrc = reinterpret_cast<uint8_t *>(&Val);
    bool bswap = (cp::HostBigEndian() != NetworkOrder);

    if (bswap)
    {
        for (i = sizeof(Val); i > 0; --i)
        {
            rv = rv && OctetInsert(pSrc[i-1]);
        }
    }
    else
    {
        for (i = 0; i < sizeof(Val); ++i)
        {
            rv = rv && OctetInsert(pSrc[i]);
        }
    }

    return rv;
}


// encode a long long into the output stream
bool StreamBase::Uint64Insert(uint64_t Val, bool NetworkOrder)
{
    bool rv = true;
    size_t i = 0;
    uint8_t *pSrc = reinterpret_cast<uint8_t *>(&Val);
    bool bswap = (cp::HostBigEndian() != NetworkOrder);

    if (bswap)
    {
        for (i = sizeof(Val); i > 0; --i)
        {
            rv = rv && OctetInsert(pSrc[i-1]);
        }
    }
    else
    {
        for (i = 0; i < sizeof(Val); ++i)
        {
            rv = rv && OctetInsert(pSrc[i]);
        }
    }

    return rv;
}


// encode a single float into the output stream
bool StreamBase::Float32Insert(float Val, bool NetworkOrder)
{
    bool rv = true;
    size_t i = 0;
    uint8_t *pSrc = reinterpret_cast<uint8_t *>(&Val);
    bool bswap = (cp::HostBigEndian() != NetworkOrder);

    if (bswap)
    {
        for (i = sizeof(Val); i > 0; --i)
        {
            rv = rv && OctetInsert(pSrc[i-1]);
        }
    }
    else
    {
        for (i = 0; i < sizeof(Val); ++i)
        {
            rv = rv && OctetInsert(pSrc[i]);
        }
    }

    return rv;
}


// encode a double float into the output stream
bool StreamBase::Float64Insert(double Val, bool NetworkOrder)
{
    bool rv = true;
    size_t i = 0;
    uint8_t *pSrc = reinterpret_cast<uint8_t *>(&Val);
    bool bswap = (cp::HostBigEndian() != NetworkOrder);

    if (bswap)
    {
        for (i = sizeof(Val); i > 0; --i)
        {
            rv = rv && OctetInsert(pSrc[i-1]);
        }
    }
    else
    {
        for (i = 0; i < sizeof(Val); ++i)
        {
            rv = rv && OctetInsert(pSrc[i]);
        }
    }

    return rv;
}


// free any allocated storage
void StreamBase::MemoryFree()
{
}


// add memory to the stream
bool StreamBase::MemoryAdd(size_t Size)
{
    (void)Size;
    return false;
}


// returns true if stream has some memory
bool StreamBase::MemoryChk() const
{
    return false;
}


// returns true if block is valid
bool StreamBase::ValidBlock(size_t Block) const
{
    (void)Block;
    return false;
}


// returns block's memory pointer
char *StreamBase::BlockMemPtr(size_t Block) const
{
    (void)Block;
    return NULL;
}


// returns specified memory block size
size_t StreamBase::BlockSize(size_t Block) const
{
    (void)Block;
    return 0;
}

}   // namespace cp
