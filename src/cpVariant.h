// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpVariant.h
//
//  Description:    Flexible data container class.
//
//  Platform:       common
//
//  History:
//  2012-05-30  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-11-16  asc Added copy constructor, assignment operator, and insertion operator.
//  2012-11-30  asc Added additional native data types for function call support.
//  2013-06-28  asc Added Clear() method.
//  2013-07-19  asc Added inert state.
// ----------------------------------------------------------------------------

#ifndef CP_VARIANT_H
#define CP_VARIANT_H

#include "cpBuffer.h"
#include "cpPooledBase.h"

namespace cp
{

// type names used during formatted display and type parsing (defined in Variant.cpp)
extern char const *g_DataTypeNames[];

// ----------------------------------------------------------------------------

class Variant : public PooledBase
{
public:
    // data type enumeration
    enum DataType_t { dt_inert, dt_none, dt_uint8, dt_int8, dt_uint16, dt_int16, dt_uint32, dt_int32,
                      dt_uint64, dt_int64, dt_float32, dt_float64, dt_bool, dt_string, dt_blob,
                      dt_DatumList, dt_NumDataTypes };

    // constructor
    Variant(bool Inert = false);

    // copy constructor
    Variant(Variant const &rhs);

    // destructor
    virtual ~Variant() {}

    // operators
    Variant &operator=(Variant const &rhs);

    // setters
    void Uint8Set(uint8_t   Val);
    void Int8Set(int8_t     Val);
    void Uint16Set(uint16_t Val);
    void Int16Set(int16_t   Val);
    void Uint32Set(uint32_t Val);
    void Int32Set(int32_t   Val);
    void Uint64Set(uint64_t Val);
    void Int64Set(int64_t   Val);
    void Float32Set(float   Val);
    void Float64Set(double  Val);

    void BoolSet(bool Val);
    void StrSet(String const &Str) { m_Buf = Str; TypeSet(dt_string); }
    void BufSet(Buffer const &Buf) { m_Buf = Buf; TypeSet(dt_blob); }
    void BufSet(uint8_t const *pBuf, size_t Len) { m_Buf.CopyIn(pBuf, Len); TypeSet(dt_blob); }

    // getters
    uint8_t  Uint8Get()   const;
    int8_t   Int8Get()    const;
    uint16_t Uint16Get()  const;
    int16_t  Int16Get()   const;
    uint32_t Uint32Get()  const;
    int32_t  Int32Get()   const;
    uint64_t Uint64Get()  const;
    int64_t  Int64Get()   const;
    float    Float32Get() const;
    double   Float64Get() const;

    bool BoolGet() const;
    String StrGet() const;
    Buffer &BufGet() { return m_Buf; }
    Buffer const &BufGet() const { return m_Buf; }

    // management methods
    DataType_t TypeGet() const { return m_Type; }
    void Clear();

    // operators
    friend std::ostream &operator<<(std::ostream &Out, Variant const &Obj);

private:
    bool ResizeBuffer(size_t Size);                         // resize buffer and set data length
    void TypeSet(DataType_t Type) { m_Type = Type; }

    // return pointer to data storage
    char *DataBuffer() { return m_Buf.c_str(); };
    char const *DataBuffer() const { return m_Buf.c_str(); }

    DataType_t          m_Type;                             // the currently stored data type
    Buffer              m_Buf;                              // the underlying storage buffer
};

}   // namespace cp

#endif  // CP_VARIANT_H
