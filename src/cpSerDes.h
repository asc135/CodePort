// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSerDes.h
//
//  Description:    Serializer/Deserializer base class.
//
//  Platform:       common
//
//  History:
//  2011-05-16  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_SERDES_H
#define CP_SERDES_H

#include "cpSerDesFactory.h"
#include "cpDatum.h"

namespace cp
{

class StreamBase;

typedef std::list<Datum *, Alloc<Datum *> > DatumStack_t;

// ----------------------------------------------------------------------------

class SerDes
{
public:
    // enumerations
    enum DecoderStates { st_ident, st_version, st_elements, st_check, st_done, st_NumStates };

    // constructor
    SerDes();

    // destructor
    virtual ~SerDes();

    // accessors
    String const &NameGet() { return m_Name; }              // return the serializer name
    virtual bool CheckEncoding(StreamBase &Stream);         // check if encoding is in this serializer's format

    // manipulators
    virtual bool Open(StreamBase &Stream, uint8_t Ver);     // generates encoding header
    virtual bool Close(Datum::CheckSum_t ChkMode);          // generates encoding footer
    virtual bool Start(Datum &Dat);                         // starts encoding a datum
    virtual bool End(Datum &Dat);                           // ends encoding a datum

    bool Decode(StreamBase &Stream,
                Datum &Dat, bool Check);                    // decodes stream to a datum

    friend class SerDesFactory;                             // needs to call CreateInstance().

protected:
    virtual bool DecodeIdent();
    virtual bool DecodeVersion();
    virtual bool DecodeElement();
    virtual bool DecodeChecksum();
    virtual SerDes *CreateInstance() = 0;                   // create and return an instance

    void Clear();                                           // reset the member variables to a known state
    bool LineGet(String &Line);                             // get a line of text from the stream
    bool ExecDecodeState(DecoderStates &State, bool Check); // exec the decoder state machine
    Variant::DataType_t TypeParse(String const &Type);      // parse a string into a data type

    String              m_Name;                             // encoder name/description
    StreamBase         *m_PtrStream;                        // pointer to encoding stream
    DatumStack_t        m_DatStack;                         // stack for recursing datums during decode
};

}   // namespace cp

#endif  // CP_SERDES_H
