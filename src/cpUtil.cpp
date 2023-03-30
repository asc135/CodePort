// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpUtil.cpp
//
//  Description:    Utility Function Library.
//
//  Platform:       common
//
//  History:
//  2011-04-29  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-02-04  asc Changed network/host functions to upper camel case.
//  2013-11-15  asc Added cascade capability to CRC functions.
//  2013-12-18  asc Added DirCreate() and moved PathCreate() into common module.
//  2013-12-18  asc Changed output parameter of Tokenize() to StringVec_t.
//  2013-12-18  asc Added Lchomp() and Rchomp() functions.
//  2014-03-30  asc Added ReadFile() and WriteFile() functions.
//  2022-04-18  asc Added .is_open() test for streams to workaround AIX bug.
//  2022-05-23  asc Added Int64ToStr() and Uint64ToStr() functions.
//  2022-05-25  asc Switched to printf macros for stdint.h types portability.
//  2022-06-10  asc Added BufferToLines() function.
//  2022-07-08  asc Modified Tokenize() to not eat spaces.
//  2022-09-09  asc Added CheckAlphaNumeric() function.
//  2023-03-29  asc Added StrToInt64() and StrToUint64() functions.
// ----------------------------------------------------------------------------

#include <fstream>
#include <cinttypes>

#include "cpUtil.h"
#include "cpBuffer.h"

namespace cp
{

// create a path including intermediate directories
bool PathCreate(String const &PathName)
{
    bool rv = true;
    String directory;
    StringVec_t pathElements;
    size_t numDirs;

    numDirs = Tokenize(PathName, PathSep(), pathElements);

    for (size_t i = 0; rv && (i < numDirs); ++i)
    {
        // first element blank means it was a root path
        if ((i == 0) && (pathElements[i].length() == 0))
        {
            pathElements[i] = PathSep();
        }

        // concatenate the next element
        directory = directory + pathElements[i];

        // create the directory
        rv = rv && DirCreate(directory);

        // concatenate path separator for next pass
        if (directory != PathSep())
        {
            directory = directory + PathSep();
        }
    }

    return rv;
}


// dump a block of data to a stream in Hex ASCII format
bool HexDump(std::ostream &Out, Buffer const &Buf, size_t LineLen)
{
    return HexDump(Out, Buf.u_str(), Buf.LenGet(), LineLen);
}


// dump a block of data to a stream in Hex ASCII format
bool HexDump(std::ostream &Out, char const *Data, size_t DataLen, size_t LineLen)
{
    return HexDump(Out, reinterpret_cast<uint8_t const *>(Data), DataLen, LineLen);
}


// dump a block of data to a stream in Hex ASCII format
bool HexDump(std::ostream &Out, uint8_t const *Data, size_t DataLen, size_t LineLen)
{
    size_t line;
    size_t chr;

    if (Data == NULL)
    {
        return false;
    }

    for (line = 0; line < DataLen; line += LineLen)
    {
        Out << std::hex << std::setw(4) << std::setfill('0') << line << ": ";

        for (chr = 0; chr < LineLen; ++chr)
        {
            if ((line + chr) < DataLen)
            {
                Out << " " << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)(Data[line + chr]);
            }
            else
            {
                Out << "   ";
            }
        }

        Out << "  ";

        for (chr = 0; chr < LineLen; ++chr)
        {
            if ((line + chr) < DataLen)
            {
                uint8_t c = Data[line + chr];

                if ((c >= ' ') && (c < CHAR_DEL))
                {
                    Out << static_cast<char>(c);
                }
                else
                {
                    Out << ".";
                }
            }
        }

        Out << std::dec << std::endl;
    }

    return Out.good();
}


// returns a string with the specified number of characters
String const GenStr(size_t Count, char Ch)
{
    char c[2] = { Ch, '\0' };
    String spc;

    // cap it to some reasonable value
    if (Count > 65535)
    {
        Count = 65535;
    }

    while (Count--)
    {
        spc = spc + c;
    }

    return spc;
}


// trims leading whitespace from a string
String &Ltrim(String &Str)
{
    String::size_type pos = 0;
    bool exitFlag = false;

    // loop through string
    while ((pos < Str.size()) && !exitFlag)
    {
        // check if the character is whitespace
        if (Str[pos] == ' ' || Str[pos] == '\t' || Str[pos] == '\r' || Str[pos] == '\n')
        {
            //skip past it
            ++pos;
        }
        else
        {
            // trim the string
            Str.erase(0, pos);
            exitFlag = true;
        }
    }

    // check if only whitespace was found
    if (!exitFlag)
    {
        // if so, erase all the whole string
        Str.clear();
    }

    return Str;
}


// trims trailing whitespace from a string
String &Rtrim(String &Str)
{
    String::size_type pos = Str.size();
    bool exitFlag = false;

    // loop through string
    while ((pos > 0) && !exitFlag)
    {
        // check if the character is whitespace
        if (Str[pos - 1] == ' ' || Str[pos - 1] == '\t' || Str[pos - 1] == '\r' || Str[pos - 1] == '\n')
        {
            // skip past it
            --pos;
        }
        else
        {
            // trim the string
            Str.erase(pos);
            exitFlag = true;
        }
    }

    // check if only whitespace was found
    if (!exitFlag)
    {
        // if so, erase all the whole string
        Str.clear();
    }

    return Str;
}


// chomps characters from the beginning of a string
String &Lchomp(String &Str, size_t Count)
{
    if (Str.length() > Count)
    {
        Str = Str.substr(Count);
    }
    else
    {
        Str.clear();
    }

    return Str;
}


// chomps characters from the end of a string
String &Rchomp(String &Str, size_t Count)
{
    if (Str.length() > Count)
    {
        Str = Str.substr(0, Str.length() - Count);
    }
    else
    {
        Str.clear();
    }

    return Str;
}


// converts string to long integer
int32_t StrToInt(String const &Str)
{
    return strtol(Str.c_str(), NULL, 10);
}


// converts string to unsigned long integer
uint32_t StrToUint(String const &Str)
{
    return strtoul(Str.c_str(), NULL, 10);
}


// converts string to long long integer
int64_t StrToInt64(String const &Str)
{
    return strtoll(Str.c_str(), NULL, 10);
}


// converts string to unsigned long long integer
uint64_t StrToUint64(String const &Str)
{
    return strtoull(Str.c_str(), NULL, 10);
}


// converts string to float
double StrToFloat(String const &Str)
{
    return strtod(Str.c_str(), NULL);
}


// converts string to boolean
bool StrToBool(String const &Str)
{
    bool rv = false;

    // if the string has any characers, the first one is used for determining
    // the boolean value: 0, f, and F mean false but everything else means true
    if (Str.size() > 0)
    {
        switch (*Str.c_str())
        {
        case '0':
        case 'f':
        case 'F':
            rv = false;
            break;

        default:
            rv = true;
            break;
        }
    }

    return rv;
}


// converts integer to string
String IntToStr(int Val)
{
    Buffer buf(32);

    snprintf(buf, buf, "%d", Val);

    return buf.c_str();
}


// converts unsigned to string integer
String UintToStr(uint32_t Val)
{
    Buffer buf(32);

    snprintf(buf, buf, "%u", Val);

    return buf.c_str();
}


// converts integer to string
String Int64ToStr(int64_t Val)
{
    Buffer buf(64);

    snprintf(buf, buf, "%" PRId64, Val);

    return buf.c_str();
}


// converts unsigned to string integer
String Uint64ToStr(uint64_t Val)
{
    Buffer buf(64);

    snprintf(buf, buf, "%" PRIu64, Val);

    return buf.c_str();
}


// converts float to string
String FloatToStr(double Val)
{
    Buffer buf(32);

    snprintf(buf, buf, "%f", Val);

    return buf.c_str();
}


// converts boolean to string
String BoolToStr(bool Val)
{
    if (Val)
    {
        return "true";
    }
    else
    {
        return "false";
    }
}


// tokenize a string into a vector of strings
size_t Tokenize(String const &StringIn, String const &Delim, StringVec_t &TokensOut)
{
    size_t n;
    String remains;
    String token;

    TokensOut.clear();
    remains = StringIn;

    // locate delimiters and break up string into tokens
    while (remains.length())
    {
        // locate delimiter if present
        n = remains.find(Delim);

        // if found, grab substring up to the delimiter,
        // otherwise remaining string is the last token
        if (n != String::npos)
        {
            token = remains.substr(0, n);
            remains = remains.substr(n + Delim.length());
        }
        else
        {
            token = remains;
            remains = "";
        }

        // Add token to the token vector.
        TokensOut.push_back(token);
    }

    return TokensOut.size();
}


// break a text buffer into a vector of lines
void BufferToLines(char const *pBufferIn, size_t BufferSize, StringVec_t &LinesOut)
{
    char const *start = pBufferIn;
    char const *ptr = start;
    size_t lineLen = 0;
    size_t count = BufferSize;

    LinesOut.clear();

    // return if invalid buffer
    if (!start)
    {
        return;
    }

    while (count--)
    {
        // check if we found a line ending or NULL
        if (*ptr == '\n' || *ptr == '\r' || *ptr == 0)
        {
            // determine line length
            lineLen = ptr - start;

            // add the line to the output vector
            if (lineLen)
            {
                LinesOut.emplace_back(start, lineLen);
            }
            else
            {
                if (*ptr != 0)
                {
                    LinesOut.push_back("");
                }
            }

            // check if it is a conjugate line ending (two chars like \n\r or \r\n)
            if (count > 0)
            {
                if ((*ptr == '\n' && *(ptr + 1) == '\r') || (*ptr == '\r' && *(ptr + 1) == '\n'))
                {
                    // conjugate line ending found, skip over second character
                    ++ptr;
                    --count;
                }
            }

            // increment the pointer and assign the new string start position
            ++ptr;
            start = ptr;
        }
        else
        {
            // check if buffer end reached without a line ending
            if (count == 0)
            {
                lineLen = ptr - start + 1;

                if (lineLen)
                {
                    LinesOut.emplace_back(start, lineLen);
                }
            }

            // increment the pointer
            ++ptr;
        }
    }
}


// reverse the bits in an octet
uint8_t Reflect8(uint8_t Val)
{
    return k_ReflectLut[Val];
}


// reverse the bits in a short word
uint16_t Reflect16(uint16_t Val)
{
    uint16_t result = 0;

    result |= (k_ReflectLut[(Val      ) & 0xff] <<  8);
    result |= (k_ReflectLut[(Val >>  8)       ]      );

    return result;
}


// reverse the bits in a long word
uint32_t Reflect32(uint32_t Val)
{
    uint32_t result = 0;

    result |= (k_ReflectLut[(Val      ) & 0xff] << 24);
    result |= (k_ReflectLut[(Val >>  8) & 0xff] << 16);
    result |= (k_ReflectLut[(Val >> 16) & 0xff] <<  8);
    result |= (k_ReflectLut[(Val >> 24)       ]      );

    return result;
}


// calculate CRC-16 for a block of data
uint16_t CalcCrc16(Buffer &Buf, uint16_t Cascade)
{
    return CalcCrc16(Buf.u_str(), Buf.LenGet(), Cascade);
}


// calculate CRC-16 for a block of data
uint16_t CalcCrc16(char const *pBuf, size_t Size, uint16_t Cascade)
{
    return CalcCrc16(reinterpret_cast<uint8_t const *>(pBuf), Size, Cascade);
}


// calculate CRC-16 for a block of data
uint16_t CalcCrc16(uint8_t const *pBuf, size_t Size, uint16_t Cascade)
{
    uint16_t xorOut    = 0;
    uint16_t remainder = Cascade;
    uint16_t mask      = 0x8000;
    uint16_t gen       = 0x1021;
    size_t shift       = 8;

    // check for NULL pointer
    if (pBuf == NULL)
    {
        return 0;
    }

    // process the input octets
    for (size_t byte = 0; byte < Size; ++byte)
    {
        remainder ^= ((*pBuf++) << shift);

        // divide by the generator and save the remainder
        for (size_t bit = 0; bit < 8; ++bit)
        {
            if (remainder & mask)
            {
                remainder = (remainder << 1) ^ gen;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    return (remainder ^ xorOut);
}


// calculate CRC-32 for a block of data
uint32_t CalcCrc32(Buffer &Buf, uint32_t Cascade)
{
    return CalcCrc32(Buf.u_str(), Buf.LenGet(), Cascade);
}


// calculate CRC-32 for a block of data
uint32_t CalcCrc32(char const *pBuf, size_t Size, uint32_t Cascade)
{
    return CalcCrc32(reinterpret_cast<uint8_t const *>(pBuf), Size, Cascade);
}


// calculate CRC-32 for a block of data
uint32_t CalcCrc32(uint8_t const *pBuf, size_t Size, uint32_t Cascade)
{
    uint32_t xorOut    = 0xffffffff;
    uint32_t remainder = xorOut;
    uint32_t mask      = 0x80000000;
    uint32_t gen       = 0x04c11db7;
    size_t shift       = 24;

    // check for NULL pointer
    if (pBuf == NULL)
    {
        return 0;
    }

    // check for data cascade
    if (Cascade != xorOut)
    {
        remainder = Reflect32((Cascade ^ xorOut));
    }

    // process the input octets
    for (size_t byte = 0; byte < Size; ++byte)
    {
        remainder ^= (Reflect8(*pBuf++) << shift);

        // divide by the generator and save the remainder
        for (size_t bit = 0; bit < 8; ++bit)
        {
            if (remainder & mask)
            {
                remainder = (remainder << 1) ^ gen;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    return (Reflect32(remainder) ^ xorOut);
}


// detect host byte order
bool HostBigEndian()
{
    uint16_t word = 1;
    uint8_t *pByte = reinterpret_cast<uint8_t *>(&word);

    return (*pByte != word);
}


// detect host byte order
bool HostLittleEndian()
{
    uint16_t word = 1;
    uint8_t *pByte = reinterpret_cast<uint8_t *>(&word);

    return (*pByte == word);
}


// swap bytes in a short word
uint16_t SwapBytes16(uint16_t Arg)
{
    uint16_t result = 0;

    result |= (Arg << 8);
    result |= (Arg >> 8);

    return result;
}


// swap bytes in a long word
uint32_t SwapBytes32(uint32_t Arg)
{
    uint32_t result = 0;

    result |= (Arg >> 24);
    result |= (Arg << 24);
    result |= ((Arg >> 8) & 0x0000ff00);
    result |= ((Arg << 8) & 0x00ff0000);

    return result;
}


// host to network short
uint16_t HtoNs(uint16_t Val)
{
    if (HostLittleEndian())
    {
        return SwapBytes16(Val);
    }
    else
    {
        return Val;
    }
}


// host to network long
uint32_t HtoNl(uint32_t Val)
{
    if (HostLittleEndian())
    {
        return SwapBytes32(Val);
    }
    else
    {
        return Val;
    }
}


// network to host short
uint16_t NtoHs(uint16_t Val)
{
    if (HostLittleEndian())
    {
        return SwapBytes16(Val);
    }
    else
    {
        return Val;
    }
}


// network to host long
uint32_t NtoHl(uint32_t Val)
{
    if (HostLittleEndian())
    {
        return SwapBytes32(Val);
    }
    else
    {
        return Val;
    }
}


// read an unsigned shortword big endian
uint16_t ReadUint16B(void const *pBuf)
{
    uint8_t const *p = reinterpret_cast<uint8_t const *>(pBuf);
    uint16_t v = 0;

    v |= (*(p + 0) << 8);
    v |= (*(p + 1));

    return v;
}


// read an unsigned longword big endian
uint32_t ReadUint32B(void const *pBuf)
{
    uint8_t const *p = reinterpret_cast<uint8_t const *>(pBuf);
    uint32_t v = 0;

    v |= (*(p + 0) << 24);
    v |= (*(p + 1) << 16);
    v |= (*(p + 2) << 8);
    v |= (*(p + 3));

    return v;
}


// read a signed shortword big endian
int16_t ReadInt16B(void const *pBuf)
{
    return static_cast<int16_t>(ReadUint16B(pBuf));
}


// read a signed longword big endian
int32_t ReadInt32B(void const *pBuf)
{
    return static_cast<int32_t>(ReadUint32B(pBuf));
}


// write an unsigned shortword big endian
void WriteUint16B(uint16_t Val, void *pBuf)
{
    uint8_t *p = reinterpret_cast<uint8_t *>(pBuf);

    *(p + 0) = Val >> 8;
    *(p + 1) = Val;
}


// write an unsigned longword big endian
void WriteUint32B(uint32_t Val, void *pBuf)
{
    uint8_t *p = reinterpret_cast<uint8_t *>(pBuf);

    *(p + 0) = Val >> 24;
    *(p + 1) = Val >> 16;
    *(p + 2) = Val >> 8;
    *(p + 3) = Val;
}


// write a signed shortword big endian
void WriteInt16B(int16_t Val, void *pBuf)
{
    WriteUint16B(static_cast<uint16_t>(Val), pBuf);
}


// write a signed longword big endian
void WriteInt32B(int32_t Val, void *pBuf)
{
    WriteUint32B(static_cast<uint32_t>(Val), pBuf);
}


// read an unsigned shortword little endian
uint16_t ReadUint16L(void const *pBuf)
{
    uint8_t const *p = reinterpret_cast<uint8_t const *>(pBuf);
    uint16_t v = 0;

    v |= (*(p + 1) << 8);
    v |= (*(p + 0));

    return v;
}


// read an unsigned longword little endian
uint32_t ReadUint32L(void const *pBuf)
{
    uint8_t const *p = reinterpret_cast<uint8_t const *>(pBuf);
    uint32_t v = 0;

    v |= (*(p + 3) << 24);
    v |= (*(p + 2) << 16);
    v |= (*(p + 1) << 8);
    v |= (*(p + 0));

    return v;
}


// read a signed shortword little endian
int16_t ReadInt16L(void const *pBuf)
{
    return static_cast<int16_t>(ReadUint16L(pBuf));
}


// read a signed longword little endian
int32_t ReadInt32L(void const *pBuf)
{
    return static_cast<int32_t>(ReadUint32L(pBuf));
}


// write an unsigned shortword little endian
void WriteUint16L(uint16_t Val, void *pBuf)
{
    uint8_t *p = reinterpret_cast<uint8_t *>(pBuf);

    *(p + 1) = Val >> 8;
    *(p + 0) = Val;
}


// write an unsigned longword little endian
void WriteUint32L(uint32_t Val, void *pBuf)
{
    uint8_t *p = reinterpret_cast<uint8_t *>(pBuf);

    *(p + 3) = Val >> 24;
    *(p + 2) = Val >> 16;
    *(p + 1) = Val >> 8;
    *(p + 0) = Val;
}


// write a signed shortword little endian
void WriteInt16L(int16_t Val, void *pBuf)
{
    WriteUint16L(static_cast<uint16_t>(Val), pBuf);
}


// write a signed longword little endian
void WriteInt32L(int32_t Val, void *pBuf)
{
    WriteUint32L(static_cast<uint32_t>(Val), pBuf);
}


// read the contents of a file into a buffer
size_t ReadFile(String const &Path, Buffer &FileData)
{
    std::ifstream dataFile(Path.c_str(), std::ios::ate | std::ifstream::binary);
    size_t fileLen = 0;
    size_t numRead = 0;

    if (dataFile.is_open() && dataFile.good())
    {
        fileLen = dataFile.tellg();

        if (fileLen > 0)
        {
            if (FileData.Resize(fileLen))
            {
                dataFile.seekg(0);
                dataFile.read(FileData, fileLen);
                numRead = dataFile.gcount();
                FileData.LenSet(numRead);
            }
        }
    }

    return numRead;
}


// write the contents of a buffer into a file
size_t WriteFile(String const &Path, Buffer &FileData)
{
    std::ofstream dataFile(Path.c_str(), std::ofstream::binary);
    size_t fileLen = 0;
    size_t numWritten = 0;

    if (dataFile.is_open() && dataFile.good())
    {
        fileLen = FileData.LenGet();

        if (fileLen > 0)
        {
            dataFile.write(FileData, fileLen);
            numWritten = dataFile.tellp();
        }
    }

    return numWritten;
}


// check if a string is alphanumeric
bool CheckAlphaNumeric(String const &Input)
{
    bool rv = !Input.empty();

    for (auto ch : Input)
    {
        rv = rv && (((ch >= '0' ) && (ch <= '9')) || ((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z')));
    }

    return rv;
}

}   // namespace cp
