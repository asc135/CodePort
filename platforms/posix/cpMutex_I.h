// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpMutex_I.h
//
//  Description:    Mutex Facility.
//
//  Platform:       posix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_MUTEX_I_H
#define CP_MUTEX_I_H

#include <pthread.h>

namespace cp
{

typedef pthread_mutex_t Mutex_t;

}   // namespace cp

#endif  // CP_MUTEX_I_H
