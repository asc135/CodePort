// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSemLite.h
//
//  Description:    Semaphore facility using lightweight implementation.
//
//  Platform:       common
//
//  History:
//  2012-08-03  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_SEMLITE_H
#define CP_SEMLITE_H

#include "cpBase.h"
#include "cpSemLite_I.h"

namespace cp
{

class SemLite : public Base
{
public:
    SemLite(String const &Name = "Anonymous Semaphore",
            uint32_t InitCount = 1,
            uint32_t MaxCount = (uint32_t)(~0));

    virtual ~SemLite();

    bool Take(uint32_t Timeout = k_InfiniteTimeout);
    bool TryTake();
    bool Give();
    bool GiveAll();

    uint32_t CountGet() const;
    uint32_t MaxCountGet() const;

private:
    SemLite_t           m_Semaphore;                        // native data storage
    uint32_t            m_MaxCount;                         // maximum allowed semaphore count
};

}   // namespace cp

#endif  // CP_SEMLITE_H
