// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSerial_I.h
//
//  Description:    Serial Communications Facility.
//
//  Platform:       posix
//
//  History:
//  2011-04-30  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#ifndef CP_SERIAL_I_H
#define CP_SERIAL_I_H

#include <termios.h>

namespace cp
{

struct Serial_t
{
    String          Device;                                 // path to device file "/dev/ttySn"
    String          Params;                                 // current device parameter string
    desc_t          Desc;                                   // read/write descriptor
    uint32_t        DataRate;                               // numeric data rate
    uint16_t        Parity;                                 // enumeration of parity mode
    uint16_t        WordSize;                               // word size in bits
    uint16_t        StopBits;                               // number of stop bits
    uint16_t        FlowCtrl;                               // enumeration of flow control type
    termios         Original;                               // original term I/O settings
    uint32_t        Interval;                               // retry interval in microseconds
    volatile bool   Wait;                                   // controls read blocking
};

}   // namespace cp

#endif  // CP_SERIAL_I_H
