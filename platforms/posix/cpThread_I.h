// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpThread_I.h
//
//  Description:    Thread Facility.
//
//  Platform:       posix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-09-28  asc Replaced cp::Sem with cp::SemLite.
//  2013-01-09  asc Moved state mutex and suspend semaphore into main class.
//  2013-02-15  asc Moved priority and stack size into common implementation.
// ----------------------------------------------------------------------------

#ifndef CP_THREAD_I_H
#define CP_THREAD_I_H

#include <pthread.h>

namespace cp
{

typedef pthread_t Thread_t;

}   // namespace cp

#endif  // CP_THREAD_I_H
