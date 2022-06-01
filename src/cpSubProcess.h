// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2022 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSubProcess.h
//
//  Description:    Creates a subprocess with a pipe to parent.
//
//  Platform:       common
//
//  History:
//  2022-02-02  asc Creation.
//  2022-02-04  asc Added Cancel method.
//  2022-03-02  asc Added StreamBufTransfer() method.
// ----------------------------------------------------------------------------

#ifndef CP_SUBPROCESS_H
#define CP_SUBPROCESS_H

#include "cpThread.h"
#include "cpStreamBuf.h"
#include "cpSubProcess_I.h"

namespace cp
{

// forward declarations

// local custom types
enum SubProcIoDirection { k_FlowIn, k_FlowOut };

// ----------------------------------------------------------------------------

// the subprocess class
class SubProcess : public Base
{
public:
    // constructor
    SubProcess(cp::String const &Command, SubProcIoDirection dir = k_FlowOut);

    // destructor
    virtual ~SubProcess();

    // public methods
    void Cancel();                                          // cancel execution
    bool IsRunning();                                       // check if subprocess currently running
    void WaitUntilDone();                                   // returns when subprocess completes
    void BufferExtract(Buffer &Buf);                        // copy out the stream buffer
    void StreamBufTransfer(StreamBuf &Dest);                // transfer out the stream buffer

private:
    void IoThread();                                        // I/O thread function
    static void *ThreadFunction(Thread *pThread);           // static thread trampoline function

    SubProcIoDirection  m_Dir;                              // direction of I/O with parent process
    SubProcessHandle_t *m_PtrHandle;                        // native storage for read/write handle
    SubProcessDesc_t    m_Descriptor;                       // descriptor for read
    Thread              m_IoThread;                         // input / output thread
    StreamBuf           m_RxBuffer;                         // subprocess receive buffer
    SemLite             m_Completed;                        // semaphore to signal end of operation
    Mutex               m_SyncIo;                           // mutex to synchronize read/write I/O
};

}   // namespace cp

#endif // CP_SUBPROCESS_H