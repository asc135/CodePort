// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSerDesNative.cpp
//
//  Description:    Serializer/Deserializer Native Format.
//
//  Platform:       common
//
//  History:
//  2011-05-16  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-11-30  asc Added additional native data types for function call support.
//  2013-02-08  asc Improved extraction handling of zero length strings and blobs.
//  2013-06-07  asc Corrected extraction handling of dt_none elements.
//  2013-11-15  asc Fixed handling of checksum calculations.
// ----------------------------------------------------------------------------

#include "cpStreamBase.h"
#include "cpUtil.h"

#include "cpSerDesNative.h"

namespace cp
{

// constructor
SerDesNative::SerDesNative()
{
    m_Name = k_SerDesNative;

    // initialize symbol maps
    m_AttribMap[Datum::attr_Name]    = mk_Name;
    m_AttribMap[Datum::attr_Val]     = mk_Val;
    m_AttribMap[Datum::attr_Min]     = mk_Min;
    m_AttribMap[Datum::attr_Max]     = mk_Max;
    m_AttribMap[Datum::attr_Def]     = mk_Def;
    m_AttribMap[Datum::attr_Units]   = mk_Units;
    m_AttribMap[Datum::attr_Info]    = mk_Info;
    m_AttribMap[Datum::attr_Choices] = mk_Choices;

    m_TagMap[mk_Name]    = Datum::attr_Name;
    m_TagMap[mk_Val]     = Datum::attr_Val;
    m_TagMap[mk_Min]     = Datum::attr_Min;
    m_TagMap[mk_Max]     = Datum::attr_Max;
    m_TagMap[mk_Def]     = Datum::attr_Def;
    m_TagMap[mk_Units]   = Datum::attr_Units;
    m_TagMap[mk_Info]    = Datum::attr_Info;
    m_TagMap[mk_Choices] = Datum::attr_Choices;
}


// destructor
SerDesNative::~SerDesNative()
{
}


// check if encoding is in this serializer's format
bool SerDesNative::CheckEncoding(StreamBase &Stream)
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
bool SerDesNative::Open(StreamBase &Stream, uint8_t Ver)
{
    bool rv = true;

    // reset the encoder state
    Clear();

    // assign stream pointer and clear the stream
    m_PtrStream = &Stream;
    Stream.Clear();

    // insert the header
    rv = rv && OctetInsert(mk_Magic);
    rv = rv && OctetInsert(mk_Pkg);
    rv = rv && OctetInsert(Ver);

    return rv;
}


// generates encoding footer
bool SerDesNative::Close(Datum::CheckSum_t ChkMode)
{
    bool rv = (m_PtrStream != NULL);
    size_t pos = 0;

    // insert the terminator
    rv = rv && OctetInsert(mk_PkgEnd);
    pos = m_PtrStream->Pos();

    // if a checksum was specified
    if (ChkMode != Datum::ck_None)
    {
        // add the opening marker and checksum type
        rv = rv && OctetInsert(mk_Chk);
        rv = rv && OctetInsert(ChkMode);

        // generate and insert optional checksum
        switch (ChkMode)
        {
        case Datum::ck_Crc32:
            rv = rv && LongInsert(m_PtrStream->Crc32Get(pos));
            break;

        case Datum::ck_Md5Sum:
            break;

        case Datum::ck_Sha1Sum:
            break;

        default:
            break;
        }

        // add the closing marker
        rv = rv && OctetInsert(mk_ChkEnd);
    }

    // reset the encoder state
    Clear();

    return rv;
}


// starts encoding a datum
bool SerDesNative::Start(Datum &Dat)
{
    bool rv = true;

    // encode the datum start
    rv = rv && OctetInsert(mk_Dat);

    // encode the value and any attributes
    Datum::Attribs_t::const_iterator i = Dat.AttribBegin();

    while (i != Dat.AttribEnd())
    {
        // insert the attribute type symbol
        rv = rv && OctetInsert(m_AttribMap[i->first]);

        // insert the data type marker
        rv = rv && OctetInsert(i->second.TypeGet());

        // insert value based on data type
        switch (i->second.TypeGet())
        {
        case Variant::dt_uint8:
            rv = rv && OctetInsert(i->second.Uint8Get());
            break;

        case Variant::dt_int8:
            rv = rv && OctetInsert(i->second.Int8Get());
            break;

        case Variant::dt_uint16:
            rv = rv && ShortInsert(i->second.Uint16Get());
            break;

        case Variant::dt_int16:
            rv = rv && ShortInsert(i->second.Int16Get());
            break;

        case Variant::dt_uint32:
            rv = rv && LongInsert(i->second.Uint32Get());
            break;

        case Variant::dt_int32:
            rv = rv && LongInsert(i->second.Int32Get());
            break;

        case Variant::dt_uint64:
            rv = rv && LongLongInsert(i->second.Uint64Get());
            break;

        case Variant::dt_int64:
            rv = rv && LongLongInsert(i->second.Int64Get());
            break;

        case Variant::dt_float32:
            rv = rv && Float32Insert(i->second.Float32Get());
            break;

        case Variant::dt_float64:
            rv = rv && Float64Insert(i->second.Float64Get());
            break;

        case Variant::dt_bool:
            rv = rv && OctetInsert(i->second.BoolGet() ? k_OctetTrue : k_OctetFalse);
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

        ++i;
    }

    return rv;
}


// ends encoding a datum
bool SerDesNative::End(Datum &Dat)
{
    bool rv = true;

    rv = rv && OctetInsert(mk_DatEnd);
    (void)Dat;

    return rv;
}


// encode a block length into the output stream
bool SerDesNative::LenInsert(size_t Len)
{
    bool rv = true;

    if (Len < 128)          // encodes into one octet
    {
        rv = rv && OctetInsert(Len);
    }
    else if (Len < 65536)   // encodes into three octets
    {
        rv = rv && OctetInsert(128);
        rv = rv && OctetInsert((Len >> 8) & 0xff);
        rv = rv && OctetInsert(Len & 0xff);
    }
    else                    // encodes into five octets
    {
        rv = rv && OctetInsert(129);
        rv = rv && LongInsert(Len);
    }

    return rv;
}


// encode an octet into the output stream
bool SerDesNative::OctetInsert(uint8_t Val)
{
    bool rv = (m_PtrStream != NULL);

    rv = rv && m_PtrStream->Write(Val);

    return rv;
}


// encode a short into the output stream
bool SerDesNative::ShortInsert(uint16_t Val)
{
    bool rv = true;
    size_t i = 0;
    uint8_t *pSrc = reinterpret_cast<uint8_t *>(&Val);

    if (cp::HostLittleEndian())
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
bool SerDesNative::LongInsert(uint32_t Val)
{
    bool rv = true;
    size_t i = 0;
    uint8_t *pSrc = reinterpret_cast<uint8_t *>(&Val);

    if (cp::HostLittleEndian())
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
bool SerDesNative::LongLongInsert(uint64_t Val)
{
    bool rv = true;
    size_t i = 0;
    uint8_t *pSrc = reinterpret_cast<uint8_t *>(&Val);

    if (cp::HostLittleEndian())
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
bool SerDesNative::Float32Insert(float Val)
{
    bool rv = true;
    size_t i = 0;
    uint8_t *pSrc = reinterpret_cast<uint8_t *>(&Val);

    if (cp::HostLittleEndian())
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
bool SerDesNative::Float64Insert(double Val)
{
    bool rv = true;
    size_t i = 0;
    uint8_t *pSrc = reinterpret_cast<uint8_t *>(&Val);

    if (cp::HostLittleEndian())
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


// encode a string into the output stream
bool SerDesNative::StringInsert(String const &Str)
{
    bool rv = (m_PtrStream != NULL);
    size_t size = Str.size();

    rv = rv && LenInsert(size);
    rv = rv && (m_PtrStream->ArrayWr(Str.c_str(), size) == size);

    return rv;
}


// encode a BLOB into the output stream
bool SerDesNative::BlobInsert(Buffer const &Buf)
{
    bool rv = (m_PtrStream != NULL);
    size_t size = Buf.LenGet();

    rv = rv && LenInsert(size);
    rv = rv && (m_PtrStream->Write(Buf, size) == size);

    return rv;
}


// decode a block length from the input stream
bool SerDesNative::LenExtract(size_t &Len)
{
    bool rv = true;
    uint8_t ch = 0;

    // get first byte of length
    rv = rv && OctetExtract(ch);

    if (rv)
    {
        // check for one byte length encoding
        if (ch < 128)
        {
            Len = ch;
        }
        else // check for three byte length encoding
        if (ch == 128)
        {
            rv = OctetExtract(ch);

            if (rv)
            {
                Len = ch << 8;

                rv = OctetExtract(ch);

                if (rv)
                {
                    Len |= ch;
                }
            }
        }
        else // check for five byte length encoding
        if (ch == 129)
        {
            uint32_t length;
            rv = LongExtract(length);

            if (rv)
            {
                Len =  length;
            }
        }
        else    // invalid first octet
        {
            rv = false;
        }
    }

    return rv;
}


// decode an octet from the input stream
bool SerDesNative::OctetExtract(uint8_t &Val)
{
    bool rv = (m_PtrStream != NULL);

    rv = rv && m_PtrStream->Read(Val);

    return rv;
}


// decode a short from the input stream
bool SerDesNative::ShortExtract(uint16_t &Val)
{
    bool rv = true;
    uint8_t *pDst = reinterpret_cast<uint8_t *>(&Val);
    size_t i;

    if (cp::HostLittleEndian())
    {
        for (i = sizeof(Val); i > 0; --i)
        {
            rv = rv && OctetExtract(pDst[i-1]);
        }
    }
    else
    {
        for (i = 0; i < sizeof(Val); ++i)
        {
            rv = rv && OctetExtract(pDst[i]);
        }
    }

    return rv;
}


// decode a long from the input stream
bool SerDesNative::LongExtract(uint32_t &Val)
{
    bool rv = true;
    uint8_t *pDst = reinterpret_cast<uint8_t *>(&Val);
    size_t i;

    if (cp::HostLittleEndian())
    {
        for (i = sizeof(Val); i > 0; --i)
        {
            rv = rv && OctetExtract(pDst[i-1]);
        }
    }
    else
    {
        for (i = 0; i < sizeof(Val); ++i)
        {
            rv = rv && OctetExtract(pDst[i]);
        }
    }

    return rv;
}


// decode a long long from the input stream
bool SerDesNative::LongLongExtract(uint64_t &Val)
{
    bool rv = true;
    uint8_t *pDst = reinterpret_cast<uint8_t *>(&Val);
    size_t i;

    if (cp::HostLittleEndian())
    {
        for (i = sizeof(Val); i > 0; --i)
        {
            rv = rv && OctetExtract(pDst[i-1]);
        }
    }
    else
    {
        for (i = 0; i < sizeof(Val); ++i)
        {
            rv = rv && OctetExtract(pDst[i]);
        }
    }

    return rv;
}


// decode a single float from the input stream
bool SerDesNative::Float32Extract(float &Val)
{
    bool rv = true;
    uint8_t *pDst = reinterpret_cast<uint8_t *>(&Val);
    size_t i;

    if (cp::HostLittleEndian())
    {
        for (i = sizeof(Val); i > 0; --i)
        {
            rv = rv && OctetExtract(pDst[i-1]);
        }
    }
    else
    {
        for (i = 0; i < sizeof(Val); ++i)
        {
            rv = rv && OctetExtract(pDst[i]);
        }
    }

    return rv;
}


// decode a double float from the input stream
bool SerDesNative::Float64Extract(double &Val)
{
    bool rv = true;
    uint8_t *pDst = reinterpret_cast<uint8_t *>(&Val);
    size_t i;

    if (cp::HostLittleEndian())
    {
        for (i = sizeof(Val); i > 0; --i)
        {
            rv = rv && OctetExtract(pDst[i-1]);
        }
    }
    else
    {
        for (i = 0; i < sizeof(Val); ++i)
        {
            rv = rv && OctetExtract(pDst[i]);
        }
    }

    return rv;
}


// decode a string from the input stream
bool SerDesNative::StringExtract(String &Str)
{
    bool rv = (m_PtrStream != NULL);
    size_t len = 0;
    Buffer buf;

    // extract the block length
    rv = rv && LenExtract(len);

    if (len > 0)
    {
        // resize buffer to accomodate length of string + terminator
        rv = rv && buf.Resize(len + sizeof(char));

        // read the block
        rv = rv && (m_PtrStream->Read(buf, len) == len);

        // terminate the string and assign result
        if (rv)
        {
            *buf.c_str(len) = '\0';
            Str = buf.c_str();
        }
    }
    else
    {
        Str.clear();
    }

    return rv;
}


// decode a BLOB from the input stream
bool SerDesNative::BlobExtract(Buffer &Buf)
{
    bool rv = (m_PtrStream != NULL);
    size_t len = 0;

    // extract the block length
    rv = rv && LenExtract(len);

    if (len > 0)
    {
        // resize buffer to accomodate length of block
        rv = rv && Buf.Resize(len);

        // read the block
        rv = rv && (m_PtrStream->Read(Buf, len) == len);
    }
    else
    {
        Buf.Resize(0);
    }

    return rv;
}


// decode a data value into a variant
bool SerDesNative::VariantExtract(Variant &Var, Variant::DataType_t Type)
{
    bool rv = false;

    switch (Type)
    {
    case Variant::dt_uint8:
        {
            uint8_t val;
            rv = OctetExtract(val);
            Var.Uint8Set(val);
        }
        break;

    case Variant::dt_int8:
        {
            int8_t val;
            rv = OctetExtract((uint8_t &)val);
            Var.Int8Set(val);
        }
        break;

    case Variant::dt_uint16:
        {
            uint16_t val;
            rv = ShortExtract(val);
            Var.Uint16Set(val);
        }
        break;

    case Variant::dt_int16:
        {
            int16_t val;
            rv = ShortExtract((uint16_t &)val);
            Var.Int16Set(val);
        }
        break;

    case Variant::dt_uint32:
        {
            uint32_t val;
            rv = LongExtract(val);
            Var.Uint32Set(val);
        }
        break;

    case Variant::dt_int32:
        {
            int32_t val;
            rv = LongExtract((uint32_t &)val);
            Var.Int32Set(val);
        }
        break;

    case Variant::dt_uint64:
        {
            uint64_t val;
            rv = LongLongExtract(val);
            Var.Uint64Set(val);
        }
        break;

    case Variant::dt_int64:
        {
            int64_t val;
            rv = LongLongExtract((uint64_t &)val);
            Var.Int64Set(val);
        }
        break;

    case Variant::dt_float32:
        {
            float val;
            rv = Float32Extract(val);
            Var.Float32Set(val);
        }
        break;

    case Variant::dt_float64:
        {
            double val;
            rv = Float64Extract(val);
            Var.Float64Set(val);
        }
        break;

    case Variant::dt_bool:
        {
            uint8_t ch;
            rv = OctetExtract(ch);
            Var.BoolSet(ch != k_OctetFalse);
        }
        break;

    case Variant::dt_string:
        {
            String str;
            rv = StringExtract(str);
            Var.StrSet(str);
        }
        break;

    case Variant::dt_blob:
        {
            Buffer buf;
            rv = BlobExtract(buf);
            Var.BufSet(buf);
        }
        break;

    case Variant::dt_none:
        Var = Variant();
        rv = true;
        break;

    default:
        break;
    }

    return rv;
}


bool SerDesNative::DecodeIdent()
{
    bool rv = true;
    uint8_t ch = 0;

    // get first byte
    rv = rv && OctetExtract(ch);

    // check if it is the magic value
    rv = rv && (ch == mk_Magic);

    // get second byte
    rv = rv && OctetExtract(ch);

    // check if it is the package start marker
    rv = rv && (ch == mk_Pkg);

    return rv;
}


bool SerDesNative::DecodeVersion()
{
    bool rv = true;
    uint8_t ch = 0;

    // no special handling of version at this time
    rv = rv && OctetExtract(ch);

    return rv;
}


bool SerDesNative::DecodeElement()
{
    bool rv = true;
    bool exitFlag = false;
    bool rootDatum = true;
    uint8_t ch = 0;
    Datum *pDat = NULL;
    String str;
    Variant var;

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
        rv = OctetExtract(ch);

        if (rv)
        {
            switch (ch)
            {
            case mk_PkgEnd:
                exitFlag = true;
                break;

            case mk_Dat:
                if (rootDatum)
                {
                    rootDatum = false;
                }
                else
                {
                    pDat->Add("");
                    pDat = &(pDat->Get());
                    m_DatStack.push_front(pDat);
                }
                break;

            case mk_DatEnd:
                rv = true;
                if (m_DatStack.size() > 1)
                {
                    m_DatStack.pop_front();
                    pDat = m_DatStack.front();
                }
                break;

            case mk_Chk:
                break;

            case mk_ChkEnd:
                break;

            case mk_Name:
            case mk_Val:
            case mk_Min:
            case mk_Max:
            case mk_Def:
            case mk_Units:
            case mk_Info:
            case mk_Choices:
                {
                    Datum::Attrib_t attrib = m_TagMap[static_cast<DataTag_t>(ch)];
                    Variant::DataType_t type;

                    rv = OctetExtract(ch);

                    if (rv)
                    {
                        type = static_cast<Variant::DataType_t>(ch);
                        rv = VariantExtract(var, type);
                    }

                    if (rv)
                    {
                        pDat->AttrSet(attrib, var);
                    }
                }
                break;

            default:    // unknown marker
                rv = false;
                break;
            }
        }
    }

    return rv;
}


bool SerDesNative::DecodeChecksum()
{
    bool rv = (m_PtrStream != NULL);
    uint32_t crcs = 0;
    uint32_t crcr = 0;
    uint8_t ch;
    size_t pos = m_PtrStream->Pos();

    // get what should be the checksum package start marker
    rv = rv && OctetExtract(ch) && (ch == mk_Chk);

    // get the checksum type marker
    rv = rv && OctetExtract(ch);

    if (rv)
    {
        switch (ch)
        {
        case Datum::ck_Crc32:
            // get the crc from the message
            rv = rv && LongExtract(crcs);

            if (rv)
            {
                // calculate crc of local copy
                crcr = m_PtrStream->Crc32Get(pos);

                // result good if they match
                rv = (crcs == crcr);
            }
            break;

        case Datum::ck_Md5Sum:
            rv = false;
            break;

        case Datum::ck_Sha1Sum:
            rv = false;
            break;

        default:
            rv = true;
            break;
        }
    }

    // get what should be the checksum package end marker
    rv = rv && OctetExtract(ch);

    return rv;
}


// create and return an instance
SerDes *SerDesNative::CreateInstance()
{
    return new (CP_NEW) SerDesNative;
}

}   // namespace cp
