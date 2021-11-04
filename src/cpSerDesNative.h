// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSerDesNative.h
//
//  Description:    Serializer/Deserializer Native Format.
//
//  Platform:       common
//
//  History:
//  2011-05-16  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-11-30  asc Added additional native data types for function call support.
// ----------------------------------------------------------------------------

#ifndef CP_SERDESNATIVE_H
#define CP_SERDESNATIVE_H

#include "cpSerDes.h"

namespace cp
{

class Buffer;

// ----------------------------------------------------------------------------

class SerDesNative : public SerDes
{
public:
    // enumerations
    enum DataTag_t
    {
        mk_None         = 0x00,

        mk_Magic        = 0x88,

        mk_Pkg          = 0xa0,
        mk_PkgEnd       = 0xa1,
        mk_Dat          = 0xa2,
        mk_DatEnd       = 0xa3,
        mk_Chk          = 0xa4,
        mk_ChkEnd       = 0xa5,

        mk_Name         = 0xb0,

        mk_Val          = 0xc0,
        mk_Min          = 0xc1,
        mk_Max          = 0xc2,
        mk_Def          = 0xc3,
        mk_Units        = 0xc4,
        mk_Info         = 0xc5,
        mk_Choices      = 0xc6
    };

    // custom data types
    typedef std::map<Datum::Attrib_t, DataTag_t, std::less<Datum::Attrib_t const>,
                     Alloc<std::pair<Datum::Attrib_t const, DataTag_t > > > AttribMap_t;

    typedef std::map<DataTag_t, Datum::Attrib_t, std::less<DataTag_t const>,
                     Alloc<std::pair<DataTag_t const, Datum::Attrib_t > > > TagMap_t;

    // constructor
    SerDesNative();

    // destructor
    virtual ~SerDesNative();

    // accessors
    virtual bool CheckEncoding(StreamBase &Stream);         // check if encoding is in this serializer's format

    // manipulators
    virtual bool Open(StreamBase &Stream, uint8_t Ver);     // generates encoding header
    virtual bool Close(Datum::CheckSum_t ChkMode);          // generates encoding footer
    virtual bool Start(Datum &Dat);                         // starts encoding a datum
    virtual bool End(Datum &Dat);                           // ends encoding a datum

private:
    virtual bool DecodeIdent();
    virtual bool DecodeVersion();
    virtual bool DecodeElement();
    virtual bool DecodeChecksum();
    virtual SerDes *CreateInstance();                       // create and return an instance

    bool LenInsert(size_t Len);                             // encode a block length into the output stream
    bool OctetInsert(uint8_t Val);                          // encode an octet into the output stream
    bool ShortInsert(uint16_t Val);                         // encode a short into the output stream
    bool LongInsert(uint32_t Val);                          // encode a long into the output stream
    bool LongLongInsert(uint64_t Val);                      // encode a long long into the output stream
    bool Float32Insert(float Val);                          // encode a single float into the output stream
    bool Float64Insert(double Val);                         // encode a double float into the output stream
    bool StringInsert(String const &Str);                   // encode a string into the output stream
    bool BlobInsert(Buffer const &Buf);                     // encode a BLOB into the output stream

    bool LenExtract(size_t &Len);                           // decode a block length from the input stream
    bool OctetExtract(uint8_t &Val);                        // decode an octet from the input stream
    bool ShortExtract(uint16_t &Val);                       // decode a short from the input stream
    bool LongExtract(uint32_t &Val);                        // decode a long from the input stream
    bool LongLongExtract(uint64_t &Val);                    // decode a long long from the input stream
    bool Float32Extract(float &Val);                        // decode a single float from the input stream
    bool Float64Extract(double &Val);                       // decode a double float from the input stream
    bool StringExtract(String &Str);                        // decode a string from the input stream
    bool BlobExtract(Buffer &Buf);                          // decode a BLOB from the input stream
    bool VariantExtract(Variant &Var,
                        Variant::DataType_t Type);          // decode a data value into a variant

    AttribMap_t         m_AttribMap;                        // attribute to serializer tag map
    TagMap_t            m_TagMap;                           // serializer tag to attribute map
};

}   // namespace cp

#endif  // CP_SERDESNATIVE_H
