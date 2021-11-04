// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSerDesXml.cpp
//
//  Description:    Serializer/Deserializer XML Format.
//
//  Platform:       common
//
//  History:
//  2011-05-16  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-11-30  asc Added additional native data types for function call support.
//  2013-06-07  asc Corrected extraction handling of dt_none elements.
//  2013-11-15  asc Restructured to align with IDL ser/des.
//  2013-11-15  asc Fixed handling of checksum calculations.
// ----------------------------------------------------------------------------

#include <sstream>

#include "cpStreamBase.h"
#include "cpUtil.h"

#include "cpSerDesXml.h"

namespace cp
{

static char const *tag_OpenTagStart  = "<";
static char const *tag_CloseTagStart = "</";
static char const *tag_TagEnd        = ">";

static char const *tag_Hdr           = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

static char const *tag_Pkg           = "pkg";
static char const *tag_Ver           = "ver";
static char const *tag_Dat           = "dat";
static char const *tag_Name          = "name";
static char const *tag_Type          = "type";
static char const *tag_Val           = "val";
static char const *tag_Min           = "min";
static char const *tag_Max           = "max";
static char const *tag_Def           = "def";
static char const *tag_Units         = "units";
static char const *tag_Info          = "info";
static char const *tag_Choices       = "choices";

static char const *tag_Comment       = "<!--";
static char const *tag_CommentEnd    = "-->";
static char const *tag_LineEnd       = "\n";

static char const *tag_Crc32         = "CRC32:";
static char const *tag_Md5Sum        = "MD5SUM:";
static char const *tag_Sha1Sum       = "SHA1SUM:";

// ----------------------------------------------------------------------------

// constructor
SerDesXml::SerDesXml() :
    m_NewLine(false),
    m_IndentSize(4),
    m_IndentLevel(0)
{
    m_Name = k_SerDesXml;

    // initialize symbol maps
    m_AttribMap[Datum::attr_Name]    = tag_Name;
    m_AttribMap[Datum::attr_Val]     = tag_Val;
    m_AttribMap[Datum::attr_Min]     = tag_Min;
    m_AttribMap[Datum::attr_Max]     = tag_Max;
    m_AttribMap[Datum::attr_Def]     = tag_Def;
    m_AttribMap[Datum::attr_Units]   = tag_Units;
    m_AttribMap[Datum::attr_Info]    = tag_Info;
    m_AttribMap[Datum::attr_Choices] = tag_Choices;

    m_TagMap[tag_Name]    = Datum::attr_Name;
    m_TagMap[tag_Val]     = Datum::attr_Val;
    m_TagMap[tag_Min]     = Datum::attr_Min;
    m_TagMap[tag_Max]     = Datum::attr_Max;
    m_TagMap[tag_Def]     = Datum::attr_Def;
    m_TagMap[tag_Units]   = Datum::attr_Units;
    m_TagMap[tag_Info]    = Datum::attr_Info;
    m_TagMap[tag_Choices] = Datum::attr_Choices;
}


// destructor
SerDesXml::~SerDesXml()
{
}


// check if encoding is in this serializer's format
bool SerDesXml::CheckEncoding(StreamBase &Stream)
{
    bool rv = true;

    // assign the stream pointer
    m_PtrStream = &Stream;

    // rewind the stream and check identifier
    rv = rv && m_PtrStream->Seek(0);
    rv = rv && DecodeIdent();

    // clear the stream pointer
    m_PtrStream = NULL;

    return rv;
}


// generates encoding header
bool SerDesXml::Open(StreamBase &Stream, uint8_t Ver)
{
    bool rv = true;
    std::stringstream version;

    // reset the encoder state
    Clear();

    // assign stream pointer and clear the stream
    m_PtrStream = &Stream;
    Stream.Clear();

    // insert the header
    version << static_cast<uint32_t>(Ver);

    rv = rv && StringInsert(tag_Hdr);
    rv = rv && NewLineInsert();
    rv = rv && OpenTagInsert(tag_Pkg, true);
    rv = rv && AttribInsert(tag_Ver, version.str().c_str());

    return rv;
}


// generates encoding footer
bool SerDesXml::Close(Datum::CheckSum_t ChkMode)
{
    bool rv = (m_PtrStream != NULL);
    size_t pos = 0;

    // insert the terminator
    rv = rv && CloseTagInsert(tag_Pkg);
    pos = m_PtrStream->Pos();

    // if a checksum was specified
    if (ChkMode != Datum::ck_None)
    {
        // add the opening marker and checksum type
        rv = rv && StringInsert(tag_Comment);

        // generate and insert optional checksum
        switch (ChkMode)
        {
        case Datum::ck_Crc32:
            {
                std::stringstream crc;
                rv = rv && StringInsert(tag_Crc32);
                crc << std::setw(8) << std::setfill('0') << std::hex << m_PtrStream->Crc32Get(pos);
                rv = rv && StringInsert(crc.str().c_str());
            }
            break;

        case Datum::ck_Md5Sum:
            rv = rv && StringInsert(tag_Md5Sum);
            break;

        case Datum::ck_Sha1Sum:
            rv = rv && StringInsert(tag_Sha1Sum);
            break;

        default:
            break;
        }

        // add the closing marker
        rv = rv && StringInsert(tag_CommentEnd);
        rv = rv && NewLineInsert();
    }

    // reset the encoder state
    Clear();

    return rv;
}


// starts encoding a datum
bool SerDesXml::Start(Datum &Dat)
{
    bool rv = true;

    // encode the datum start
    rv = rv && OpenTagInsert(tag_Dat, true);
    rv = rv && AttribInsert(tag_Type, g_DataTypeNames[Dat.Val().TypeGet()], true);

    // encode the value and any attributes
    Datum::Attribs_t::const_iterator i = Dat.AttribBegin();

    while (i != Dat.AttribEnd())
    {
        // insert the attribute type marker
        rv = rv && OpenTagInsert(m_AttribMap[i->first]);

        // insert value based on data type
        switch (i->second.TypeGet())
        {
        case Variant::dt_uint8:
            rv = rv && UnsignedLongInsert(i->second.Uint8Get());
            break;

        case Variant::dt_int8:
            rv = rv && SignedLongInsert(i->second.Int8Get());
            break;

        case Variant::dt_uint16:
            rv = rv && UnsignedLongInsert(i->second.Uint16Get());
            break;

        case Variant::dt_int16:
            rv = rv && SignedLongInsert(i->second.Int16Get());
            break;

        case Variant::dt_uint32:
            rv = rv && UnsignedLongInsert(i->second.Uint32Get());
            break;

        case Variant::dt_int32:
            rv = rv && SignedLongInsert(i->second.Int32Get());
            break;

        case Variant::dt_uint64:
            rv = rv && UnsignedLongLongInsert(i->second.Uint64Get());
            break;

        case Variant::dt_int64:
            rv = rv && SignedLongLongInsert(i->second.Int64Get());
            break;

        case Variant::dt_float32:
            rv = rv && Float32Insert(i->second.Float32Get());
            break;

        case Variant::dt_float64:
            rv = rv && Float64Insert(i->second.Float64Get());
            break;

        case Variant::dt_bool:
            rv = rv && StringInsert((i->second.BoolGet() ? "true" : "false"));
            break;

        case Variant::dt_string:
            rv = rv && StringInsert(i->second.StrGet());
            break;

        case Variant::dt_blob:
            rv = rv && BlobInsert(i->second.BufGet());
            break;

        case Variant::dt_none:
            break;

        default:
            rv = false;
            break;
        }

        // insert closing tag
        rv = rv && CloseTagInsert(m_AttribMap[i->first]);

        ++i;
    }

    return rv;
}


// ends encoding a datum
bool SerDesXml::End(Datum &Dat)
{
    bool rv = true;

    rv = rv && CloseTagInsert(tag_Dat);
    (void)Dat;

    return rv;
}


// encode a line ending into the output buffer
bool SerDesXml::NewLineInsert()
{
    m_NewLine = StringInsert(tag_LineEnd);
    return m_NewLine;
}


// encode an unsigned long into the output buffer
bool SerDesXml::UnsignedLongInsert(uint32_t Val)
{
    std::stringstream ss;
    ss << Val;
    return StringInsert(ss.str().c_str());
}


// encode a signed long into the output buffer
bool SerDesXml::SignedLongInsert(int32_t Val)
{
    std::stringstream ss;
    ss << Val;
    return StringInsert(ss.str().c_str());
}


// encode an unsigned long long into the output buffer
bool SerDesXml::UnsignedLongLongInsert(uint64_t Val)
{
    std::stringstream ss;
    ss << Val;
    return StringInsert(ss.str().c_str());
}


// encode a signed long long into the output buffer
bool SerDesXml::SignedLongLongInsert(int64_t Val)
{
    std::stringstream ss;
    ss << Val;
    return StringInsert(ss.str().c_str());
}


// encode a single float into the output buffer
bool SerDesXml::Float32Insert(float Val)
{
    std::stringstream ss;
    ss.precision(k_FloatDigits);
    ss << std::fixed << Val;
    return StringInsert(ss.str().c_str());
}


// encode a double float into the output buffer
bool SerDesXml::Float64Insert(double Val)
{
    std::stringstream ss;
    ss.precision(k_DoubleDigits);
    ss << std::fixed << Val;
    return StringInsert(ss.str().c_str());
}


// encode a string into the output buffer
bool SerDesXml::StringInsert(String const &Str)
{
    bool rv = (m_PtrStream != NULL);
    size_t size = Str.size();

    if (m_NewLine)
    {
        size_t indent = m_IndentSize * m_IndentLevel;
        rv = rv && (m_PtrStream->ArrayWr(GenStr(indent, ' ').c_str(), indent) == indent);
        m_NewLine = false;
    }

    rv = rv && (m_PtrStream->ArrayWr(Str.c_str(), size) == size);

    return rv;
}


// encode a BLOB into the output buffer
bool SerDesXml::BlobInsert(Buffer const &Buf)
{
    std::stringstream hexOctet;
    bool rv = true;

    for (size_t i = 0; i < Buf.LenGet(); ++i)
    {
        if ((i & 0x0f) == 0)
        {
            rv = rv && NewLineInsert();
        }

        hexOctet.seekp(0);
        hexOctet << std::hex << std::setw(2) << std::setfill('0') << (uint32_t)(*Buf.u_str(i)) << " ";
        rv = rv && StringInsert(hexOctet.str().c_str());
    }

    rv = rv && NewLineInsert();

    return rv;
}


// insert an xml open tag
bool SerDesXml::OpenTagInsert(String const &TagName, bool Attribute)
{
    bool rv = true;

    rv = rv && StringInsert(tag_OpenTagStart);
    rv = rv && StringInsert(TagName);

    if (Attribute == false)
    {
        rv = rv && StringInsert(tag_TagEnd);
    }

    ++m_IndentLevel;

    return rv;
}


// insert an xml close tag
bool SerDesXml::CloseTagInsert(String const &TagName)
{
    bool rv = true;

    if (m_IndentLevel > 0)
    {
        --m_IndentLevel;
    }

    rv = rv && StringInsert(tag_CloseTagStart);
    rv = rv && StringInsert(TagName);
    rv = rv && StringInsert(tag_TagEnd);
    rv = rv && NewLineInsert();

    return rv;
}


// insert an xml attribute
bool SerDesXml::AttribInsert(String const &AttrName, String const &Value, bool Final)
{
    bool rv = true;

    rv = rv && StringInsert(" ");
    rv = rv && StringInsert(AttrName);
    rv = rv && StringInsert("=\"");
    rv = rv && StringInsert(Value);
    rv = rv && StringInsert("\"");

    if (Final)
    {
        rv = rv && StringInsert(tag_TagEnd);
        rv = rv && NewLineInsert();
    }

    return rv;
}


// decode a data value into a variant
bool SerDesXml::VariantExtract(Variant &Var, Variant::DataType_t Type, String const &Value)
{
    bool rv = true;

    switch (Type)
    {
    case Variant::dt_int8:
        Var.Int8Set(StrToInt(Value));
        break;

    case Variant::dt_uint8:
        Var.Uint8Set(StrToUint(Value));
        break;

    case Variant::dt_int16:
        Var.Int16Set(StrToInt(Value));
        break;

    case Variant::dt_uint16:
        Var.Uint16Set(StrToUint(Value));
        break;

    case Variant::dt_int32:
        Var.Int32Set(StrToInt(Value));
        break;

    case Variant::dt_uint32:
        Var.Uint32Set(StrToUint(Value));
        break;

    case Variant::dt_int64:
        Var.Int64Set(StrToInt(Value));
        break;

    case Variant::dt_uint64:
        Var.Uint64Set(StrToUint(Value));
        break;

    case Variant::dt_float32:
        Var.Float32Set(StrToFloat(Value));
        break;

    case Variant::dt_float64:
        Var.Float64Set(StrToFloat(Value));
        break;

    case Variant::dt_bool:
        Var.BoolSet(StrToBool(Value));
        break;

    case Variant::dt_string:
        Var.StrSet(Value);
        break;

    case Variant::dt_blob:
        {
            Buffer buf;
            HexDecode(Value, buf);
            Var.BufSet(buf);
        }
        break;

    case Variant::dt_none:
        break;

    default:
        rv = false;
        break;
    }

    return rv;
}


bool SerDesXml::DecodeIdent()
{
    bool rv = false;
    String line;

    // find the XML header
    while (!rv && LineGet(line))
    {
        rv = TagTrim(line, tag_Hdr);
    }

    return rv;
}


bool SerDesXml::DecodeVersion()
{
    bool rv = false;
    return rv;
}


bool SerDesXml::DecodeElement()
{
    bool rv = false;
    return rv;
}


bool SerDesXml::DecodeChecksum()
{
    bool rv = true;
    bool exitFlag = false;
    String line;
    Buffer buf;
    uint32_t crcs;
    uint32_t crcr;
    size_t pos = m_PtrStream->Pos();

    // decode elements
    while (rv && !exitFlag)
    {
        // this should be on an element boundary
        rv = LineGet(line);

        if (rv && (line.size() > 0))
        {
            // crc is stored in a comment following a package end tag
            if (TagTrim(line, tag_Comment))
            {
                // decode the hex into a binary value
                if (HexDecode(line, buf) == sizeof(crcs))
                {
                    // assemble bytes into a host order unsigned longword
                    for (size_t i = 0; i < sizeof(crcs); ++i)
                    {
                        crcs = crcs << 8;
                        crcs |= *buf.u_str(i);
                    }

                    // calculate the crc-32 of the received data
                    if (m_PtrStream != NULL)
                    {
                        crcr = m_PtrStream->Crc32Get(pos);
                    }

                    //  valid result when sent and received crc's match
                    rv = (crcs == crcr);
                    exitFlag = rv;
                }
            }
        }
    }

    return rv;
}


// detects a tag then retuns it and its value
bool SerDesXml::TagParse(String const &Line, String &TagName, String &Value)
{
    bool rv = false;
    (void)Line;
    (void)TagName;
    (void)Value;
    return rv;
}


// detects and trims a tag if found
bool SerDesXml::TagTrim(String &Line, char const *pTagName)
{
    bool rv = false;
    (void)Line;
    (void)pTagName;
    return rv;
}


// returns the value of a tag
bool SerDesXml::TagValGet(String &Line, char const *pTagName, String &Val)
{
    bool rv = false;
    Val.clear();

    rv = TagTrim(Line, pTagName);

    if (rv)
    {
        Val = Line;
    }

    return rv;
}


// convert string data type to enum type
Variant::DataType_t SerDesXml::DataTypeGet(String const &Line)
{
    bool found = false;
    Variant::DataType_t type = Variant::dt_NumDataTypes;
    char const **pType = &g_DataTypeNames[0];
    int i = 0;

    while (!found && (pType != NULL))
    {
        if (Line == *pType)
        {
            type = static_cast<Variant::DataType_t>(i);
            found = true;
        }

        ++i;
        ++pType;
    }

    return type;
}


// create and return an instance
SerDes *SerDesXml::CreateInstance()
{
    return new (CP_NEW) SerDesXml;
}

}   // namespace cp
