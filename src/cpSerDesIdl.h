// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSerDesIdl.h
//
//  Description:    Serializer/Deserializer IDL Format.
//
//  Platform:       common
//
//  History:
//  2013-11-15  asc Creation.
// ----------------------------------------------------------------------------

#ifndef CP_SERDESIDL_H
#define CP_SERDESIDL_H

#include "cpSerDes.h"

namespace cp
{

class Buffer;

// ----------------------------------------------------------------------------

class SerDesIdl : public SerDes
{
public:
    // custom data types
    typedef char const * DataTag_t;

    // custom data types
    typedef std::map<Datum::Attrib_t, DataTag_t, std::less<Datum::Attrib_t const>,
                     Alloc<std::pair<Datum::Attrib_t const, DataTag_t > > > AttribMap_t;

    typedef std::map<DataTag_t, Datum::Attrib_t, std::less<DataTag_t const>,
                     Alloc<std::pair<DataTag_t const, Datum::Attrib_t > > > TagMap_t;

    // constructor
    SerDesIdl();

    // destructor
    virtual ~SerDesIdl();

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

    bool NewLineInsert();                                   // encode a line ending into the output buffer
    bool UnsignedLongInsert(uint32_t Val);                  // encode an unsigned long into the output buffer
    bool SignedLongInsert(int32_t Val);                     // encode a signed long into the output buffer
    bool UnsignedLongLongInsert(uint64_t Val);              // encode an unsigned long long into the output buffer
    bool SignedLongLongInsert(int64_t Val);                 // encode a signed long long into the output buffer
    bool Float32Insert(float Val);                          // encode a single float into the output buffer
    bool Float64Insert(double Val);                         // encode a double float into the output buffer
    bool StringInsert(String const &Str);                   // encode a string into the output buffer
    bool BlobInsert(Buffer const &Buf);                     // encode a BLOB into the output buffer

    bool OpenTagInsert(String const &TagName, bool Attribute = false);
    bool CloseTagInsert(String const &TagName);
    bool AttribInsert(String const &AttrName, String const &Value, bool Final = true);

    bool VariantExtract(Variant &Var,
                        Variant::DataType_t Type,
                        String const &Value);               // decode a data value into a variant

    bool TagParse(String const &Line,
                  String &TagName,
                  String &Value);                           // detects a tag then retuns it and its value

    bool TagTrim(String &Line, char const *pTagName);       // detects and trims a tag if found

    bool TagValGet(String &Line, char const *pTagName,
                   String &Val);                            // returns the value of a tag

    Variant::DataType_t DataTypeGet(String const &Line);    // convert string data type to enum type

    bool                m_NewLine;                          // indicates the start of a new line
    size_t              m_IndentSize;                       // indentation size per level
    size_t              m_IndentLevel;                      // indentation level of nested datum
    AttribMap_t         m_AttribMap;                        // attribute to serializer tag map
    TagMap_t            m_TagMap;                           // serializer tag to attribute map
};

}   // namespace cp

#endif  // CP_SERDESIDL_H
