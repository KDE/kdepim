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
# pragma implementation "RMM_MimeType.h"
#endif

#include <qcstring.h>

#include <RMM_Defines.h>
#include <RMM_MimeType.h>
#include <RMM_Enum.h>

using namespace RMM;

RMimeType::RMimeType()
    :   RHeaderBody()
{
    // Empty.
}

RMimeType::RMimeType(const RMimeType & t)
    :    RHeaderBody(t)
{
    // Empty.
}

RMimeType::RMimeType(const QCString & s)
    :    RHeaderBody(s)
{
    // Empty.
}


RMimeType::~RMimeType()
{
    // Empty.
}

    RMimeType &
RMimeType::operator = (const RMimeType & t)
{
    if (this == &t) return *this; // Avoid a = a
    
    boundary_        = t.boundary_;
    name_            = t.name_;
    type_            = t.type_;
    subType_        = t.subType_;
    parameterList_    = t.parameterList_;
    
    RHeaderBody::operator = (t);
    return *this;
}

    bool
RMimeType::operator == (RMimeType & t)
{
    parse();
    t.parse();
    
    return (
        boundary_        == t.boundary_    &&
        name_            == t.name_        &&
        type_            == t.type_        &&
        subType_        == t.subType_    &&
        parameterList_    == t.parameterList_);
}

    MimeType
RMimeType::type()
{
    parse();
    return type_;
}

    void
RMimeType::setType(MimeType t)
{
    parse();
    type_ = t;
}

    void
RMimeType::setType(const QCString & s)
{
    parse();
    type_ = mimeTypeStr2Enum(s);
}

    MimeSubType
RMimeType::subType()
{
    parse();
    return subType_;
}

    void
RMimeType::setSubType(MimeSubType t)
{
    parse();
    subType_ = t;
}

    void
RMimeType::setSubType(const QCString & s)
{
    parse();
    subType_ = mimeSubTypeStr2Enum(s);
}

    QCString
RMimeType::boundary()
{
    parse();
    return boundary_;
}

    void
RMimeType::setBoundary(const QCString & s)
{
    parse();
    boundary_ = s;
}

    QCString
RMimeType::name()
{
    parse();
    return name_;
}

    void
RMimeType::setName(const QCString & s)
{
    parse();
    name_ = s;
}

    void
RMimeType::_parse()
{
    // STUB
}

    void
RMimeType::_assemble()
{
    // STUB
}

    void
RMimeType::createDefault()
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
