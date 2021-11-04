// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012-2014 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpVariant.cpp
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
//  2013-06-08  asc Prevented String from being assigned a NULL value.
//  2013-06-28  asc Added Clear() method.
//  2013-07-19  asc Added inert state.
//  2014-03-30  asc Inserted newline before hex dump of blob data.
// ----------------------------------------------------------------------------

#include "cpVariant.h"
#include "cpUtil.h"

namespace cp
{

// type names used during formatted display
char const *g_DataTypeNames[] = { "Inert", "None", "Uint8", "Int8", "Uint16", "Int16", "Uint32", "Int32",
                                  "Uint64", "Int64", "Float32", "Float64", "Bool", "String", "BLOB",
                                  "DatumList", NULL };

// ----------------------------------------------------------------------------

// constructor
Variant::Variant(bool Inert) :
    m_Type(dt_none)
{
    if (Inert)
    {
        m_Type = dt_inert;
    }

    ResizeBuffer(sizeof(double));
}


// copy constructor
Variant::Variant(Variant const &rhs) :
    m_Type(dt_none)
{
    // invoke assignment operator
    *this = rhs;
}


// operators
Variant &Variant::operator=(Variant const &rhs)
{
    // check for self assignment or inert state
    if ((this != &rhs) && (m_Type != dt_inert))
    {
        m_Type = rhs.m_Type;
        m_Buf  = rhs.m_Buf;
    }

    return *this;
}


// setters
void Variant::Uint8Set(uint8_t Val)
{
    if (ResizeBuffer(sizeof(uint8_t)))
    {
        *reinterpret_cast<uint8_t *>(DataBuffer()) = Val;
        TypeSet(dt_uint8);
    }
}


void Variant::Int8Set(int8_t Val)
{
    if (ResizeBuffer(sizeof(int8_t)))
    {
        *reinterpret_cast<int8_t *>(DataBuffer()) = Val;
        TypeSet(dt_int8);
    }
}


void Variant::Uint16Set(uint16_t Val)
{
    if (ResizeBuffer(sizeof(uint16_t)))
    {
        *reinterpret_cast<uint16_t *>(DataBuffer()) = Val;
        TypeSet(dt_uint16);
    }
}


void Variant::Int16Set(int16_t Val)
{
    if (ResizeBuffer(sizeof(int16_t)))
    {
        *reinterpret_cast<int16_t *>(DataBuffer()) = Val;
        TypeSet(dt_int16);
    }
}


void Variant::Uint32Set(uint32_t Val)
{
    if (ResizeBuffer(sizeof(uint32_t)))
    {
        *reinterpret_cast<uint32_t *>(DataBuffer()) = Val;
        TypeSet(dt_uint32);
    }
}


void Variant::Int32Set(int32_t Val)
{
    if (ResizeBuffer(sizeof(int32_t)))
    {
        *reinterpret_cast<int32_t *>(DataBuffer()) = Val;
        TypeSet(dt_int32);
    }
}


void Variant::Uint64Set(uint64_t Val)
{
    if (ResizeBuffer(sizeof(uint64_t)))
    {
        *reinterpret_cast<uint64_t *>(DataBuffer()) = Val;
        TypeSet(dt_uint64);
    }
}


void Variant::Int64Set(int64_t Val)
{
    if (ResizeBuffer(sizeof(int64_t)))
    {
        *reinterpret_cast<int64_t *>(DataBuffer()) = Val;
        TypeSet(dt_int64);
    }
}


void Variant::Float32Set(float Val)
{
    if (ResizeBuffer(sizeof(float)))
    {
        *reinterpret_cast<float *>(DataBuffer()) = Val;
        TypeSet(dt_float32);
    }
}


void Variant::Float64Set(double Val)
{
    if (ResizeBuffer(sizeof(double)))
    {
        *reinterpret_cast<double *>(DataBuffer()) = Val;
        TypeSet(dt_float64);
    }
}


void Variant::BoolSet(bool Val)
{
    if (ResizeBuffer(sizeof(bool)))
    {
        *DataBuffer() = (Val ? '1' : '0');
        TypeSet(dt_bool);
    }
}


// getters
uint8_t Variant::Uint8Get() const
{
    return static_cast<uint8_t>(Uint64Get());
}


int8_t Variant::Int8Get() const
{
    return static_cast<int8_t>(Uint64Get());
}


uint16_t Variant::Uint16Get() const
{
    return static_cast<uint16_t>(Uint64Get());
}


int16_t Variant::Int16Get() const
{
    return static_cast<int16_t>(Uint64Get());
}


uint32_t Variant::Uint32Get() const
{
    return static_cast<uint32_t>(Uint64Get());
}


int32_t Variant::Int32Get() const
{
    return static_cast<int32_t>(Uint64Get());
}


uint64_t Variant::Uint64Get() const
{
    uint64_t rv;

    switch (TypeGet())
    {
    case dt_uint8:
    case dt_int8:
        rv = *reinterpret_cast<uint8_t const *>(DataBuffer());
        break;

    case dt_uint16:
    case dt_int16:
        rv = *reinterpret_cast<uint16_t const *>(DataBuffer());
        break;

    case dt_uint32:
    case dt_int32:
        rv = *reinterpret_cast<uint32_t const *>(DataBuffer());
        break;

    case dt_uint64:
    case dt_int64:
        rv = *reinterpret_cast<uint64_t const *>(DataBuffer());
        break;

    case dt_float32:
        rv = static_cast<uint64_t>(*reinterpret_cast<float const *>(DataBuffer()));
        break;

    case dt_float64:
        rv = static_cast<uint64_t>(*reinterpret_cast<double const *>(DataBuffer()));
        break;

    case dt_bool:
        rv = (*DataBuffer() == '1') ? 1 : 0;
        break;

    case dt_string:
        rv = cp::StrToUint(DataBuffer());
        break;

    case dt_blob:
        rv = m_Buf.LenGet();
        break;

    default:
        rv = 0;
        break;
    }

    return rv;
}


int64_t Variant::Int64Get() const
{
    return static_cast<int64_t>(Uint64Get());
}


float Variant::Float32Get() const
{
    float rv;

    switch (TypeGet())
    {
    case dt_uint8:
    case dt_int8:
        rv = *reinterpret_cast<int8_t const *>(DataBuffer());
        break;

    case dt_uint16:
    case dt_int16:
        rv = *reinterpret_cast<int16_t const *>(DataBuffer());
        break;

    case dt_uint32:
    case dt_int32:
        rv = *reinterpret_cast<int32_t const *>(DataBuffer());
        break;

    case dt_uint64:
    case dt_int64:
        rv = *reinterpret_cast<int64_t const *>(DataBuffer());
        break;

    case dt_float32:
        rv = *reinterpret_cast<float const *>(DataBuffer());
        break;

    case dt_float64:
        rv = *reinterpret_cast<double const *>(DataBuffer());
        break;

    case dt_bool:
        rv = (*DataBuffer() == '1') ? 1.0 : 0.0;
        break;

    case dt_string:
        rv = cp::StrToFloat(DataBuffer());
        break;

    case dt_blob:
        rv = m_Buf.LenGet();
        break;

    default:
        rv = 0.0F;
        break;
    }

    return rv;
}


double Variant::Float64Get() const
{
    double rv;

    switch (TypeGet())
    {
    case dt_uint8:
    case dt_int8:
        rv = *reinterpret_cast<int8_t const *>(DataBuffer());
        break;

    case dt_uint16:
    case dt_int16:
        rv = *reinterpret_cast<int16_t const *>(DataBuffer());
        break;

    case dt_uint32:
    case dt_int32:
        rv = *reinterpret_cast<int32_t const *>(DataBuffer());
        break;

    case dt_uint64:
    case dt_int64:
        rv = *reinterpret_cast<int64_t const *>(DataBuffer());
        break;

    case dt_float32:
        rv = *reinterpret_cast<float const *>(DataBuffer());
        break;

    case dt_float64:
        rv = *reinterpret_cast<double const *>(DataBuffer());
        break;

    case dt_bool:
        rv = (*DataBuffer() == '1') ? 1.0 : 0.0;
        break;

    case dt_string:
        rv = cp::StrToFloat(DataBuffer());
        break;

    case dt_blob:
        rv = m_Buf.LenGet();
        break;

    default:
        rv = 0.0;
        break;
    }

    return rv;
}


bool Variant::BoolGet() const
{
    bool rv;

    switch (TypeGet())
    {
    case dt_uint8:
    case dt_int8:
        rv = (*reinterpret_cast<uint8_t const *>(DataBuffer()) > 0);
        break;

    case dt_uint16:
    case dt_int16:
        rv = (*reinterpret_cast<uint16_t const *>(DataBuffer()) > 0);
        break;

    case dt_uint32:
    case dt_int32:
        rv = (*reinterpret_cast<uint32_t const *>(DataBuffer()) > 0);
        break;

    case dt_uint64:
    case dt_int64:
        rv = (*reinterpret_cast<uint64_t const *>(DataBuffer()) > 0);
        break;

    case dt_float32:
        rv = (*reinterpret_cast<float const *>(DataBuffer()) > 0.5);
        break;

    case dt_float64:
        rv = (*reinterpret_cast<double const *>(DataBuffer()) > 0.5);
        break;

    case dt_bool:
        rv = (*DataBuffer() == '1');
        break;

    case dt_string:
        rv = (*DataBuffer() != '\0');
        break;

    case dt_blob:
        rv = (m_Buf.LenGet() > 0);
        break;

    default:
        rv = false;
        break;
    }

    return rv;
}


String Variant::StrGet() const
{
    String rv;

    switch (TypeGet())
    {
    case dt_uint8:
        rv = cp::UintToStr(*reinterpret_cast<uint8_t const *>(DataBuffer()));
        break;

    case dt_int8:
        rv = cp::IntToStr(*reinterpret_cast<int8_t const *>(DataBuffer()));
        break;

    case dt_uint16:
        rv = cp::UintToStr(*reinterpret_cast<uint16_t const *>(DataBuffer()));
        break;

    case dt_int16:
        rv = cp::IntToStr(*reinterpret_cast<int16_t const *>(DataBuffer()));
        break;

    case dt_uint32:
        rv = cp::UintToStr(*reinterpret_cast<uint32_t const *>(DataBuffer()));
        break;

    case dt_int32:
        rv = cp::IntToStr(*reinterpret_cast<int32_t const *>(DataBuffer()));
        break;

    case dt_uint64:
        rv = cp::UintToStr(*reinterpret_cast<uint64_t const *>(DataBuffer()));
        break;

    case dt_int64:
        rv = cp::IntToStr(*reinterpret_cast<int64_t const *>(DataBuffer()));
        break;

    case dt_float32:
        rv = cp::FloatToStr(*reinterpret_cast<float const *>(DataBuffer()));
        break;

    case dt_float64:
        rv = cp::FloatToStr(*reinterpret_cast<double const *>(DataBuffer()));
        break;

    case dt_bool:
        rv = (*DataBuffer() == '1') ? "1" : "0";
        break;

    case dt_string:
    case dt_blob:
        if (DataBuffer())
        {
            rv = DataBuffer();
        }
        break;

    default:
        break;
    }

    return rv;
}


// resize buffer and set data length
bool Variant::ResizeBuffer(size_t Size)
{
    bool rv = (m_Type != dt_inert);

    rv = rv && m_Buf.Resize(Size);

    if (rv)
    {
        m_Buf.LenSet(Size);
    }

    return rv;
}


// operators
std::ostream &operator<<(std::ostream &Out, Variant const &Obj)
{
    switch (Obj.TypeGet())
    {
    case Variant::dt_int8:
        Out << Obj.Int8Get();
        break;

    case Variant::dt_uint8:
        Out << Obj.Uint8Get();
        break;

    case Variant::dt_int16:
        Out << Obj.Int16Get();
        break;

    case Variant::dt_uint16:
        Out << Obj.Uint16Get();
        break;

    case Variant::dt_int32:
        Out << Obj.Int32Get();
        break;

    case Variant::dt_uint32:
        Out << Obj.Uint32Get();
        break;

    case Variant::dt_int64:
        Out << Obj.Int64Get();
        break;

    case Variant::dt_uint64:
        Out << Obj.Uint64Get();
        break;

    case Variant::dt_float32:
        Out << Obj.Float32Get();
        break;

    case Variant::dt_float64:
        Out << Obj.Float64Get();
        break;

    case Variant::dt_bool:
        Out << (Obj.BoolGet() ? "T" : "F");
        break;

    case Variant::dt_string:
        Out << Obj.StrGet();
        break;

    case Variant::dt_blob:
        Out << "\n";
        HexDump(Out, Obj.m_Buf);
        break;

    case Variant::dt_inert:
        Out << "*Inert*";
        break;

    case Variant::dt_none:
    default:
        break;
    }

    return Out;
}


void Variant::Clear()
{
    if (m_Type != dt_inert)
    {
        m_Type = dt_none;
        m_Buf.Resize(0);
    }
}

}   // namespace cp
