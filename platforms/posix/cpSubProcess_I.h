// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2022 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSubProcess_I.h
//
//  Description:    Creates a subprocess with a pipe to parent.
//
//  Platform:       posix
//
//  History:
//  2022-02-02  asc Creation.
// ----------------------------------------------------------------------------

#ifndef CP_SUBPROCESS_I_H
#define CP_SUBPROCESS_I_H

#include <cstdio>

namespace cp
{

typedef FILE SubProcessHandle_t;

}   // namespace cp

#endif // CP_SUBPROCESS_I_H