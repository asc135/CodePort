// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSerDes.cpp
//
//  Description:    Serializer/Deserializer base class.
//
//  Platform:       common
//
//  History:
//  2011-05-16  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#include "cpSerDes.h"
#include "cpStreamBase.h"

namespace cp
{

// constructor
SerDes::SerDes() :
    m_Name("None"),
    m_PtrStream(NULL)
{
}


// destructor
SerDes::~SerDes()
{
}


// check if encoding is in this serializer's format
bool SerDes::CheckEncoding(StreamBase &Stream)
{
    (void)Stream;
    return false;
}


// generates encoding header
bool SerDes::Open(StreamBase &Stream, uint8_t Ver)
{
    m_PtrStream = &Stream;
    (void)Ver;
    return true;
}


// generates encoding footer
bool SerDes::Close(Datum::CheckSum_t ChkMode)
{
    m_PtrStream = NULL;
    (void)ChkMode;
    return true;
}


// starts encoding a datum
bool SerDes::Start(Datum &Dat)
{
    (void)Dat;
    return true;
}


// ends encoding a datum
bool SerDes::End(Datum &Dat)
{
    (void)Dat;
    return true;
}


// decodes stream to a datum
bool SerDes::Decode(StreamBase &Stream, Datum &Dat, bool Check)
{
    bool rv = true;
    bool exitFlag = false;
    DecoderStates state = st_ident;

    // clear the data structures
    Dat.Clear();
    Clear();

    // assign the stream object and insert the datum into the stack
    m_PtrStream = &Stream;
    m_PtrStream->Seek(0);
    m_DatStack.push_front(&Dat);

    // perform the decode
    while (!exitFlag)
    {
        rv = ExecDecodeState(state, Check);
        exitFlag = ((state == st_done) || (rv == false));
    }

    // clear datum if decode failed
    if (rv == false)
    {
        Dat.Clear();
    }

    return rv;
}


bool SerDes::DecodeIdent()
{
    return true;
}


bool SerDes::DecodeVersion()
{
    return true;
}


bool SerDes::DecodeElement()
{
    return true;
}


bool SerDes::DecodeChecksum()
{
    return true;
}


// reset the member variables to a known state
void SerDes::Clear()
{
    m_PtrStream = NULL;
    m_DatStack.clear();
}


// get a line of text from the stream
bool SerDes::LineGet(String &Line)
{
    return m_PtrStream->ReadLine(Line);
}


// exec the decoder state machine
bool SerDes::ExecDecodeState(DecoderStates &State, bool Check)
{
    bool rv = false;

    // select action based on current decoder state
    switch (State)
    {
    case st_ident:      // identify input stream encoding
        rv = DecodeIdent();

        if (rv)
        {
            State = st_version;
        }
        break;

    case st_version:    // obtain the package encoding version
        rv = DecodeVersion();

        if (rv)
        {
            State = st_elements;
        }
        break;

    case st_elements:   // decode datum elements
        rv = DecodeElement();

        if (rv)
        {
            State = Check ? st_check : st_done;
        }
        break;

    case st_check:      // decode and validate package checksum
        rv = DecodeChecksum();

        if (rv)
        {
            State = st_done;
        }
        break;

    default:
        break;
    }

    return rv;
}


// parse a string into a data type
Variant::DataType_t SerDes::TypeParse(String const &Type)
{
    uint32_t i = 0;
    bool exitFlag = false;

    while (!exitFlag)
    {
        if (Type == g_DataTypeNames[i])
        {
            exitFlag = true;
        }
        else
        {
            ++i;
        }

        if (*g_DataTypeNames[i] == 0)
        {
            i = 0;
            exitFlag = true;
        }
    }

    return static_cast<Variant::DataType_t>(i);
}

}   // namespace cp
