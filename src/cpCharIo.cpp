// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpCharIo.cpp
//
//  Description:    Character I/O wrapper class.
//
//  Platform:       common
//
//  History:
//  2011-04-30  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#include "cpCharIo.h"

namespace cp
{

// constructor
CharIo::CharIo(String const &Name, IoDev *pDevice) :
    Base(Name),
    m_PtrIoDev(pDevice)
{
}


// destructor
CharIo::~CharIo()
{
}


// get a character from I/O device
bool CharIo::CharGet(char &Ch)
{
    bool rv = false;

    if ((m_PtrIoDev != NULL) && (m_PtrIoDev->Recv(&Ch, 1) == 1))
    {
        rv = true;
    }

    return rv;
}


// put a character to I/O device
bool CharIo::CharPut(char Ch)
{
    bool rv = false;

    if ((m_PtrIoDev != NULL) && (m_PtrIoDev->Send(&Ch, 1) == 1))
    {
        rv = true;
    }

    return rv;
}


// prints a null-terminated C string
bool CharIo::Print(char const *Buf)
{
    bool rv = false;
    size_t len = 0;

    if (Buf != NULL)
    {
        len = strlen(Buf);
    }

    if ((m_PtrIoDev != NULL) && (m_PtrIoDev->Send(Buf, len) == static_cast<ssize_t>(len)))
    {
        rv = true;
    }

    return rv;
}


// flush device I/O buffers
void CharIo::Flush()
{
    if (m_PtrIoDev != NULL)
    {
        m_PtrIoDev->Flush();
    }
}


// cancel pended I/O operations
void CharIo::Cancel()
{
    if (m_PtrIoDev != NULL)
    {
        m_PtrIoDev->Cancel();
    }
}

}   // namespace cp
