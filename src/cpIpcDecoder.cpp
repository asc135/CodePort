// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2013 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcDecoder.cpp
//
//  Description:    IPC message decoder.
//
//  Platform:       common
//
//  History:
//  2013-01-18  asc Creation.
// ----------------------------------------------------------------------------

#include "cpIpcDecoder.h"

namespace cp
{

// copy constructor
IpcDecoder::IpcDecoder(IpcDecoder &rhs) :
    m_Message("Decoded")
{
    // invoke the assignment operator
    *this = rhs;
}


// operators
IpcDecoder &IpcDecoder::operator=(IpcDecoder &rhs)
{
    // check for self assignment
    if (this == &rhs)
    {
        return *this;
    }

    // this is disabled (private), so nothing is copied

    return *this;
}


// load a segment list into the decoder
bool IpcDecoder::LoadSegment(IpcSegment *pSeg)
{
    bool rv = false;

    m_Buffer.Clear();
    m_Message.Clear();
    m_Stream.Clear();

    if (pSeg)
    {
        // load the segment stream
        m_Stream.SegmentInject(pSeg);

        // decode the message
        switch (pSeg->MsgType())
        {
        case cp::IpcSegment::msg_Raw:
            // intentional fall-through

        case cp::IpcSegment::msg_Control:
            // copy it into the buffer
            m_Stream.Seek(0);
            rv = (m_Stream.Read(m_Buffer, m_Stream.LenGet()) == m_Stream.LenGet());
            break;

        case cp::IpcSegment::msg_Datum:
            // attempt to decode the datum
            rv = m_Message.Decode(m_Stream);
            break;

        default:
            break;
        }
    }

    return rv;
}


}   // namespace cp
