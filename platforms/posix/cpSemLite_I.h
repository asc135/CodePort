// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSemLite_I.h
//
//  Description:    Semaphore facility using lightweight implementation.
//
//  Platform:       posix
//
//  History:
//  2012-08-03  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-11-28  asc Added enabled and spare flags.
// ----------------------------------------------------------------------------

#ifndef CP_SEMLITE_I_H
#define CP_SEMLITE_I_H

#include <pthread.h>

namespace cp
{

struct SemLite_t
{
    bool            l_ok;
    bool            c_ok;
    bool            enabled;
    bool            spare;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    uint32_t        count;
};

}   // namespace cp

#endif  // CP_SEMLITE_I_H
