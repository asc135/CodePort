// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpString.cpp
//
//  Description:    String Facility.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#include "cpString.h"

namespace cp
{

// prints formatted contents to stream
std::ostream &operator <<(std::ostream &Out, String const &Str)
{
    Out << Str.c_str();
    return Out;
}

}   // namespace cp
