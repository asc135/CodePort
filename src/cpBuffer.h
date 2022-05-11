// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpBuffer.h
//
//  Description:    Managed Buffer Class.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-08-31  asc Added copy constructors to match conversion assignment operators.
//  2022-05-10  asc Added list and vector containers for Buffer.
// ----------------------------------------------------------------------------

#ifndef CP_BUFFER_H
#define CP_BUFFER_H

#include "cpString.h"

namespace cp
{

class MemBlock;
class StreamBase;

// ----------------------------------------------------------------------------

class Buffer
{
public:
    // constructor
    Buffer(size_t Size = 0);

    // copy constructors
    Buffer(Buffer const &rhs);
    Buffer(StreamBase &rhs);
    Buffer(String const &rhs);
    Buffer(char const *rhs);
    Buffer(uint8_t const *rhs);

    // destructor
    ~Buffer();

    // accessors
    bool IsValid()                                          // determine if a memory block is present
    {
        return (m_PtrBlock != NULL);
    }

    size_t LenGet() const;                                  // return the data length
    size_t Size() const;                                    // return the memory buffer size
    uint32_t Crc32Get();                                    // calculate the CRC-32 of the buffer contents
    char *c_str(size_t Offset = 0);                         // return a pointer to the memory buffer
    char const *c_str(size_t Offset = 0) const;             // return a pointer to the memory buffer
    uint8_t *u_str(size_t Offset = 0);                      // return a pointer to the memory buffer
    uint8_t const *u_str(size_t Offset = 0) const;          // return a pointer to the memory buffer
    bool CopyOut(uint8_t *pBuf, size_t Size) const;         // copy data out of the buffer

    // manipulators
    void Clear(int Val = 0);                                // zero or fill the memory buffer
    void LenSet(size_t Len);                                // set the data length
    bool Resize(size_t NewSize);                            // acquire a new memory block
    bool CopyIn(uint8_t const *pBuf, size_t Len);           // copy data into the buffer
    void XferMemBlk(Buffer &Dest);                          // move memory block to destination Buffer
    MemBlock *GetMemBlk();                                  // extract memory block from Buffer
    void SetMemBlk(MemBlock *pBlock);                       // insert memory block into Buffer

    // operators
    Buffer &operator=(Buffer const &rhs);                   // assignment operator -- any binary content
    Buffer &operator=(StreamBase &rhs);                     // assignment conversion operator -- any binary content
    Buffer &operator=(String const &rhs);                   // assignment conversion operator -- terminated strings only
    Buffer &operator=(char const *rhs);                     // assignment conversion operator -- terminated strings only
    Buffer &operator=(uint8_t const *rhs);                  // assignment conversion operator -- terminated strings only

    // conversion operators
    operator uint32_t *();                                  // convert instance to uint32_t pointer
    operator uint16_t *();                                  // convert instance to uint16_t pointer
    operator uint8_t *();                                   // convert instance to uint8_t pointer
    operator char *();                                      // convert instance to char pointer
    operator size_t();                                      // convert instance to buffer size

private:
    MemBlock           *m_PtrBlock;                         // pointer to managed memory block
    size_t              m_DataLen;                          // length of buffer used
};


// define some useful buffer container types
typedef std::list<Buffer, Alloc<Buffer> > BufferList_t;
typedef std::vector<Buffer, Alloc<Buffer> > BufferVec_t;

}   // namespace cp

#endif // CP_BUFFER_H
