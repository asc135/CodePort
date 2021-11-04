// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2011 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpCmdOptions.h
//
//  Description:    Command Line Options Parser.
//
//  Platform:       posix
//
//  History:
//  2011-08-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-12-18  asc Replaced locally defined string vectors with pooled type.
// ----------------------------------------------------------------------------

#ifndef CP_CMDOPTIONS_H
#define CP_CMDOPTIONS_H

#include <map>

#include <getopt.h>

#include "cpString.h"

namespace cp
{

class CmdOptions
{
public:
    // types
    typedef std::map<String, String> OptionMap_t;

    // constructor
    CmdOptions(String const &ShortOptions, option const *pLongOptions = NULL);

    // destructor
    ~CmdOptions();

    // manipulators
    bool Parse(int argc, char *argv[]);

    // accessors
    StringVec_t &NonOptions() { return m_NonOptionsFound; }
    OptionMap_t &Options() { return m_OptionsFound; }

private:
    String                  m_ShortOptions;                 // possible short options
    option const           *m_LongOptions;                  // possible long options
    OptionMap_t             m_OptionsFound;                 // options and arguments found
    StringVec_t             m_NonOptionsFound;              // non-option arguments found
};

}   // namespace cp

#endif // CP_CMDOPTIONS_H
