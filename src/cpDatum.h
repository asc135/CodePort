// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpDatum.h
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
//  2012-12-05  asc Added SubCount() method.
//  2013-04-22  asc Added encode/decode to/from Buffer.
//  2013-06-27  asc Removed requirement for name and added array index operator.
//  2013-06-28  asc Made name and inert attributes instead of a members.
//  2013-07-17  asc Added Inert() static accessor method.
//  2013-07-19  asc Reverted inert to a member boolean.
// ----------------------------------------------------------------------------

#ifndef CP_DATUM_H
#define CP_DATUM_H

#include <map>

#include "cpVariant.h"

namespace cp
{

// forward references
class Datum;
class SerDes;
class StreamBase;

// ----------------------------------------------------------------------------

class Datum : public PooledBase
{
public:
    // attribute types
    enum Attrib_t
    {
        attr_None,
        attr_Name,
        attr_Val,
        attr_Min,
        attr_Max,
        attr_Def,
        attr_Units,
        attr_Info,
        attr_Choices
    };

    // checksum types
    enum CheckSum_t
    {
        ck_None,
        ck_Crc32,
        ck_Md5Sum,
        ck_Sha1Sum
    };

    // custom data types
    typedef std::list<Datum, Alloc<Datum> > Data_t;
    typedef std::map<Attrib_t, Variant, std::less<Attrib_t>, Alloc< std::pair<Attrib_t const, Variant> > > Attribs_t;

    // constructor
    Datum(String const &Name = "", bool Inert = false);

    // copy constructor
    Datum(Datum const &rhs);

    // destructor
    virtual ~Datum();

    // accessors
    String NameGet() const;                                 // return the datum name
    Variant const &Attr(Attrib_t Type) const;               // return reference to an attribute
    Variant const &Val() const;                             // return the value attribute
    Variant const &Min() const;                             // return the minimum attribute
    Variant const &Max() const;                             // return the maximum attribute
    Variant const &Def() const;                             // return the default attribute
    String UnitsGet() const;                                // return the datum units
    String InfoGet() const;                                 // return the datum info
    String ChoicesGet() const;                              // return the datum enumerated choices
    size_t SubCount() const { return m_Datums.size(); }     // return the number of subdatums
    static Datum &Inert();                                  // return an inert datum instance

    // datum manipulators and accessors
    bool Select(String const &Name);                        // select the named datum as current
    Datum &Get();                                           // return reference to current datum
    Datum &Get(String const &Name);                         // return reference to named sub-datum
    Datum &GetRecursive();                                  // return reference to recursive datum

    // object state accessors
    void Display(std::ostream &Log, uint32_t Level = 0);    // display contents in formatted form
    bool Validate() const;                                  // validates data
    bool IsActive() const { return !m_Inert; }              // returns true if not inert
    bool Inactive() const { return  m_Inert; }              // returns true if inert
    uint8_t DatumVersion() const;                           // returns the datum proper version

    // attribute iteration methods
    Attribs_t::const_iterator AttribBegin() const { return m_Attribs.begin(); }
    Attribs_t::const_iterator AttribEnd()   const { return m_Attribs.end();   }

    // operators
    Datum &operator=(Datum const &rhs);                     // standard assignment operator
    Datum &operator[](uint32_t Index);                      // returns reference to a subdatum by index position
    Datum &operator++();                                    // increments to the next virtual datum recursively
    bool operator!();                                       // returns false if current virtual datum is invalid

    // converting assignment operators
    Datum &operator=(uint32_t rhs);
    Datum &operator=(int32_t rhs);
    Datum &operator=(uint64_t rhs);
    Datum &operator=(int64_t rhs);
    Datum &operator=(float &rhs);
    Datum &operator=(double &rhs);
    Datum &operator=(bool rhs);
    Datum &operator=(String const &rhs);
    Datum &operator=(uint8_t const *rhs);
    Datum &operator=(char const *rhs);
    Datum &operator=(Buffer const &rhs);

    // conversion operators
    operator bool();
    operator uint32_t();
    operator int32_t();
    operator uint64_t();
    operator int64_t();
    operator float();
    operator double();
    operator String();
    operator uint8_t const *();
    operator char const *();

    // manipulators
    void NameSet(String const &Name);                       // set the datum name
    void AttrSet(Attrib_t Type, Variant const &Var);        // set an attribute
    void AttrDel(Attrib_t Type);                            // delete an attribute
    Variant &AttrSet(Attrib_t Type);                        // set an attribute
    Variant &ValSet();                                      // set the value attribute
    Variant &MinSet();                                      // set the minimum attribute
    Variant &MaxSet();                                      // set the maximum attribute
    Variant &DefSet();                                      // set the default attribute
    void UnitsSet(String const &Units);                     // set the datum units
    void InfoSet(String const &Info);                       // set the datum info
    void ChoicesSet(String const &Choices);                 // set the datum enumerated choices

    // tree manipulators
    bool Rewind();                                          // rewinds to the first datum
    bool Next()    { return Advance(false); }               // advances iterator to next sub-datum
    bool Recurse() { return Advance(true);  }               // advances iterator recursively to next datum
    bool Erase();                                           // removes the active datum
    bool Erase(String const &Name);                         // removes the specified datum
    void Clear();                                           // clears the datum's contents

    bool Encode(Buffer &Buf,
                String const &Enc = k_SerDesNative,
                CheckSum_t ChkMode = ck_None);              // encodes collection to a buffer

    bool Encode(StreamBase &Stream,
                String const &Enc = k_SerDesNative,
                CheckSum_t ChkMode = ck_None);              // encodes collection to a cp stream object

    bool Decode(Buffer const &Buf,
                bool Check = false,
                String const &Enc = k_SerDesAuto);          // decodes collection from a buffer

    bool Decode(StreamBase &Stream,
                bool Check = false,
                String const &Enc = k_SerDesAuto);          // decodes collection from a cp stream object

    Datum &Add();                                           // adds an anonymous datum to the collection
    Datum &Add(String const &Name);                         // adds a named datum to the collection
    Datum &Add(Datum const &rhs);                           // adds an existing datum to the collection

protected:

    Data_t::iterator Find(String const &Name);              // find a datum with the specified name
    Data_t::const_iterator Find(String const &Name) const;  // find a datum with the specified name
    bool Find(Attrib_t Type, Attribs_t::iterator &It);      // find an attribute with the specified type
    bool Find(Attrib_t Type,
              Attribs_t::const_iterator &It) const;         // find an attribute with the specified type

    size_t Encode(SerDes *pSerDes);                         // encodes datum and its sub-data recursively
    bool Advance(bool Recurse = false);                     // advance the current datum pointer

    bool                m_Inert;                            // indicates an inert instance that won't allow state change
    bool                m_Self;                             // true to indicate current datum is reference to itself
    Attribs_t           m_Attribs;                          // the datum value attribute element(s)
    Data_t::iterator    m_IterData;                         // iterator to the current datum
    Data_t              m_Datums;                           // storage for sub-datum instances
};

}   // namespace cp

#endif  // CP_DATUM_H
