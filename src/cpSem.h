// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSem.h
//
//  Description:    Semaphore facility using native implementation.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Added GiveAll() method.
// ----------------------------------------------------------------------------

#ifndef CP_SEM_H
#define CP_SEM_H

#include "cpBase.h"
#include "cpSem_I.h"

namespace cp
{

class Sem : public Base
{
public:
    Sem(String const &Name = "Anonymous Semaphore",
        uint32_t InitCount = 1,
        uint32_t MaxCount = (uint32_t)(~0));

    virtual ~Sem();

    bool Take(uint32_t Timeout = k_InfiniteTimeout);
    bool TryTake();
    bool Give();
    bool GiveAll();

    uint32_t CountGet();
    uint32_t MaxCountGet() const;

private:
    Sem_t               m_Semaphore;                        // native data storage
    uint32_t            m_MaxCount;                         // maximum allowed semaphore count
};

}   // namespace cp

#endif  // CP_SEM_H
