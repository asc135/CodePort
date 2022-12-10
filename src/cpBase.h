// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2010 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpBase.h
//
//  Description:    Common Base Class.
//
//  Platform:       common
//
//  History:
//  2010-10-10  asc Creation.
//  2012-08-10  asc Moved identifiers to cp namespace.
//  2013-05-16  asc Added spare flags to align member memory.
//  2013-08-26  asc Redesigned flag support.
// ----------------------------------------------------------------------------

#ifndef CP_BASE_H
#define CP_BASE_H

#include "cpString.h"

namespace cp
{

class Base
{
public:
    // constructor
    Base(String const &Name) :
        m_Valid(false),
        m_Disabled(false),
        m_Flag1(false),
        m_Flag2(false),
        m_Name(Name)
    {
    }

    // destructor
    virtual ~Base()
    {
        m_Valid = false;
    }

    // return a copy of the instance's validity flag
    bool IsValid() const { return m_Valid; }

    // return a copy of the instance's disabled flag
    bool IsDisabled() const { return m_Disabled; }

    // return a copy of the instance's flag 1
    bool Flag1Get() const { return m_Flag1; }

    // return a copy of the instance's flag 2
    bool Flag2Get() const { return m_Flag2; }

    // return a copy of the instance's name string
    String const &NameGet() const { return m_Name; }

    // enable object
    void Enable() { m_Disabled = false; }

    // disable object
    void Disable() { m_Disabled = true; }

    // set flag 1 state
    void Flag1Set(bool State) { m_Flag1 = State; }

    // set flag 2 state
    void Flag2Set(bool State) { m_Flag2 = State; }

protected:
    // return a copy of the instance's validity flag and print an error if invalid
    bool IsValid(String const &MethodId) const
    {
        if (m_Valid == false)
        {
            LogErr << MethodId << ", instance: " << this << " cannot be called.  Object initialization invalid." << std::endl;
        }

        return m_Valid;
    }

    bool                m_Valid;                            // true when the instance initialized successfully
    bool                m_Disabled;                         // true when objection is disabled
    bool                m_Flag1;                            // general purpose flag 1
    bool                m_Flag2;                            // general purpose flag 2

private:
    String              m_Name;                             // the instance's name
};

}   // namespace cp

#endif  // CP_BASE_H
