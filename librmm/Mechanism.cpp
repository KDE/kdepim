/* This file is part of the KDE project

   Copyright (C) 1999, 2000 Rik Hemsley <rik@kde.org>
             (C) 1999, 2000 Wilco Greven <j.w.greven@student.utwente.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include <rmm/Mechanism.h>
#include <rmm/Enum.h>
#include <rmm/Defines.h>

using namespace RMM;

Mechanism::Mechanism()
{
    // Empty.
}

Mechanism::Mechanism(const Mechanism & m)
    :    HeaderBody(m)
{
    // Empty.
}

Mechanism::Mechanism(const QCString & s)
    :    HeaderBody(s)
{
    // Empty.
}

Mechanism::~Mechanism()
{
    // Empty.
}

    Mechanism &
Mechanism::operator = (const Mechanism & m)
{
    if (this == &m) return *this; // Don't do a = a.
    
    HeaderBody::operator = (m);
    return *this;
}

    Mechanism &
Mechanism::operator = (const QCString & s)
{
    HeaderBody::operator = (s);
    return *this;
}

    bool
Mechanism::operator == (Mechanism & m)
{
    parse();
    m.parse();

    return (HeaderBody::operator == (m));
}

    void
Mechanism::_parse()
{
    // STUB
}

    void
Mechanism::_assemble()
{
    // STUB
}

    void
Mechanism::createDefault()
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
