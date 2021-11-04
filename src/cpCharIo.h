// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpCharIo.h
//
//  Description:    Character I/O wrapper class.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_CHARIO_H
#define CP_CHARIO_H

#include "cpIoDev.h"

namespace cp
{

class CharIo : public Base
{
public:
    // constructor
    CharIo(String const &Name = "Anonymous Char I/O Object", IoDev *pDevice = NULL);

    // destructor
    ~CharIo();

    // accessors
    bool CharGet(char &Ch);                                 // get a character from I/O device
    bool CharPut(char Ch);                                  // put a character to I/O device
    bool Print(char const *Buf);                            // prints a null-terminated C string

    // manipulators
    void Flush();                                           // flush device I/O buffers
    void Cancel();                                          // cancel pended I/O operations

private:
    IoDev              *m_PtrIoDev;                         // Underlying I/O device
};

}   // namespace cp

#endif  // CP_CHARIO_H
