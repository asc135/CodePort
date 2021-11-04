// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpString.h
//
//  Description:    String Facility.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-12-18  asc Added some useful string container definitions.
// ----------------------------------------------------------------------------

#ifndef CP_STRING_H
#define CP_STRING_H

#include <list>
#include <vector>

#include "cpPlatform.h"
#include "cpAlloc.h"

namespace cp
{

// define a String as a basic_string with a custom allocator using MemMgr.
typedef std::basic_string<char, std::char_traits<char>, Alloc<char> > String;

// prints formatted contents to stream
std::ostream &operator <<(std::ostream &Out, String const &Str);

// define some useful string container types
typedef std::list<String, Alloc<String> > StringList_t;
typedef std::vector<String, Alloc<String> > StringVec_t;

}   // namespace cp

#endif // CP_STRING_H
