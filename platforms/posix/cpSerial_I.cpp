// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpSerial_I.cpp
//
//  Description:    Serial Communications Facility.
//
//  Platform:       posix
//
//  History:
//  2011-04-30  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2012-12-11  asc Added timeout parameter to SendData() and recvData().
//  2013-12-18  asc Updated to new output parameter type for Tokenize().
// ----------------------------------------------------------------------------

#include <sys/time.h>
#include <fcntl.h>
#include <cstdlib>

#include "cpUtil.h"
#include "cpSerial.h"

namespace cp
{

// constructor
Serial::Serial(String const &Name, String const &DevicePath, String const &Params) :
    IoDev(Name)
{
    m_Device.Device = DevicePath;
    m_Device.Wait = true;
    ParamSet(Params);
    m_Valid = OpenPort();

    // save the existing port mode for restoring at object destruction
    ModeSave();

    // set the line parameters
    ParamSet(Params);
}


// destructor
Serial::~Serial()
{
    ModeRest();
}


// ----------------------------------------------------------------------------
//  Function Name:  SendData
//
//  Description:    sends data to the serial port
//
//  Inputs:         pBuf - pointer to target buffer
//                  SndLen - number of bytes to send
//                  BytesWritten - number of bytes previously written
//                  Timeout - I/O timeout
//
//  Outputs:        none
//
//  Returns:        number of characters sent or < 0 if error
// ----------------------------------------------------------------------------
int Serial::SendData(char const *pBuf, size_t SndLen, size_t BytesWritten, uint32_t Timeout)
{
    (void)Timeout;
    return ::write(m_dWrite, pBuf + BytesWritten, SndLen - BytesWritten);
}


// ----------------------------------------------------------------------------
//  Function Name:  RecvData
//
//  Description:    receive data from the serial port
//
//  Inputs:         pBuf - pointer to target buffer
//                  RcvLen - number of bytes to read
//                  BytesRead - number of bytes previously read
//                  Timeout - I/O timeout
//
//  Outputs:        none
//
//  Returns:        number of characters read or < 0 if error
// ----------------------------------------------------------------------------
int Serial::RecvData(char *pBuf, size_t RcvLen, size_t BytesRead, uint32_t Timeout)
{
    (void)Timeout;
    return ::read(m_dRead, pBuf + BytesRead, RcvLen - BytesRead);
}


// flush device I/O buffers
void Serial::Flush()
{
    // (.)(.) need to implement serial buffer Flush()
}


// cancel pended I/O operations
void Serial::Cancel()
{
    m_Device.Wait = false;
    sleep(3);
    m_Device.Wait = true;
}


// set the port communications parameters
void Serial::ParamSet()
{
    termios settings;
    uint32_t DataRate;
    uint32_t WordSize;
    uint32_t StopBits;

    // set non-blocking mode (redundant with O_NDELAY in the open() call but needed for portability
    fcntl(m_Device.Desc, F_SETFL, FNDELAY);

    // get the existing port settings
    tcgetattr(m_Device.Desc, &settings);

    // set the data rate
    switch (m_Device.DataRate)
    {
    case 115200:
        DataRate = B115200;
        break;

    case 57600:
        DataRate = B57600;
        break;

    case 38400:
        DataRate = B38400;
        break;

    case 19200:
        DataRate = B19200;
        break;

    case 9600:
        DataRate = B9600;
        break;

    case 4800:
        DataRate = B4800;
        break;

    case 2400:
        DataRate = B2400;
        break;

    case 1200:
        DataRate = B1200;
        break;

    case 600:
        DataRate = B600;
        break;

    case 300:
        DataRate = B300;
        break;

    case 200:
        DataRate = B200;
        break;

    case 150:
        DataRate = B150;
        break;

    case 134:
        DataRate = B134;
        break;

    case 110:
        DataRate = B110;
        break;

    case 75:
        DataRate = B75;
        break;

    case 50:
        DataRate = B50;
        break;

    default:
        DataRate = B9600;
    }

    cfsetispeed(&settings, DataRate);
    cfsetospeed(&settings, DataRate);

    // calculate the retry interval in microseconds
    // it is the transfer time of ten characters
    m_Device.Interval = static_cast<uint32_t>(1.0e+8 / m_Device.DataRate);

    // enable the receiver and set local mode
    settings.c_cflag |= (CLOCAL | CREAD);

    // set the word size
    switch (m_Device.WordSize)
    {
    case 5:
        WordSize = CS5;
        break;

    case 6:
        WordSize = CS6;
        break;

    case 7:
        WordSize = CS7;
        break;

    case 8:
        WordSize = CS8;
        break;

    default:
        WordSize = CS8;
    }

    settings.c_cflag &= ~CSIZE;
    settings.c_cflag |= WordSize;

    // clear the parity flags
    settings.c_cflag &= ~(PARENB | PARODD);
    settings.c_iflag &= ~(ISTRIP | INPCK);

    // set the parity
    switch (m_Device.Parity)
    {
    case 0:         // no parity
        break;

    case 1:         // odd parity
        settings.c_cflag |= (PARENB | PARODD);
        settings.c_iflag |= (ISTRIP | INPCK);
        break;

    case 2:         // even parity
        settings.c_cflag |= PARENB;
        settings.c_iflag |= (ISTRIP | INPCK);
        break;

    default:        // no parity
        break;
    }

    // set the stop bits
    switch (m_Device.StopBits)
    {
    case 1:         // 1 stop bit
        StopBits = 0;
        break;

    case 2:         // 2 stop bits
        StopBits = CSTOPB;
        break;

    default:        // 1 stop bit
        StopBits = 0;
    }

    settings.c_cflag &= ~CSTOPB;
    settings.c_cflag |= StopBits;

    // set raw mode
    settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    settings.c_iflag &= ~(INLCR | ICRNL | IUCLC);
    settings.c_oflag &= ~OPOST;

    // clear the handshaking mode
    settings.c_iflag &= ~(IXON | IXOFF | IXANY);
    settings.c_cflag &= ~CRTSCTS;

    // set the handshaking
    switch (m_Device.FlowCtrl)
    {
    case 0:     // no flow control
        break;

    case 1:     // xon_xoff
        settings.c_iflag |= (IXON | IXOFF | IXANY);
        settings.c_cflag &= ~CRTSCTS;
        break;

    case 2:     // rts_cts
        settings.c_iflag &= ~(IXON | IXOFF | IXANY);
        settings.c_cflag |= CRTSCTS;
        break;

    default:    // no flow control
        break;
    }

    // flush the buffers and apply new attributes
    tcsetattr(m_Device.Desc, TCSAFLUSH, &settings);
}


// set the port communications parameters from a string
bool Serial::ParamSet(String const &Params)
{
    bool rv = false;
    StringVec_t configs;
    StringVec_t::iterator i;
    char c;

    cp::Tokenize(Params, ",", configs);

    // should be five items:  data rate, parity, word length, stop bits, flow
    if (configs.size() == 5)
    {
        i = configs.begin();

        // set data rate
        m_Device.DataRate = strtoul(i->c_str(), NULL, 10);

        // set parity
        ++i;

        if (i->size() > 0)
        {
            c = (*i)[0];
        }
        else
        {
            c = 0;
        }

        switch (c)
        {
        case 'n':
        case 'N':
            m_Device.Parity = 0;
            break;

        case 'o':
        case 'O':
            m_Device.Parity = 1;
            break;

        case 'e':
        case 'E':
            m_Device.Parity = 2;
            break;

        default:
            m_Device.Parity = 0;
        }

        // set word length
        ++i;
        m_Device.WordSize = strtoul(i->c_str(), NULL, 10);

        // set stop bits
        ++i;
        m_Device.StopBits = strtoul(i->c_str(), NULL, 10);

        // set flow control
        ++i;

        if (i->size() > 0)
        {
            c = (*i)[0];
        }
        else
        {
            c = 0;
        }

        switch (c)
        {
        case 'n':   // no flow control
        case 'N':
            m_Device.FlowCtrl = 0;
            break;

        case 'x':   // xon_xoff software flow control
        case 'X':
            m_Device.FlowCtrl = 1;
            break;

        case 'r':   // rts_cts hardware flow control
        case 'R':
            m_Device.FlowCtrl = 2;
            break;

        default:
            m_Device.FlowCtrl = 0;
        }

        rv = true;
    }

    // apply the parsed parameters to the port
    if (rv)
    {
        ParamSet();
    }

    return rv;
}


// get the number of characters waiting to be read
bool Serial::GetCharsWaiting(uint32_t &Count)
{
    bool rv = false;
    Count = 0;
    return rv;
}


// get the state of the CTS signal line
bool Serial::GetCtsState(bool &State)
{
    bool rv = false;
    State = false;
    return rv;
}


// set the state of the DTR signal line
bool Serial::SetDtrState(bool State)
{
    bool rv = false;
    (void)State;
    return rv;
}


// set the state of the RTS signal line
bool Serial::SetRtsState(bool State)
{
    bool rv = false;
    (void)State;
    return rv;
}


// open the device for I/O
bool Serial::OpenPort()
{
    m_dWrite = open(m_Device.Device.c_str(), O_WRONLY | O_NOCTTY | O_NDELAY);

    if (m_dWrite == k_InvalidDescriptor)
    {
        return false;
    }

    m_dRead  = open(m_Device.Device.c_str(), O_RDONLY | O_NOCTTY | O_NDELAY);

    if (m_dRead == k_InvalidDescriptor)
    {
        close(m_dWrite);
        m_dWrite = k_InvalidDescriptor;
        return false;
    }

    return true;
}


// close the device
bool Serial::ClosePort()
{
    bool rv = true;

    if (m_dWrite != k_InvalidDescriptor)
    {
        rv = rv && (close(m_dWrite) != k_Error);
        m_dWrite = k_InvalidDescriptor;
    }

    if (m_dRead != k_InvalidDescriptor)
    {
        rv = rv && (close(m_dRead) != k_Error);
        m_dRead = k_InvalidDescriptor;
    }

    return rv;
}


// set I/O to character mode
void Serial::ModeChar()
{
    termios mode;

    // disable echo and turn off line mode
    tcgetattr(m_Device.Desc, &mode);
    mode.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(m_Device.Desc, TCSANOW, &mode);
}


// set I/O to line edit mode
void Serial::ModeLine()
{
    termios mode;

    // enable echo and turn on line mode
    tcgetattr(m_Device.Desc, &mode);
    mode.c_lflag |= (ECHO | ICANON);
    tcsetattr(m_Device.Desc, TCSANOW, &mode);
}


// save current I/O mode
void Serial::ModeSave()
{
    // store current term I/O settings
    tcgetattr(m_Device.Desc, &m_Device.Original);
}


// set I/O to saved mode
void Serial::ModeRest()
{
    // restore term I/O settings to saved values
    tcsetattr(m_Device.Desc, TCSANOW, &m_Device.Original);
}

}   // namespace cp
