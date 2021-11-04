// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSemLite_I.h
//
//  Description:    Lightweight semaphore facility using native implementation.
//
//  Platform:       mswin
//
//  History:
//  2012-12-10  asc Creation.
// ----------------------------------------------------------------------------

#ifndef CP_SEMLITE_I_H
#define CP_SEMLITE_I_H

#include <semaphore.h>

namespace cp
{

typedef HANDLE SemLite_t;

}   // namespace cp

#endif  // CP_SEMLITE_I_H
