// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpStreamBase.h
//
//  Description:    Stream I/O Base Class.
//
//  Platform:       common
//
//  History:
//  2012-05-01  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-06-03  asc Added primitive data serializers.
//  2013-08-07  asc Replaced byte order state with separate B/L insertion methods.
//  2013-08-29  asc Refactored Clear() operation to eliminate inheritance pitfalls.
//  2022-03-15  asc Added flag to return terminator, if present, with ReadLine().
// ----------------------------------------------------------------------------
#ifndef CP_STREAMBASE_H
#define CP_STREAMBASE_H

#include "cpString.h"

namespace cp
{

class Buffer;

// ----------------------------------------------------------------------------

class StreamBase
{
public:
    // constructor
    StreamBase();

    // destructor
    virtual ~StreamBase();

    // accessors
    virtual char Peek();                                    // observe next character available to read
    virtual bool Readable();                                // true when at least one character can be read
    virtual size_t Pos() const;                             // return the current stream position
    virtual size_t LenGet() const;                          // return the stream data size
    virtual size_t BufSize() const;                         // return the stream buffer size
    virtual size_t ArrayRd(char *pBuf, size_t Len);         // read up to Len octets from stream into array
    virtual size_t ArrayWr(char const *pBuf, size_t Len);   // write up to Len octets to stream from array

    size_t ArrayRd(uint8_t *pBuf, size_t Len);              // read up to Len octets from stream into array
    size_t ArrayWr(uint8_t const *pBuf, size_t Len);        // write up to Len octets to stream from array
    size_t Read(Buffer &Buf, size_t Len);                   // read up to Len octets from stream
    size_t Write(Buffer const &Buf, size_t Len);            // write up to Len octets to stream
    bool Read(uint8_t &Ch);                                 // read one octet from stream
    bool Write(uint8_t Ch);                                 // write one octet to stream
    bool Read(char &Ch);                                    // read one octet from stream
    bool Write(char Ch);                                    // write one octet to stream

    bool ReadLine(String &Line, char Term = '\n',
                  bool DiscardTerm = true);                 // read until terminator or buf size
    bool BinDump(std::ostream &Out);                        // binary dump contents to an ostream object
    bool HexDump(std::ostream &Out);                        // hex dump contents to an ostream object
    bool BinLoad(std::istream &In);                         // binary load contents from an ostream object
    bool HexLoad(std::istream &In);                         // hex load contents from an ostream object
    uint32_t Crc32Get(size_t Len = 0);                      // calculate the CRC-32 of the buffer contents

    void BlockList(std::ostream &Out);                      // list the currently allocated blocks
    void BlockDump(std::ostream &Out);                      // display contents of currently allocated blocks

    // manipulators
    void Clear();                                           // clears the stream and seeks to zero
    virtual bool Seek(size_t Pos = 0);                      // seeks to position specified
    virtual bool Skip(size_t Num = 1);                      // skips forward by specified count
    virtual bool Back(size_t Num = 1);                      // skips backward by specified count

    // octet insertion
    bool OctetInsert(uint8_t Val)    { return Write(Val);                }

    // little endian insertion
    bool Uint16InsertL(uint16_t Val) { return Uint16Insert(Val, false);  }
    bool Uint32InsertL(uint32_t Val) { return Uint32Insert(Val, false);  }
    bool Uint64InsertL(uint64_t Val) { return Uint64Insert(Val, false);  }
    bool Float32InsertL(float Val)   { return Float32Insert(Val, false); }
    bool Float64InsertL(double Val)  { return Float64Insert(Val, false); }

    // big endian insertion
    bool Uint16InsertB(uint16_t Val) { return Uint16Insert(Val, true);   }
    bool Uint32InsertB(uint32_t Val) { return Uint32Insert(Val, true);   }
    bool Uint64InsertB(uint64_t Val) { return Uint64Insert(Val, true);   }
    bool Float32InsertB(float Val)   { return Float32Insert(Val, true);  }
    bool Float64InsertB(double Val)  { return Float64Insert(Val, true);  }

    bool CStringInsert(cp::String const &Str);              // insert a C string into the stream
    bool StringInsert(cp::String const &Str);               // insert a string into the stream
    bool BlobInsert(cp::Buffer const &Buf);                 // insert a binary blob into the stream

    // operators

protected:
    bool Uint16Insert(uint16_t Val, bool NetworkOrder);     // insert a short into the stream
    bool Uint32Insert(uint32_t Val, bool NetworkOrder);     // insert a long into the stream
    bool Uint64Insert(uint64_t Val, bool NetworkOrder);     // insert a long long into the stream
    bool Float32Insert(float Val, bool NetworkOrder);       // insert a float into the stream
    bool Float64Insert(double Val, bool NetworkOrder);      // insert a double into the stream

    virtual void MemoryFree();                              // free any allocated storage
    virtual bool MemoryAdd(size_t Size);                    // add memory to the stream
    virtual bool MemoryChk() const;                         // returns true if stream has some memory
    virtual bool ValidBlock(size_t Block) const;            // returns true if block is valid
    virtual char *BlockMemPtr(size_t Block) const;          // returns block's memory pointer
    virtual size_t BlockSize(size_t Block) const;           // returns specified memory block size

    size_t              m_CurBlock;                         // index to current memory block pointer
    size_t              m_CurPos;                           // current stream position in current block
    size_t              m_LastBlock;                        // index to last memory block pointer
    size_t              m_LastPos;                          // last used position in last block
};

}   // namespace cp

#endif // CP_STREAMBASE_H
