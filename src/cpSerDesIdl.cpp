// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSerDesIdl.cpp
//
//  Description:    Serializer/Deserializer IDL Format.
//
//  Platform:       common
//
//  History:
//  2013-11-15  asc Creation.
//  2013-11-15  asc Fixed handling of checksum calculations.
// ----------------------------------------------------------------------------

#include <sstream>

#include "cpStreamBase.h"
#include "cpUtil.h"

#include "cpSerDesIdl.h"

namespace cp
{

static char const *tag_Begin         = "Begin: ";
static char const *tag_End           = "End";


static char const *tag_Hdr           = "// DATUM IDL";

static char const *tag_Pkg           = "Package ";
static char const *tag_Ver           = "Version: ";
static char const *tag_Dat           = "Datum ";
static char const *tag_Name          = "Name: ";

static char const *tag_Val           = "Value: ";
static char const *tag_Min           = "MinVal: ";
static char const *tag_Max           = "MaxVal: ";
static char const *tag_Def           = "Default: ";
static char const *tag_Units         = "Units: ";
static char const *tag_Info          = "Info: ";
static char const *tag_Choices       = "Choices: ";

static char const *tag_Comment       = "//";
static char const *tag_CommentEnd    = "";
static char const *tag_LineEnd       = "\n";

static char const *tag_Crc32         = " CRC32:";
static char const *tag_Md5Sum        = " MD5SUM:";
static char const *tag_Sha1Sum       = " SHA1SUM:";

// ----------------------------------------------------------------------------

// constructor
SerDesIdl::SerDesIdl() :
    m_NewLine(false),
    m_IndentSize(4),
    m_IndentLevel(0)
{
    m_Name = k_SerDesIdl;

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
SerDesIdl::~SerDesIdl()
{
}


// check if encoding is in this serializer's format
bool SerDesIdl::CheckEncoding(StreamBase &Stream)
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
bool SerDesIdl::Open(StreamBase &Stream, uint8_t Ver)
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
bool SerDesIdl::Close(Datum::CheckSum_t ChkMode)
{
    bool rv = (m_PtrStream != NULL);
    size_t pos = 0;

    // insert the terminator
    rv = rv && CloseTagInsert(tag_Pkg);
    rv = rv && NewLineInsert();
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
bool SerDesIdl::Start(Datum &Dat)
{
    bool rv = true;

    // encode the datum start
    rv = rv && OpenTagInsert(tag_Dat, true);
    rv = rv && StringInsert(g_DataTypeNames[Dat.Val().TypeGet()]);
    rv = rv && NewLineInsert();

    // encode the value and any attributes
    Datum::Attribs_t::const_iterator i = Dat.AttribBegin();

    while (i != Dat.AttribEnd())
    {
        // insert the attribute type marker
        rv = rv && StringInsert(m_AttribMap[i->first]);

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

        // insert a newline
        rv = rv && NewLineInsert();

        ++i;
    }

    return rv;
}


// ends encoding a datum
bool SerDesIdl::End(Datum &Dat)
{
    bool rv = true;

    rv = rv && CloseTagInsert(tag_Dat);
    rv = rv && NewLineInsert();
    (void)Dat;

    return rv;
}


// encode a line ending into the output buffer
bool SerDesIdl::NewLineInsert()
{
    m_NewLine = StringInsert(tag_LineEnd);
    return m_NewLine;
}


// encode an unsigned long into the output buffer
bool SerDesIdl::UnsignedLongInsert(uint32_t Val)
{
    std::stringstream ss;
    ss << Val;
    return StringInsert(ss.str().c_str());
}


// encode a signed long into the output buffer
bool SerDesIdl::SignedLongInsert(int32_t Val)
{
    std::stringstream ss;
    ss << Val;
    return StringInsert(ss.str().c_str());
}


// encode an unsigned long long into the output buffer
bool SerDesIdl::UnsignedLongLongInsert(uint64_t Val)
{
    std::stringstream ss;
    ss << Val;
    return StringInsert(ss.str().c_str());
}


// encode a signed long long into the output buffer
bool SerDesIdl::SignedLongLongInsert(int64_t Val)
{
    std::stringstream ss;
    ss << Val;
    return StringInsert(ss.str().c_str());
}


// encode a single float into the output buffer
bool SerDesIdl::Float32Insert(float Val)
{
    std::stringstream ss;
    ss.precision(k_FloatDigits);
    ss << std::fixed << Val;
    return StringInsert(ss.str().c_str());
}


// encode a double float into the output buffer
bool SerDesIdl::Float64Insert(double Val)
{
    std::stringstream ss;
    ss.precision(k_DoubleDigits);
    ss << std::fixed << Val;
    return StringInsert(ss.str().c_str());
}


// encode a string into the output buffer
bool SerDesIdl::StringInsert(String const &Str)
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
bool SerDesIdl::BlobInsert(Buffer const &Buf)
{
    std::stringstream hexOctet;
    bool rv = true;

    for (size_t i = 0; i < Buf.LenGet(); ++i)
    {
        hexOctet.seekp(0);
        hexOctet << std::hex << std::setw(2) << std::setfill('0') << (uint32_t)(*Buf.u_str(i)) << " ";
        rv = rv && StringInsert(hexOctet.str().c_str());
    }

    return rv;
}


// insert an xml open tag
bool SerDesIdl::OpenTagInsert(String const &TagName, bool Attribute)
{
    bool rv = true;

    rv = rv && StringInsert(TagName);
    rv = rv && StringInsert(tag_Begin);

    if (Attribute == false)
    {
        rv = rv && NewLineInsert();
    }

    ++m_IndentLevel;

    return rv;
}


// insert an xml close tag
bool SerDesIdl::CloseTagInsert(String const &TagName)
{
    bool rv = true;

    if (m_IndentLevel > 0)
    {
        --m_IndentLevel;
    }

    rv = rv && StringInsert(TagName);
    rv = rv && StringInsert(tag_End);

    return rv;
}


// insert an xml attribute
bool SerDesIdl::AttribInsert(String const &AttrName, String const &Value, bool Final)
{
    bool rv = true;

    rv = rv && StringInsert(AttrName);
    rv = rv && StringInsert(Value);

    if (Final)
    {
        rv = rv && NewLineInsert();
    }

    return rv;
}


// decode a data value into a variant
bool SerDesIdl::VariantExtract(Variant &Var, Variant::DataType_t Type, String const &Value)
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


bool SerDesIdl::DecodeIdent()
{
    bool rv = false;
    String line;

    // find the IDL header
    while (!rv && LineGet(line))
    {
        rv = TagTrim(line, tag_Hdr);
    }

    return rv;
}


bool SerDesIdl::DecodeVersion()
{
    bool rv = false;
    String line;

    // find the package header
    while (!rv && LineGet(line))
    {
        // find the package keyword
        if (TagTrim(line, tag_Pkg))
        {
            // find the begin keyword
            if (TagTrim(line, tag_Begin))
            {
                String ver;

                // find the ver keyword and get its value
                rv = TagValGet(line, tag_Ver, ver);
            }
        }
    }

    return rv;
}


bool SerDesIdl::DecodeElement()
{
    bool rv = true;
    bool exitFlag = false;
    bool rootDatum = true;
    String line;
    String value;
    Datum *pDat = NULL;
    Variant::DataType_t type = Variant::dt_none;

    // retrieve most current datum from stack
    if (m_DatStack.size() > 0)
    {
        pDat = m_DatStack.front();
    }

    // abort if we don't have an active datum
    if (pDat == NULL)
    {
        return false;
    }

    // decode elements
    while (rv && !exitFlag)
    {
        // this should be on an element boundary
        rv = LineGet(line);
        Ltrim(line);

        if (rv)
        {
            rv = false;

            // ignore blank lines
            if (line.size() == 0)
            {
                rv = true;
            }

            // check if line contains 'Datum Begin/End'
            if (!rv)
            {
                // check if line contains 'Datum'
                if (TagTrim(line, tag_Dat))
                {
                    // check if line contains 'Begin'
                    if (TagTrim(line, tag_Begin))
                    {
                        // get the datum type
                        type = DataTypeGet(line);

                        if (rootDatum)
                        {
                            rootDatum = false;
                        }
                        else
                        {
                            pDat->Add();
                            pDat = &(pDat->Get());
                            m_DatStack.push_front(pDat);
                        }

                        rv = true;
                    }
                    else
                    // check if line contains 'End'
                    if (TagTrim(line, tag_End))
                    {
                        if (m_DatStack.size() > 1)
                        {
                            m_DatStack.pop_front();
                            pDat = m_DatStack.front();
                        }

                        rv = true;
                    }
                }
            }

            // check if line contains an attribute
            if (!rv)
            {
                TagMap_t::iterator i = m_TagMap.begin();

                while ((i != m_TagMap.end()) && !rv)
                {
                    if (TagValGet(line, i->first, value))
                    {
                        Variant var;
                        Variant::DataType_t attributeType;

                        switch (i->second)
                        {
                        case Datum::attr_Name:
                        case Datum::attr_Units:
                        case Datum::attr_Info:
                        case Datum::attr_Choices:
                            attributeType = Variant::dt_string;
                            break;

                        default:
                            attributeType = type;
                            break;
                        }

                        rv = VariantExtract(var, attributeType, value);
                        pDat->AttrSet(i->second, var);
                    }

                    ++i;
                }
            }

            // check if line contains 'Package End'
            if (!rv)
            {
                // check if line contains 'Package'
                if (TagTrim(line, tag_Pkg))
                {
                    // check if line contains 'End'
                    if (TagTrim(line, tag_End))
                    {
                        exitFlag = true;
                        rv = true;
                    }
                }
            }

            // check if line contains a comment
            if (!rv)
            {
                // check if line contains '//'
                if (TagTrim(line, tag_Comment))
                {
                    rv = true;
                }
            }
        }
    }

    if (!rv)
    {
        LogErr << "Bad decoding of line: \"" << line << "\"" << std::endl;
    }

    return rv;
}


bool SerDesIdl::DecodeChecksum()
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
                // trim off the checksum type
                if (TagTrim(line, tag_Crc32))
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
    }

    return rv;
}


// detects a tag then retuns it and its value
bool SerDesIdl::TagParse(String const &Line, String &TagName, String &Value)
{
    bool rv = false;
    String local = Line;
    size_t colpos = 0;

    // remove any leading spaces
    Ltrim(local);

    // locate colon followed by space
    colpos = local.find(": ");

    // if found, assign the tag and value
    if (colpos != String::npos)
    {
        TagName = local.substr(0, colpos + 2);
        Value = local.substr(colpos + 2);
        rv = true;
    }

    return rv;
}


// detects and trims a tag if found
bool SerDesIdl::TagTrim(String &Line, char const *pTagName)
{
    bool rv = false;
    size_t len = 0;
    String str;
    String tag;

    if (pTagName == NULL)
    {
        return false;
    }
    else
    {
        tag = pTagName;
    }

    // remove any leading spaces
    Ltrim(Line);
    Ltrim(tag);

    // get a substring the size of the token
    len = tag.length();

    if (len <= Line.size())
    {
        str = Line.substr(0, len);
    }

    // search for the token in the substring
    if (str == tag)
    {
        // trim off the found tag
        Line = Line.substr(len);
        rv = true;
    }

    return rv;
}


// returns the value of a tag
bool SerDesIdl::TagValGet(String &Line, char const *pTagName, String &Val)
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
Variant::DataType_t SerDesIdl::DataTypeGet(String const &Line)
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
SerDes *SerDesIdl::CreateInstance()
{
    return new (CP_NEW) SerDesIdl;
}

}   // namespace cp
