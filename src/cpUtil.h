// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpUtil.h
//
//  Description:    Utility Function Library.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-02-04  asc Changed network/host functions to upper camel case.
//  2012-03-16  asc Added Ipv4ToStr() and StrToIpv4() functions.
//  2013-06-13  asc Added StartProcess() function.
//  2013-11-15  asc Added cascade capability to CRC functions.
//  2013-12-18  asc Added DirCreate() and moved PathCreate() into common module.
//  2013-12-18  asc Changed output parameter of Tokenize() to StringVec_t.
//  2013-12-18  asc Added Lchomp() and Rchomp() functions.
//  2014-03-30  asc Added ReadFile() and WriteFile() functions.
//  2022-05-22  asc Added HostName() and DomainName() functions.
//  2022-05-23  asc Added Int64ToStr() and Uint64ToStr() functions.
//  2022-05-26  asc Made Input buffer to HexEncode a const.
//  2022-06-09  asc Added CpuTime64() function.
//  2022-06-10  asc Added RunProgramGetOutput(), and BufferToLines() functions.
//  2022-09-09  asc Added CheckAlphaNumeric() function.
//  2023-03-29  asc Added StrToInt64() and StrToUint64() functions.
//  2023-04-03  asc Added UpperCase() and LowerCase() functions.
//  2023-04-04  asc Added several more functions and reorganized into common and platform.
// ----------------------------------------------------------------------------

#ifndef CP_UTIL_H
#define CP_UTIL_H

#include "cpString.h"
#include "cpUtil_I.h"

namespace cp
{

class Buffer;

// ----------------------------------------------------------------------------

// used by StartProcess()
typedef std::vector<char const *, Alloc<char const *> > CStringVec_t;

// ----------------------------------------------------------------------------

// format structure used by HexIo
class HexIoCfg
{
public:
    // constructor
    HexIoCfg() :
        groupLenMax(1),
        groupLen(0),
        lineLenMax(0),
        lineLen(0),
        preLine(false),
        postLine(false),
        preserve(false),
        finalPass(true),
        separator(" ")
    {
    }

    // diagnostic display
    void Display();

    size_t groupLenMax;     // maximum number of input octets per output group (default = 1)
    size_t groupLen;        // number of input octets in current group
    size_t lineLenMax;      // maximum output line length in characters (default = 0 / unlimited)
    size_t lineLen;         // number of output characters in current line
    bool preLine;           // true to insert a separator at the beginning of each line
    bool postLine;          // true to insert a separator at the end of each line
    bool preserve;          // preserve values for next call to encode
    bool finalPass;         // true to inject closing formatting tokens after last data item
    String prefix;          // group prefix string
    String suffix;          // group suffix string
    String separator;       // group separator string
};

// ----------------------------------------------------------------------------

// ASCII standard control codes
enum AsciiControlCodes
{
    CHAR_NUL  = 0x00,
    CHAR_SOH  = 0x01,
    CHAR_STX  = 0x02,
    CHAR_ETX  = 0x03,
    CHAR_EOT  = 0x04,
    CHAR_ENQ  = 0x05,
    CHAR_ACK  = 0x06,
    CHAR_BEL  = 0x07,
    CHAR_BS   = 0x08,
    CHAR_HTAB = 0x09,
    CHAR_LF   = 0x0a,
    CHAR_VTAB = 0x0b,
    CHAR_FF   = 0x0c,
    CHAR_CR   = 0x0d,
    CHAR_SO   = 0x0e,
    CHAR_SI   = 0x0f,
    CHAR_DLE  = 0x10,
    CHAR_DC1  = 0x11,
    CHAR_XON  = 0x11,
    CHAR_DC2  = 0x12,
    CHAR_DC3  = 0x13,
    CHAR_XOFF = 0x13,
    CHAR_DC4  = 0x14,
    CHAR_NAK  = 0x15,
    CHAR_SYN  = 0x16,
    CHAR_ETB  = 0x17,
    CHAR_CAN  = 0x18,
    CHAR_EM   = 0x19,
    CHAR_SUB  = 0x1a,
    CHAR_ESC  = 0x1b,
    CHAR_FS   = 0x1c,
    CHAR_GS   = 0x1d,
    CHAR_RS   = 0x1e,
    CHAR_US   = 0x1f,
    CHAR_DEL  = 0x7f
};

// ----------------------------------------------------------------------------
// platform independent (common) functions
// ----------------------------------------------------------------------------

// create a path including intermediate directories
bool PathCreate(String const &PathName);

// dump a block of data to a stream in Hex ASCII format
bool HexDump(std::ostream &Out, Buffer const &Buf, size_t LineLen = 16);
bool HexDump(std::ostream &Out, char const *Data, size_t DataLen, size_t LineLen = 16);
bool HexDump(std::ostream &Out, uint8_t const *Data, size_t DataLen, size_t LineLen = 16);

// Encode a block of data to ASCII hex
size_t HexEncode(Buffer const &Input, String &Output, HexIoCfg &Form);

// decode a block of ASCII hex data
size_t HexDecode(String const &Input, Buffer &Output);

// returns a string with the specified number of characters
String const GenStr(size_t Count, char Ch = ' ');

// trims leading whitespace from a string
String &Ltrim(String &Str);

// trims trailing whitespace from a string
String &Rtrim(String &Str);

// chomps characters from the beginning of a string
String &Lchomp(String &Str, size_t Count = 1);

// chomps characters from the end of a string
String &Rchomp(String &Str, size_t Count = 1);

// converts string to long integer
int32_t StrToInt(String const &Str);

// converts string to unsigned long integer
uint32_t StrToUint(String const &Str);

// converts string to long long integer
int64_t StrToInt64(String const &Str);

// converts string to unsigned long long integer
uint64_t StrToUint64(String const &Str);

// converts string to float
double StrToFloat(String const &Str);

// converts string to boolean
bool StrToBool(String const &Str);

// converts integer to string
String IntToStr(int Val);

// converts unsigned to string integer
String UintToStr(uint32_t Val);

// converts integer to string
String Int64ToStr(int64_t Val);

// converts unsigned to string integer
String Uint64ToStr(uint64_t Val);

// converts float to string
String FloatToStr(double Val);

// converts boolean to string
String BoolToStr(bool Val);

// tokenize a string into a vector of strings
size_t Tokenize(String const &StringIn, String const &Delim, StringVec_t &TokensOut);

// break a text buffer into a vector of lines
void BufferToLines(char const *pBufferIn, size_t BufferSize, StringVec_t &LinesOut);

// reverse the bits in an octet
uint8_t Reflect8(uint8_t Val);

// reverse the bits in a short word
uint16_t Reflect16(uint16_t Val);

// reverse the bits in a long word
uint32_t Reflect32(uint32_t Val);

// calculate CRC-16 for a block of data
uint16_t CalcCrc16(Buffer &Buf, uint16_t Cascade = 0xffff);
uint16_t CalcCrc16(char const *pBuf, size_t Size, uint16_t Cascade = 0xffff);
uint16_t CalcCrc16(uint8_t const *pBuf, size_t Size, uint16_t Cascade = 0xffff);

// calculate CRC-32 for a block of data
uint32_t CalcCrc32(Buffer &Buf, uint32_t Cascade = 0xffffffff);
uint32_t CalcCrc32(char const *pBuf, size_t Size, uint32_t Cascade = 0xffffffff);
uint32_t CalcCrc32(uint8_t const *pBuf, size_t Size, uint32_t Cascade = 0xffffffff);

// detect host byte order
bool HostBigEndian();

// detect host byte order
bool HostLittleEndian();

// swap bytes in a short word
uint16_t SwapBytes16(uint16_t Arg);

// swap bytes in a long word
uint32_t SwapBytes32(uint32_t Arg);

// host to network short
uint16_t HtoNs(uint16_t Val);

// host to network long
uint32_t HtoNl(uint32_t Val);

// network to host short
uint16_t NtoHs(uint16_t Val);

// network to host long
uint32_t NtoHl(uint32_t Val);

// read an unsigned shortword big endian
uint16_t ReadUint16B(void const *pBuf) ;

// read an unsigned longword big endian
uint32_t ReadUint32B(void const *pBuf);

// read a signed shortword big endian
int16_t ReadInt16B(void const *pBuf);

// read a signed longword big endian
int32_t ReadInt32B(void const *pBuf);

// write an unsigned shortword big endian
void WriteUint16B(uint16_t Val, void *pBuf);

// write an unsigned longword big endian
void WriteUint32B(uint32_t Val, void *pBuf);

// write a signed shortword big endian
void WriteInt16B(int16_t Val, void *pBuf);

// write a signed longword big endian
void WriteInt32B(int32_t Val, void *pBuf);

// read an unsigned shortword little endian
uint16_t ReadUint16L(void const *pBuf);

// read an unsigned longword little endian
uint32_t ReadUint32L(void const *pBuf);

// read a signed shortword little endian
int16_t ReadInt16L(void const *pBuf);

// read a signed longword little endian
int32_t ReadInt32L(void const *pBuf);

// write an unsigned shortword little endian
void WriteUint16L(uint16_t Val, void *pBuf);

// write an unsigned longword little endian
void WriteUint32L(uint32_t Val, void *pBuf);

// write a signed shortword little endian
void WriteInt16L(int16_t Val, void *pBuf);

// write a signed longword little endian
void WriteInt32L(int32_t Val, void *pBuf);

// read the contents of a file into a buffer
size_t ReadFile(String const &Path, Buffer &FileData);

// write the contents of a buffer into a file
size_t WriteFile(String const &Path, Buffer &FileData);

// check if a string is alphanumeric
bool CheckAlphaNumeric(String const &Input);

// convert a string to upper case
String UpperCase(String const &Input);

// convert a string to lower case
String LowerCase(String const &Input);

// check if a string has a prefix
bool CheckPrefix(String const &Input, String const &Prefix);

// remove a prefix from a string if it is present
String RemovePrefix(String const &Input, String const &Prefix);

// replace all occurrences of a substring
String ReplaceSubString(String const &Input, String const &OrigSub, String const &NewSub);

// get components of a path
void GetPathComponents(String const &Path, String &Directory, String &Filename, String &Extension);

// ----------------------------------------------------------------------------
// platform dependent functions
// ----------------------------------------------------------------------------

// get the platform path separator
String const PathSep();

// get the current temp directory path
String const TempDir();

// test if a path exists
bool PathExists(String const &PathName);

// create a directory
bool DirCreate(String const &PathName);

// start a new process
uint32_t StartProcess(String const &FilePath, StringVec_t const &Args, StringVec_t const &EnvVars);

// run a program and get its output
bool RunProgramGetOutput(String const &Command, StringVec_t &Output);

// get current task identifier
uint32_t TaskId();

// get current thread identifier
uint32_t ThreadId();

// relinquish current thread's execution
bool ThreadYield();

// enter critical section
bool CriticalEnter();

// exit critical section
bool CriticalExit();

// return time in seconds (unix epoch)
uint32_t Time32();

// return time in milliseconds (unix epoch)
uint64_t Time64();

// return process CPU time in milliseconds
uint64_t CpuTime64();

// suspend execution for second intervals
bool Sleep(uint32_t Delay);

// suspend execution for millisecond intervals
bool MilliSleep(uint32_t Delay);

// suspend execution for microsecond intervals
bool MicroSleep(uint32_t Delay);

// suspend execution for nanosecond intervals
bool NanoSleep(uint32_t Delay);

// obtain the system's host name
String &HostName(String &Name);

// obtain the system's domain namae
String &DomainName(String &Name);

// determine if a path is a file or a directory
bool GetPathType(String const &Path, bool &IsFile, bool &IsDir);

// get file size
size_t GetFileSize(String const &Path);

// get file size
size_t GetFileSize(desc_t Descriptor);

// get file attributes
uint32_t GetFileAttr(String const &Path);

// get file attributes
uint32_t GetFileAttr(desc_t Descriptor);

// convert a numeric IPv4 address to a string
String Ipv4ToStr(uint32_t Addr);

// convert a numeric IPv4 address to a string
String Ipv4ToStr(void const *pAddr);

// convert a string to a numeric IPv4 address
uint32_t StrToIpv4(String const &Addr);

// convert a numeric IPv6 address to string
String Ipv6ToStr(void const *pAddress);

// convert a string to a numeric IPv6 address
Buffer StrToIpv6(String const &Addr);

}   // namespace cp

#endif // CP_UTIL_H
