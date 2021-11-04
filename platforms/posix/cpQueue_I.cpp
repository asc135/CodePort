// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpQueue_I.cpp
//
//  Description:    Inter-Process Message Queue Facility.
//
//  Platform:       posix
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Added timeout parameter to SendData() and recvData().
//  2014-12-03  asc Replaced readiness polling with timed send/receive.
// ----------------------------------------------------------------------------

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <ctime>
#include <algorithm>

#include "cpQueue.h"
#include "cpUtil.h"
#include "cpBuffer.h"

namespace cp
{

// transform invalid characters to legal ones
static void LegalChars(char &c)
{
    if (c == ' ')
    {
        c = '_';
    }
}

// ----------------------------------------------------------------------------

// constructor, owner
Queue::Queue(String const &Name, size_t MsgSize, uint32_t MaxMsgs) :
    IoDev(Name),
    m_Cleanup(true),
    m_MsgSize(MsgSize),
    m_MaxMsgs(MaxMsgs)
{
    mq_attr ma;

    // queue name is of the form /name
    m_SysName = "/";
    m_SysName += Name;

    // replace any spaces in name with underscores
    std::for_each(m_SysName.begin(), m_SysName.end(), LegalChars);

    // specify message queue attributes.  Queue is created as blocking since the
    // Queue API has mechanisms for non-blocking get/put operations
    ma.mq_flags = 0;                // blocking read/write
    ma.mq_maxmsg = m_MaxMsgs;       // maximum number of messages allowed in queue
    ma.mq_msgsize = m_MsgSize;      // messages are discrete size
    ma.mq_curmsgs = 0;              // number of messages currently in queue

    // create the message queue with some default settings
    m_MsgQueue = mq_open(m_SysName.c_str(), O_RDWR | O_CREAT, 0700, &ma);

    // -1 indicates an error
    if (m_MsgQueue == k_Error)
    {
        LogErr << "Queue::Queue(): Message queue could not be created: "
               << NameGet() << std::endl;
    }
    else
    {
        m_Valid = true;
    }
}


// constructor, user
Queue::Queue(String const &Name) :
    IoDev(Name),
    m_Cleanup(false),
    m_MsgSize(0),
    m_MaxMsgs(0)
{
    mq_attr ma;

    // queue name is of the form /name
    m_SysName = "/";
    m_SysName += Name;

    // replace any spaces in name with underscores
    std::for_each(m_SysName.begin(), m_SysName.end(), LegalChars);

    // open the message queue with some default settings
    m_MsgQueue = mq_open(m_SysName.c_str(), O_RDWR);

    // -1 indicates an error
    if (m_MsgQueue == k_Error)
    {
        LogErr << "Queue::Queue(): Message queue could not be opened: "
               << NameGet() << std::endl;
    }
    else
    {
        // get the message queue attributes
        if (mq_getattr(m_MsgQueue, &ma) == 0)
        {
            m_MaxMsgs = ma.mq_maxmsg;
            m_MsgSize = ma.mq_msgsize;
            m_Valid = true;
        }
        else
        {
            LogErr << "Queue::Queue(): Message queue could not read attributes: "
                   << NameGet() << std::endl;
        }
    }
}


// destructor
Queue::~Queue()
{
    // close the message queue if it had been opened
    if (m_Valid)
    {
        if (mq_close(m_MsgQueue) == k_Error)
        {
            LogErr << "Queue::~Queue(): Error closing message queue: "
                   << NameGet() << std::endl;
        }

        // delete the queue device if this instance is the owner
        if (m_Cleanup)
        {
            if (mq_unlink(m_SysName.c_str()) == k_Error)
            {
                LogErr << "Queue::~Queue(): Error deleting message queue: "
                       << NameGet() << std::endl;
            }
        }
    }
}


// ----------------------------------------------------------------------------
//  Function Name:  SendReady
//
//  Description:    test if queue is ready to accept data
//
//  Inputs:         Timeout - time to wait in milliseconds for ready state
//
//  Outputs:        none
//
//  Returns:        true if ready, false if not ready
// ----------------------------------------------------------------------------
bool Queue::SendReady(uint32_t Timeout)
{
    (void)Timeout;
    return true;
}


// ----------------------------------------------------------------------------
//  Function Name:  RecvReady
//
//  Description:    test if queue has data ready to read
//
//  Inputs:         Timeout - time to wait in milliseconds for ready state
//
//  Outputs:        none
//
//  Returns:        true if ready, false if not ready
// ----------------------------------------------------------------------------
bool Queue::RecvReady(uint32_t Timeout)
{
    (void)Timeout;
    return true;
}


// ----------------------------------------------------------------------------
//  Function Name:  SendData
//
//  Description:    send data to the queue
//
//  Inputs:         pBuf - pointer to source buffer
//                  SndLen - number of bytes to send
//                  BytesWritten - number of bytes previously written
//                  Timeout - I/O timeout
//
//  Outputs:        none
//
//  Returns:        number of characters written or < 0 if error
// ----------------------------------------------------------------------------
int Queue::SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout)
{
    int rv = k_Error;
    timespec absTimeout;

    // get the current time and add the timeout
    clock_gettime(CLOCK_REALTIME, &absTimeout);
    absTimeout.tv_nsec += (Timeout * 1000000);

    // adjust for nanosecond overflow
    if (absTimeout.tv_nsec > 999999999)
    {
        absTimeout.tv_nsec -= 1000000000;
        absTimeout.tv_sec += 1;
    }

    // send the data
    if (mq_timedsend(m_MsgQueue, pBuf + BytesWritten, SndLen - BytesWritten, 1, &absTimeout) == 0)
    {
        rv = (SndLen - BytesWritten);
    }

    return rv;
}


// ----------------------------------------------------------------------------
//  Function Name:  RecvData
//
//  Description:    receive data from the queue
//
//  Inputs:         pBuf - pointer to target buffer
//                  RcvLen - number of bytes to receive
//                  BytesRead - number of bytes previously read
//                  Timeout - I/O timeout
//
//  Outputs:        none
//
//  Returns:        number of characters read or < 0 if error
// ----------------------------------------------------------------------------
int Queue::RecvData(char *pBuf, size_t RcvLen, size_t BytesRead, uint32_t Timeout)
{
    timespec absTimeout;

    // get the current time and add the timeout
    clock_gettime(CLOCK_REALTIME, &absTimeout);
    absTimeout.tv_nsec += (Timeout * 1000000);

    // adjust for nanosecond overflow
    if (absTimeout.tv_nsec > 999999999)
    {
        absTimeout.tv_nsec -= 1000000000;
        absTimeout.tv_sec += 1;
    }

    // receive the data
    return mq_timedreceive(m_MsgQueue, pBuf + BytesRead, RcvLen - BytesRead, NULL, &absTimeout);
}


// ----------------------------------------------------------------------------
//  Function Name:  Flush
//
//  Description:    force the queue to empty
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        none
// ----------------------------------------------------------------------------
void Queue::Flush()
{
    Buffer buf(m_MsgSize);

    // abort if object not properly initialized
    if (!m_Valid)
    {
        return;
    }

    // pull message off queue while messages are available.  Don't use "wait"
    // mode because you don't want to block if the queue is empty
    while (!IsEmpty())
    {
        Recv(buf, buf);
    }
}


// ----------------------------------------------------------------------------
//  Function Name:  IsEmpty
//
//  Description:    tests if the queue is empty
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        true if the queue is empty
// ----------------------------------------------------------------------------
bool Queue::IsEmpty() const
{
    return (NumUsed() == 0);
}


// ----------------------------------------------------------------------------
//  Function Name:  IsFull
//
//  Description:    tests if the queue is full
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        true if the queue is full
// ----------------------------------------------------------------------------
bool Queue::IsFull() const
{
    return (NumUsed() == m_MaxMsgs);
}


// ----------------------------------------------------------------------------
//  Function Name:  NumFree
//
//  Description:    retrieve number of free positions in queue
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        number of free positions in the queue
// ----------------------------------------------------------------------------
uint32_t Queue::NumFree() const
{
    uint32_t rv = 0;
    mq_attr ma;

    // abort if object not properly initialized
    if (!m_Valid)
    {
        return 0;
    }

    // get the current queue state
    // get number of messages in queue and then calculate free space from known max size
    if (mq_getattr(m_MsgQueue, &ma) == 0)
    {
        rv = (m_MaxMsgs - ma.mq_curmsgs);
    }

    return rv;
}


// ----------------------------------------------------------------------------
//  Function Name:  NumUsed
//
//  Description:    retrieve number of used positions in queue
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        number of used positions in the queue
// ----------------------------------------------------------------------------
uint32_t Queue::NumUsed() const
{
    uint32_t rv = 0;
    mq_attr ma;

    // abort if object not properly initialized
    if (!m_Valid)
    {
        return 0;
    }

    // get the current queue state
    // get number of messages in queue
    if (mq_getattr(m_MsgQueue, &ma) == 0)
    {
        rv = ma.mq_curmsgs;
    }

    return rv;
}


// ----------------------------------------------------------------------------
//  Function Name:  MaxMsgsGet
//
//  Description:    retrieve maximum number of messages possible in queue
//
//  Inputs:         none
//
//  Outputs:        none
//
//  Returns:        maximum number of positions in queue
// ----------------------------------------------------------------------------
uint32_t Queue::MaxMsgsGet() const
{
    return m_MaxMsgs;
}

}   // namespace cp
