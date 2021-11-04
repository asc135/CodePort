// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpDatum.cpp
//
//  Description:    Key-Value data container.
//
//  Platform:       common
//
//  History:
//  2011-05-09  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-08-13  asc Renamed some public methods.
//  2012-11-14  asc Replaced Next() and Recurse() with common implementation.
//  2012-11-16  asc Major overhaul of public interface.
//  2012-12-07  asc Fixed problem in Select() which was not clearing m_Self.
//  2013-04-22  asc Added encode/decode to/from Buffer.
//  2013-06-27  asc Removed requirement for name and added array index operator.
//  2013-06-28  asc Made name and inert attributes instead of a members.
//  2013-07-17  asc Added Inert() static accessor method.
//  2013-07-19  asc Reverted inert to a member boolean.
// ----------------------------------------------------------------------------

#include "cpDatum.h"
#include "cpUtil.h"
#include "cpSerDes.h"
#include "cpVariant.h"
#include "cpStreamBuf.h"
#include "cpSerDesFactory.h"

namespace cp
{

// global inert datum used for invalid reference
static Datum g_InertDatum("-Inert-", true);

// global inert variant used for invalid reference
static Variant g_InertValue(true);

// ----------------------------------------------------------------------------


// constructor
Datum::Datum(String const &Name, bool Inert) :
    m_Inert(Inert),
    m_Self(true)
{
    // set name manually if inert
    if (m_Inert)
    {
        m_Attribs[attr_Name].StrSet(Name);
    }

    // set the name
    NameSet(Name);

    // initialize iterator
    Rewind();
}


// copy constructor
Datum::Datum(Datum const &rhs) :
    m_Inert(false)
{
    *this = rhs;
}


// destructor
Datum::~Datum()
{
}


// return the datum name
String Datum::NameGet() const
{
    return Attr(attr_Name).StrGet();
}


// return reference to an attribute
Variant const &Datum::Attr(Attrib_t Type) const
{
    Attribs_t::const_iterator i;

    i = m_Attribs.find(Type);

    if (i == m_Attribs.end())
    {
        return g_InertValue;
    }
    else
    {
        return i->second;
    }
}


// return the value attribute
Variant const &Datum::Val() const
{
    return Attr(attr_Val);
}


// return the minimum attribute
Variant const &Datum::Min() const
{
    return Attr(attr_Min);
}


// return the maximum attribute
Variant const &Datum::Max() const
{
    return Attr(attr_Max);
}


// return the default attribute
Variant const &Datum::Def() const
{
    return Attr(attr_Def);
}


// return the datum units
String Datum::UnitsGet() const
{
    return Attr(attr_Units).StrGet();
}


// return the datum info
String Datum::InfoGet() const
{
    return Attr(attr_Info).StrGet();
}


// return the datum enumerated choices
String Datum::ChoicesGet() const
{
    return Attr(attr_Choices).StrGet();
}


// return an inert datum instance
Datum &Datum::Inert()
{
    return g_InertDatum;
}


// select the named datum as current
bool Datum::Select(String const &Name)
{
    bool rv = false;

    // do nothing if inert
    if (Inactive())
    {
        return false;
    }

    Data_t::iterator i = Find(Name);

    if (i != m_Datums.end())
    {
        m_Self = false;
        rv = true;
    }

    m_IterData = i;

    return rv;
}


// return reference to current datum
Datum &Datum::Get()
{
    if (m_Self)
    {
        return *this;
    }

    if (m_IterData == m_Datums.end())
    {
        return g_InertDatum;
    }
    else
    {
        return *m_IterData;
    }
}


// return reference to named sub-datum
Datum &Datum::Get(String const &Name)
{
    // do nothing if inert
    if (Inactive())
    {
        return g_InertDatum;
    }

    Data_t::iterator i = Find(Name);

    if (i == m_Datums.end())
    {
        return g_InertDatum;
    }
    else
    {
        return *i;
    }
}


// return reference to virtual datum
Datum &Datum::GetRecursive()
{
    if (m_Self)
    {
        return *this;
    }

    if (m_IterData == m_Datums.end())
    {
        return g_InertDatum;
    }
    else
    {
        return m_IterData->GetRecursive();
    }
}


// display contents in formatted form
void Datum::Display(std::ostream &Log, uint32_t Level)
{
    Data_t::iterator i = m_Datums.begin();
    Attribs_t::iterator j;
    uint32_t indent = Level * 4;    // convert level to spaces

    // set up to display floats with full precision
    Log << std::fixed;

    Log << GenStr(indent) << "\n";
    Log << GenStr(indent) << "Datum Contents: '" << NameGet() << "' " << this << "\n";
    Log << GenStr(indent) << "----------------------------------\n";

    if (Inactive())
    {
        Log << GenStr(indent) << "INERT\n";
        Log << GenStr(indent) << "----------------------------------\n";
        return;
    }

    Log << GenStr(indent) << "Type    = " << g_DataTypeNames[Val().TypeGet()] << "\n";

    if (UnitsGet().size() > 0)
    {
        Log << GenStr(indent) << "Units   = " << UnitsGet() << "\n";
    }

    if (InfoGet().size() > 0)
    {
        Log << GenStr(indent) << "Info    = " << InfoGet() << "\n";
    }

    if (ChoicesGet().size() > 0)
    {
        Log << GenStr(indent) << "Choices = " << ChoicesGet() << "\n";
    }

    Log << GenStr(indent) << "Value   = " << Val() << "\n";

    if (Find(attr_Min, j))
    {
        Log << GenStr(indent) << "MinVal  = " << j->second << "\n";
    }

    if (Find(attr_Max, j))
    {
        Log << GenStr(indent) << "MaxVal  = " << j->second << "\n";
    }

    if (Find(attr_Def, j))
    {
        Log << GenStr(indent) << "Default = " << j->second << "\n";
    }

    // display sub-data
    while (i != m_Datums.end())
    {
        i->Display(Log, Level + 1);
        ++i;
    }

    Log << GenStr(indent) << "----------------------------------" << std::endl;
}


// validates data
bool Datum::Validate() const
{
    bool rv = true;

    // do nothing if inert
    if (Inactive())
    {
        return false;
    }

#if 0
    Data_t::const_iterator i = m_Datums.begin();

    // (.)(.) need to implement Validate()
    // validate this datum
    switch (TypeGet())
    {
    case Variant::dt_bool:
    case Variant::dt_uint:
    case Variant::dt_int:
    case Variant::dt_float:
        break;

    default:
        break;
    }

    // validate sub-data
    while (i != m_Datums.end())
    {
        rv = rv && i->Validate();
        ++i;
    }
#endif
    return rv;
}


// returns the datum proper version
uint8_t Datum::DatumVersion() const
{
    return 0;
}


// assignment operator
Datum &Datum::operator=(Datum const &rhs)
{
    // detect self-assignment or inert status
    if ((this == &rhs) || Inactive())
    {
        return *this;
    }

    m_Inert     = rhs.m_Inert;
    m_Self      = rhs.m_Self;

    // remove all previous attributes
    m_Attribs.clear();

    m_Attribs = rhs.m_Attribs;
    m_Datums  = rhs.m_Datums;

    Rewind();

    return *this;
}


// returns reference to a subdatum by index position
Datum &Datum::operator[](uint32_t Index)
{
    // do nothing if inert
    if (Inactive())
    {
        return g_InertDatum;
    }

    Data_t::iterator i = m_Datums.begin();

    while (Index && (i != m_Datums.end()))
    {
        ++i;
        --Index;
    }

    if (i != m_Datums.end())
    {
        return *i;
    }
    else
    {
        return g_InertDatum;
    }
}


// increments to the next virtual datum recursively
Datum &Datum::operator++()
{
    if (IsActive() && Advance(true))
    {
        return GetRecursive();
    }

    return g_InertDatum;
}


// returns false if current virtual datum is invalid
bool Datum::operator!()
{
    return (GetRecursive().IsActive());
}


// converting assignment operators
Datum &Datum::operator=(uint32_t rhs)
{
    if (IsActive())
    {
        ValSet().Uint32Set(rhs);
    }

    return *this;
}


Datum &Datum::operator=(int32_t rhs)
{
    if (IsActive())
    {
        ValSet().Int32Set(rhs);
    }

    return *this;
}


Datum &Datum::operator=(uint64_t rhs)
{
    if (IsActive())
    {
        ValSet().Uint64Set(rhs);
    }

    return *this;
}


Datum &Datum::operator=(int64_t rhs)
{
    if (IsActive())
    {
        ValSet().Int64Set(rhs);
    }

    return *this;
}


Datum &Datum::operator=(float &rhs)
{
    if (IsActive())
    {
        ValSet().Float32Set(rhs);
    }

    return *this;
}


Datum &Datum::operator=(double &rhs)
{
    if (IsActive())
    {
        ValSet().Float64Set(rhs);
    }

    return *this;
}


Datum &Datum::operator=(bool rhs)
{
    if (IsActive())
    {
        ValSet().BoolSet(rhs);
    }

    return *this;
}


Datum &Datum::operator=(String const &rhs)
{
    if (IsActive())
    {
        ValSet().StrSet(rhs);
    }

    return *this;
}


Datum &Datum::operator=(uint8_t const *rhs)
{
    if (IsActive())
    {
        ValSet().StrSet(reinterpret_cast<char const *>(rhs));
    }

    return *this;
}


Datum &Datum::operator=(char const *rhs)
{
    if (IsActive())
    {
        ValSet().StrSet(rhs);
    }

    return *this;
}


Datum &Datum::operator=(Buffer const &rhs)
{
    if (IsActive())
    {
        ValSet().BufSet(rhs);
    }

    return *this;
}


// conversion operators
Datum::operator bool()
{
    return Val().BoolGet();
}


Datum::operator uint32_t()
{
    return Val().Uint32Get();
}


Datum::operator int32_t()
{
    return Val().Int32Get();
}


Datum::operator uint64_t()
{
    return Val().Uint64Get();
}


Datum::operator int64_t()
{
    return Val().Int64Get();
}


Datum::operator float()
{
    return Val().Float32Get();
}


Datum::operator double()
{
    return Val().Float64Get();
}


Datum::operator String()
{
    return Val().StrGet();
}


Datum::operator uint8_t const *()
{
    return Val().BufGet().u_str();
}


Datum::operator char const *()
{
    return Val().StrGet().c_str();
}


// set the datum name
void Datum::NameSet(String const &Name)
{
    if (IsActive())
    {
        if (Name.length() > 0)
        {
            m_Attribs[attr_Name].StrSet(Name);
        }
        else
        {
            AttrDel(attr_Name);
        }
    }
}


// set an attribute
void Datum::AttrSet(Attrib_t Type, Variant const &Var)
{
    if (IsActive())
    {
        m_Attribs[Type] = Var;
    }
}


// delete an attribute
void Datum::AttrDel(Attrib_t Type)
{
    Attribs_t::iterator i;

    if (IsActive())
    {
        i = m_Attribs.find(Type);

        if (i != m_Attribs.end())
        {
            m_Attribs.erase(i);
        }
    }
}


// set an attribute
Variant &Datum::AttrSet(Attrib_t Type)
{
    if (IsActive())
    {
        return m_Attribs[Type];
    }
    else
    {
        return g_InertValue;
    }
}


// set the value attribute
Variant &Datum::ValSet()
{
    return AttrSet(attr_Val);
}


// set the minimum attribute
Variant &Datum::MinSet()
{
    return AttrSet(attr_Min);
}


// set the maximum attribute
Variant &Datum::MaxSet()
{
    return AttrSet(attr_Max);
}


// set the default attribute
Variant &Datum::DefSet()
{
    return AttrSet(attr_Def);
}


// set the datum units
void Datum::UnitsSet(String const &Units)
{
    if (IsActive())
    {
        m_Attribs[attr_Units].StrSet(Units);
    }
}


// set the datum info
void Datum::InfoSet(String const &Info)
{
    if (IsActive())
    {
        m_Attribs[attr_Info].StrSet(Info);
    }
}


// set the datum enumerated choices
void Datum::ChoicesSet(String const &Choices)
{
    if (IsActive())
    {
        m_Attribs[attr_Choices].StrSet(Choices);
    }
}


// rewinds to the first datum
bool Datum::Rewind()
{
    bool rv = false;

    // make itself the current datum
    m_Self = true;

    // move iterator to the first sub-datum
    m_IterData = m_Datums.begin();

    // check if resulting datum exists
    rv = (m_IterData != m_Datums.end());

    return rv;
}


// clears the datum's contents
void Datum::Clear()
{
    // do nothing if inert
    if (Inactive())
    {
        return;
    }

    // assign it to a default constructed temporary datum with the same name
    *this = Datum(NameGet());
}


// encodes collection to a buffer
bool Datum::Encode(Buffer &Buf, String const &Enc, CheckSum_t ChkMode)
{
    bool rv = false;
    StreamBuf strm;

    rv = Encode(strm, Enc, ChkMode);
    Buf = strm;

    return rv;
}


// encodes collection to a cp stream object
bool Datum::Encode(StreamBase &Stream, String const &Enc, CheckSum_t ChkMode)
{
    bool rv = true;
    SerDes *pSerDes = NULL;

    // do nothing if inert
    if (Inactive())
    {
        return false;
    }

    // get a serializer
    pSerDes = SerDesFactory::InstanceGet()->SerDesGet(Enc);

    // make sure specified encoder has a handler
    if (pSerDes == NULL)
    {
        LogErr << "Datum::Encode(): Failed to locate serializer: "
               << Enc << " for Datum: " << NameGet() << std::endl;
        return false;
    }

    // serialize to the stream
    rv = rv && pSerDes->Open(Stream, DatumVersion());
    rv = rv && Encode(pSerDes);
    rv = rv && pSerDes->Close(ChkMode);

    // return serializer
    SerDesFactory::InstanceGet()->SerDesPut(pSerDes);

    return rv;
}


// decodes collection from a buffer
bool Datum::Decode(Buffer const &Buf, bool Check, String const &Enc)
{
    bool rv = false;
    StreamBuf strm;

    strm = Buf;
    rv = Decode(strm, Check, Enc);

    return rv;
}


// decodes collection from a cp stream object
bool Datum::Decode(StreamBase &Stream, bool Check, String const &Enc)
{
    bool rv = false;
    SerDes *pSerDes = NULL;
    String encoder;

    // do nothing if inert
    if (Inactive())
    {
        return false;
    }

    // determine serializer type
    if (Enc == "Auto")
    {
        encoder = SerDesFactory::InstanceGet()->DetectEncoding(Stream);
    }
    else
    {
        encoder = Enc;
    }

    // check that an assignment took place
    if (encoder.size() == 0)
    {
        LogErr << "Datum::Decode(): Serializer type indeterminate for Datum: "
               << NameGet() << std::endl;
        return false;
    }

    // get a serializer
    pSerDes = SerDesFactory::InstanceGet()->SerDesGet(encoder);

    // make sure specified serializer has a handler
    if (pSerDes == NULL)
    {
        LogErr << "Datum::Decode(): Serializer not available: "
               << encoder << " for Datum: " << NameGet() << std::endl;
        return false;
    }

    // deserialize the stream
    rv = pSerDes->Decode(Stream, *this, Check);

    // return serializer
    SerDesFactory::InstanceGet()->SerDesPut(pSerDes);

    return rv;
}


// removes the active datum
bool Datum::Erase()
{
    bool rv = false;

    if (m_IterData != m_Datums.end())
    {
        m_Datums.erase(m_IterData);
        // iterators become invalidated so force them to end
        m_IterData = m_Datums.end();
        rv = true;
    }

    return rv;
}


// removes the specified datum
bool Datum::Erase(String const &Name)
{
    bool rv = false;
    Data_t::iterator i = Find(Name);

    if (i != m_Datums.end())
    {
        m_Datums.erase(i);
        // iterators become invalidated so force them to end
        m_IterData = m_Datums.end();
        rv = true;
    }

    return rv;
}


// adds an anonymous datum to the collection
Datum &Datum::Add()
{
    // do nothing if inert
    if (Inactive())
    {
        return g_InertDatum;
    }

    return Add("");
}


// adds a named datum to the collection
Datum &Datum::Add(String const &Name)
{
    return Add(Datum(Name));
}


// adds an existing datum to the collection
Datum &Datum::Add(Datum const &rhs)
{
    // do nothing if inert
    if (Inactive())
    {
        return g_InertDatum;
    }

    Data_t::iterator i = Find(rhs.NameGet());

    // if it already exists just assign the new value otherwise add it to the collection
    if ((i != m_Datums.end()) && (rhs.NameGet().length() > 0))
    {
        *i = rhs;
    }
    else
    {
        m_Datums.push_back(rhs);

        i = m_Datums.end();
        --i;
    }

    // set newly added (or updated) sub-datum to be current
    m_IterData = i;
    m_Self = false;

    if (i != m_Datums.end())
    {
        return *i;
    }

    return g_InertDatum;
}


// find a datum with the specified name
Datum::Data_t::iterator Datum::Find(String const &Name)
{
    Data_t::iterator i = m_Datums.begin();

    while (i != m_Datums.end())
    {
        if (i->NameGet() == Name)
        {
            break;
        }

        ++i;
    }

    return i;
}


// find an attribute with the specified type
bool Datum::Find(Attrib_t Type, Attribs_t::iterator &It)
{
    It = m_Attribs.find(Type);
    return (It != m_Attribs.end());
}


// find a datum with the specified name
Datum::Data_t::const_iterator Datum::Find(String const &Name) const
{
    return Find(Name);
}


// find an attribute with the specified type
bool Datum::Find(Attrib_t Type, Attribs_t::const_iterator &It) const
{
    It = m_Attribs.find(Type);
    return (It != m_Attribs.end());
}


// encodes contents recursively in serialized form
size_t Datum::Encode(SerDes *pSerDes)
{
    size_t size = 0;
    Data_t::iterator i = m_Datums.begin();

    // encode this datum
    size += pSerDes->Start(*this);

    // encode any sub-data
    while (i != m_Datums.end())
    {
        size += i->Encode(pSerDes);
        ++i;
    }

    // close out the datum
    size += pSerDes->End(*this);

    return size;
}


// advance the current datum pointer
bool Datum::Advance(bool Recurse)
{
    bool rv = false;
    bool rewind = true;

    // do nothing if inert
    if (Inactive())
    {
        return false;
    }

    // advance to next datum
    if (m_Self)
    {
        // if current datum was self, negate that
        m_Self = false;
        m_IterData = m_Datums.begin();
    }
    else
    {
        // current datum was not self so iterate
        if (m_IterData != m_Datums.end())
        {
            if (Recurse)
            {
                if (m_IterData->Advance(true))
                {
                    rewind = false;
                }
                else
                {
                    // reached end node of branch, so advance this level
                    ++m_IterData;
                }
            }
            else
            {
                // not recursing so just advance this level
                ++m_IterData;
            }
        }
    }

    // check if resulting datum is valid
    rv = m_IterData != m_Datums.end();

    // if iterator points to a valid datum, rewind it
    if (rv && rewind)
    {
        m_IterData->Rewind();
    }

    return rv;
}

}   // namespace cp
