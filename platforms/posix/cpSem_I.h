// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSem_I.h
//
//  Description:    Semaphore facility using native implementation.
//
//  Platform:       posix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_SEM_I_H
#define CP_SEM_I_H

#include <semaphore.h>

namespace cp
{

typedef sem_t Sem_t;

}   // namespace cp

#endif  // CP_SEM_I_H
