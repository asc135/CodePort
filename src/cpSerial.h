// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSerial.h
//
//  Description:    Serial Communications Facility.
//
//  Platform:       common
//
//  History:
//  2011-04-30  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Added timeout parameter to SendData() and recvData().
// ----------------------------------------------------------------------------

#ifndef CP_SERIAL_H
#define CP_SERIAL_H

#include "cpIoDev.h"
#include "cpSerial_I.h"

namespace cp
{

class Serial : public IoDev
{
public:
    // constructor
    Serial(String const &Name, String const &DevicePath, String const &Params);

    // destructor
    virtual ~Serial();

    // manipulators
    virtual void Flush();                                   // flush device I/O buffers
    virtual void Cancel();                                  // cancel pended I/O operations

    // device control
    void ParamSet();                                        // set the port communications parameters
    bool ParamSet(String const &Params);                    // set the port communications parameters from a string
    bool GetCharsWaiting(uint32_t &Count);                  // get the number of characters waiting to be read
    bool GetCtsState(bool &State);                          // get the state of the CTS signal line
    bool SetDtrState(bool State);                           // set the state of the DTR signal line
    bool SetRtsState(bool State);                           // set the state of the RTS signal line
    bool OpenPort();                                        // open the device for I/O
    bool ClosePort();                                       // close the device
    void ModeChar();                                        // set I/O to character mode
    void ModeLine();                                        // set I/O to line edit mode
    void ModeSave();                                        // save current I/O mode
    void ModeRest();                                        // set I/O to saved mode

protected:
    virtual int SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout);
    virtual int RecvData(char       *pBuf, size_t RcvLen, size_t BytesRead,    uint32_t Timeout);

private:
    Serial_t            m_Device;                           // native object data storage
};

}   // namespace cp

#endif  // CP_SERIAL_H
