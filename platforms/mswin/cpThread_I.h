// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpThread_I.h
//
//  Description:    Thread Facility.
//
//  Platform:       mswin
//
//  History:
//  2012-12-11  asc Creation.
//  2013-01-09  asc Moved state mutex and suspend semaphore into main class.
//  2013-02-15  asc Moved priority and stack size into common implementation.
// ----------------------------------------------------------------------------

#ifndef CP_THREAD_I_H
#define CP_THREAD_I_H

namespace cp
{

struct Thread_t
{
    HANDLE              m_Handle;                           // thread handle
    uint32_t            m_ThreadId;                         // thread ID
};

}   // namespace cp

#endif  // CP_THREAD_I_H
