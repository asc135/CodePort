// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpIpcAccum.cpp
//
//  Description:    IPC segment accumulator.
//
//  Platform:       common
//
//  History:
//  2012-09-28  asc Creation.
//  2013-03-22  asc Added support for accumulator timeout handling.
// ----------------------------------------------------------------------------

#include "cpIpcAccum.h"
#include "cpIpcSegment.h"
#include "cpUtil.h"

namespace cp
{

// constructor
IpcAccum::IpcAccum() :
    m_Timeout(0),
    m_Total(0),
    m_Received(0),
    m_PtrHead(NULL)
{
    ResetTimeout(k_IpcAccumulatorTimeout);
}


// copy constructor
IpcAccum::IpcAccum(IpcAccum const &rhs) :
    m_Timeout(0),
    m_Total(0),
    m_Received(0),
    m_PtrHead(NULL)
{
    // invoke assignment operator
    *this = rhs;
}


// destructor
IpcAccum::~IpcAccum()
{
    // purge any message segments left in list
    if (m_PtrHead)
    {
        delete m_PtrHead;
    }
}


// assignment operator
IpcAccum &IpcAccum::operator=(IpcAccum const &rhs)
{
    // check for self-assignment
    if (this != &rhs)
    {
        m_Timeout  = rhs.m_Timeout;
        m_Total    = rhs.m_Total;
        m_Received = rhs.m_Received;
        m_PtrHead  = rhs.m_PtrHead;
    }

    return *this;
}


// submit a segment for accumulation
void IpcAccum::SubmitSegment(IpcSegment *pSegment)
{
    // check for invalid pointer
    if (pSegment == NULL)
    {
        return;
    }

    // increment segment counter
    ++m_Received;

    // check if this is the highest numbered segment
    uint8_t options = pSegment->Options();

    if ((options & IpcSegment::opt_Multipart) && (options & IpcSegment::opt_Initial))
    {
        m_Total = pSegment->FragNum();
    }

    // add segment to the list
    if (m_PtrHead == NULL)
    {
        // if list is empty add it as the head
        m_PtrHead = pSegment;
    }
    else
    {
        IpcSegment *pPrev = NULL;
        IpcSegment *pCurr = m_PtrHead;

        // search for insertion point
        while (pCurr != NULL)
        {
            // insert segment immediately before a higher fragment number
            if (pSegment->FragNum() < pCurr->FragNum())
            {
                // point segment to current element
                pSegment->NextSet(pCurr);

                if (pPrev == NULL)
                {
                    // if no previous element, make this segment the list head
                    m_PtrHead = pSegment;
                }
                else
                {
                    // otherwise, point previous element to segment
                    pPrev->NextSet(pSegment);
                }

                // indicate that the segment was inserted
                pSegment = NULL;

                // exit the loop
                break;
            }

            // adjust pointers to next entry
            pPrev = pCurr;
            pCurr = pCurr->NextGet();
        }

        // insert segment at end if it wasn't
        // inserted somewhere in the list
        if (pSegment != NULL)
        {
            // check if list is empty
            if (pPrev != NULL)
            {
                // add new segment to end of list
                pPrev->NextSet(pSegment);
            }
            else
            {
                // list is empty so insert it as the head
                m_PtrHead = pSegment;
            }
        }
    }
}


// get the received set of segments
bool IpcAccum::MessageGet(IpcSegment *&pSegment)
{
    bool rv = (m_PtrHead != NULL);

    if (rv)
    {
        pSegment = m_PtrHead;
        m_PtrHead = NULL;
    }

    return rv;
}


// return true if all message segments received
bool IpcAccum::Complete()
{
    bool rv = false;

    // check if entire message has been received
    rv = ((m_PtrHead != NULL) && (m_Total > 0) && (m_Total == m_Received));

    return rv;
}


// returns true if this accumulator has expired
bool IpcAccum::Expired()
{
    return (m_Timeout <= Time64());
}


// resets the accumulation timeout
void IpcAccum::ResetTimeout(uint32_t MilliSecs)
{
    m_Timeout = Time64() + MilliSecs;
}

}   // namespace cp
