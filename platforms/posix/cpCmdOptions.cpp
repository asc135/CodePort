// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpCmdOptions.cpp
//
//  Description:    Command Line Options Parser.
//
//  Platform:       posix
//
//  History:
//  2011-08-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
// ----------------------------------------------------------------------------

#include "cpPlatform.h"
#include "cpCmdOptions.h"

namespace cp
{

// Constructor
CmdOptions::CmdOptions(String const &ShortOptions, option const *pLongOptions)
{
    // prefix of "-" causes non-option arguments to be treated as
    // if they are arguments for an option of character code 0x01
    // which facilitates parsing into the result vector
    // the prefix ":" means missing option arguments are indicated
    // with a colon instead of question mark which is normally
    // used to indicate an unrecognized argument
    m_ShortOptions = "-:";
    m_ShortOptions += ShortOptions;

    // long options are defined as an array of structures
    m_LongOptions = pLongOptions;
}


// Destructor
CmdOptions::~CmdOptions()
{

}


bool CmdOptions::Parse(int argc, char *argv[])
{
    bool rv = true;
    bool exitFlag = false;
    int result;
    int longIndex;
    String longOption;
    char shortOption[2] = { 0 };

    // disable getopt() or getopt_long() from issuing its own error messages to stdout.
    opterr = 0;

    // process the command line arguments
    while (!exitFlag)
    {
        // run the correct version of getopt based on the supplied options
        if (m_LongOptions != NULL)
        {
            result = getopt_long(argc, argv, m_ShortOptions.c_str(), m_LongOptions, &longIndex);
        }
        else
        {
            result = getopt(argc, argv, m_ShortOptions.c_str());
        }

        switch (result)
        {
        case -1:
            exitFlag = true;
            break;

        case 0:     // long option with possible arguments
            longOption = m_LongOptions[longIndex].name;

            if (optarg != NULL)
            {
                m_OptionsFound[longOption] = optarg;
            }
            else
            {
                m_OptionsFound[longOption] = "t";
            }
            break;

        case 1:     // non-option parameters
            m_NonOptionsFound.push_back(optarg);
            break;

        case ':':   // missing option argument
            LogErr << "\nError: Missing argument to option: " << argv[optind - 1] << "\n" << std::endl;
            rv = false;
            break;

        case '?':   // unrecognized option
            LogErr << "\nError: Invalid option: " << argv[optind - 1] << "\n" << std::endl;
            rv = false;
            break;

        default:    // all other cases
            shortOption[0] = result;

            if (optarg != NULL)
            {
                m_OptionsFound[shortOption] = optarg;
            }
            else
            {
                m_OptionsFound[shortOption] = "t";
            }
            break;
        }
    }

    return rv;
}

}   // namespace cp
