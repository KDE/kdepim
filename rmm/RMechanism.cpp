/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "RMM_Mechanism.h"
#endif

#include <RMM_Mechanism.h>
#include <RMM_Enum.h>
#include <RMM_Defines.h>

using namespace RMM;

RMechanism::RMechanism()
{
    // Empty.
}

RMechanism::RMechanism(const RMechanism & m)
    :    RHeaderBody(m)
{
    // Empty.
}

RMechanism::RMechanism(const QCString & s)
    :    RHeaderBody(s)
{
    // Empty.
}

RMechanism::~RMechanism()
{
    // Empty.
}

    RMechanism &
RMechanism::operator = (const RMechanism & m)
{
    if (this == &m) return *this; // Don't do a = a.
    
    RHeaderBody::operator = (m);
    return *this;
}

    RMechanism &
RMechanism::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RMechanism::operator == (RMechanism & m)
{
    parse();
    m.parse();

    return (RHeaderBody::operator == (m));
}

    void
RMechanism::_parse()
{
    // STUB
}

    void
RMechanism::_assemble()
{
    // STUB
}

    void
RMechanism::createDefault()
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
