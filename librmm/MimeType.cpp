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

#include <qcstring.h>

#include <rmm/Defines.h>
#include <rmm/MimeType.h>
#include <rmm/Enum.h>

using namespace RMM;

MimeType::MimeType()
    :   HeaderBody()
{
    // Empty.
}

MimeType::MimeType(const MimeType & t)
    :   HeaderBody      (t),
        boundary_       (t.boundary_.copy()),
        name_           (t.name_.copy()),
        group_          (t.group_),
        value_          (t.value_),
        parameterList_  (t.parameterList_)
{
    // Empty.
}

MimeType::MimeType(const QCString & s)
    :    HeaderBody(s)
{
    // Empty.
}


MimeType::~MimeType()
{
    // Empty.
}

    MimeType &
MimeType::operator = (const MimeType & t)
{
    if (this == &t) return *this; // Avoid a = a
    
    boundary_           = t.boundary_.copy();
    name_               = t.name_.copy();
    group_              = t.group_;
    value_              = t.value_;
    parameterList_      = t.parameterList_;
    
    HeaderBody::operator = (t);
    return *this;
}

    bool
MimeType::operator == (MimeType & t)
{
    parse();
    t.parse();

    return (
        boundary_       == t.boundary_          &&
        name_           == t.name_              &&
        group_          == t.group_             &&
        value_          == t.value_             &&
        parameterList_  == t.parameterList_);
}

    MimeGroup
MimeType::group()
{
    parse();
    return group_;
}

    void
MimeType::setGroup(MimeGroup t)
{
    parse();
    group_ = t;
}

    void
MimeType::setGroup(const QCString & s)
{
    parse();
    group_ = mimeGroupStr2Enum(s);
}

    MimeValue
MimeType::value()
{
    parse();
    return value_;
}

    void
MimeType::setValue(MimeValue t)
{
    parse();
    value_ = t;
}

    void
MimeType::setValue(const QCString & s)
{
    parse();
    value_ = mimeValueStr2Enum(s);
}

    QCString
MimeType::boundary()
{
    parse();
    return boundary_;
}

    void
MimeType::setBoundary(const QCString & s)
{
    parse();
    boundary_ = s.copy();
}

    QCString
MimeType::name()
{
    parse();
    return name_;
}

    void
MimeType::setName(const QCString & s)
{
    parse();
    name_ = s.copy();
}

    void
MimeType::_parse()
{
    // STUB
}

    void
MimeType::_assemble()
{
    // STUB
}

    void
MimeType::createDefault()
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
