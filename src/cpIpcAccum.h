// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcAccum.h
//
//  Description:    IPC segment accumulator.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
//  2013-03-22  asc Added support for accumulator timeout handling.
//  2013-03-25  asc Added Count() and Head() accessor methods.
// ----------------------------------------------------------------------------

#ifndef CP_IPCACCUM_H
#define CP_IPCACCUM_H

#include "cpPlatform.h"

namespace cp
{

// forward references
class IpcSegment;

// ----------------------------------------------------------------------------

class IpcAccum
{
public:
    // constructor
    IpcAccum();

    // copy constructor
    IpcAccum(IpcAccum const &rhs);

    // destructor
    ~IpcAccum();

    // operators
    IpcAccum &operator=(IpcAccum const &rhs);

    // accessors
    void SubmitSegment(IpcSegment *pSegment);               // submit a segment for accumulation
    bool MessageGet(IpcSegment *&pSegment);                 // get the received set of segments
    bool Complete();                                        // return true if all message segments received
    bool Expired();                                         // returns true if this accumulator has expired
    void ResetTimeout(uint32_t MilliSecs);                  // resets the accumulation timeout
    uint32_t Count() const { return m_Received; }           // return number of segments received
    IpcSegment *Head() const { return m_PtrHead; }          // return a pointer to the head segment

private:
    uint64_t            m_Timeout;                          // accumulation expiration time (milliseconds)
    uint32_t            m_Total;                            // total number of segments in this message
    uint32_t            m_Received;                         // number of segments received
    IpcSegment         *m_PtrHead;                          // pointer to head of segment list
};

}   // namespace cp

#endif  // CP_IPCACCUM_H
