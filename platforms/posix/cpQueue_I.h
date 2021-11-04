// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpQueue_I.h
//
//  Description:    Inter-Process Message Queue Facility.
//
//  Platform:       posix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_QUEUE_I_H
#define CP_QUEUE_I_H

#include <mqueue.h>

namespace cp
{

typedef mqd_t Queue_t;

}   // namespace cp

#endif  // CP_QUEUE_I_H
