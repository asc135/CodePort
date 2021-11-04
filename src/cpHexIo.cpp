// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpHexIo.cpp
//
//  Description:    Hex Utility Function Library.
//
//  Platform:       common
//
//  History:
//  2011-08-01  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-04-03  asc Cleared outpu string in HexEncode().
// ----------------------------------------------------------------------------

#include <cstdlib>

#include "cpBuffer.h"
#include "cpUtil.h"

namespace cp
{

static size_t const k_OutputCharsPerInputOctet = 2;

static void FormatOutput(String &Output, HexIoCfg &Form, bool Final = false);

// ----------------------------------------------------------------------------


// display the format structure elements
void HexIoCfg::Display()
{
    LogMsg << "------------------------------" << std::endl;
    LogMsg << "groupLenMax  = " << groupLenMax << std::endl;
    LogMsg << "groupLen     = " << groupLen    << std::endl;
    LogMsg << "lineLenMax   = " << lineLenMax  << std::endl;
    LogMsg << "lineLen      = " << lineLen     << std::endl;
    LogMsg << "preLine      = " << (preLine   ? "T" : "F") << std::endl;
    LogMsg << "postLine     = " << (postLine  ? "T" : "F") << std::endl;
    LogMsg << "preserve     = " << (preserve  ? "T" : "F") << std::endl;
    LogMsg << "finalPass    = " << (finalPass ? "T" : "F") << std::endl;
    LogMsg << "prefix       = '" << prefix     << "'" << std::endl;
    LogMsg << "suffix       = '" << suffix     << "'" << std::endl;
    LogMsg << "separator    = '" << separator  << "'" << std::endl;
    LogMsg << "------------------------------" << std::endl;
}


// encode a block of data to ASCII hex
size_t HexEncode(Buffer &Input, String &Output, HexIoCfg &Form)
{
    Buffer buf(16);

    // clear the output string
    Output.clear();

    // make sure maximum group length is not zero
    if (Form.groupLenMax == 0)
    {
        ++Form.groupLenMax;
    }

    // initialize some values if this is not a successive call
    if (Form.preserve == false)
    {
        Form.groupLen = 0;
        Form.lineLen = 0;
    }

    // convert the input data to hex
    for (size_t i = 0; i < Input.LenGet(); ++i)
    {
        // insert any formatting items
        FormatOutput(Output, Form);

        // convert and add an octet
        snprintf(buf, buf, "%2.2x", *Input.u_str(i));
        Output += buf.c_str();
        Form.lineLen += k_OutputCharsPerInputOctet;
        ++Form.groupLen;
    }

    // post-data formatting
    if (Form.finalPass)
    {
        // check if any final formatting needs to be inserted
        FormatOutput(Output, Form, true);
    }

    return Output.size();
}


// decode a block of ASCII hex data
size_t HexDecode(String const &Input, Buffer &Output)
{
    char ch = 0;
    size_t index = 0;
    String hexDigits = "0123456789ABCDEFabcdef";
    String element;

    // resize and clear the buffer
    Output.Resize(Input.size() / 2);

    // decode the input hex data
    for (size_t i = 0; i < Input.size(); ++i)
    {
        // get a digit
        ch = Input[i];

        // discard any C hex prefix (0x)
        if ( ((ch == 'x') || (ch == 'X')) && (element == "0"))
        {
           element.clear();
        }

        // check if a valid hex digit was read
        if (hexDigits.find(ch) != String::npos)
        {
            // concatenate it to the element string
            element += ch;
        }

        // check if a pair of hex digits are ready to decode
        if (element.size() == k_OutputCharsPerInputOctet)
        {
            // translate and write the octet to the output buffer
            if (Output.Size() > index)
            {
                *Output.u_str(index++) = strtoul(element.c_str(), NULL, 16);
            }

            // clear the processed element
            element.clear();
        }
    }

    // set the buffer data length
    Output.LenSet(index);

    return index;
}


// insert formatting tokens into output
static void FormatOutput(String &Output, HexIoCfg &Form, bool Final)
{
    // actions when ending a group or on final pass
    if (Final || (Form.groupLen >= Form.groupLenMax))
    {
        // zero the group length
        Form.groupLen = 0;

        // add any suffix
        if (Form.suffix.size() > 0)
        {
            Output += Form.suffix;
            Form.lineLen += Form.suffix.size();
        }
    }

    // actions when starting a group or on final pass
    if (Final || (Form.groupLen == 0))
    {
        // determine the number of formatted characters per output group
        size_t tokenLen = Form.prefix.size() + (k_OutputCharsPerInputOctet * Form.groupLenMax) + Form.suffix.size();
        size_t sepLen = Form.separator.size();

        // start a new line if there isn't enough room for another output group or if final pass
        if (Final || ((Form.lineLenMax > 0) && ((Form.lineLen + tokenLen + (Form.postLine ? sepLen : 0)) > Form.lineLenMax)))
        {
            // append any post-line separator
            if (Form.postLine)
            {
                Output += Form.separator;
            }

            Output += "\n";
            Form.lineLen = 0;
        }
        else
        {
            // if not final pass and not beginning of line add a separator
            if (Form.lineLen > 0)
            {
                // add a group separator
                Output += Form.separator;
                Form.lineLen += sepLen;
            }
        }

        // actions at a group start when not on a final pass
        if (!Final)
        {
            // if specified, add a pre-line separator
            if (Form.preLine && (Form.lineLen == 0))
            {
                Output += Form.separator;
                Form.lineLen += sepLen;
            }

            // if specified, at a group prefix
            if (Form.prefix.size() > 0)
            {
                Output += Form.prefix;
                Form.lineLen += Form.prefix.size();
            }
        }
    }
}

}   // namespace cp
