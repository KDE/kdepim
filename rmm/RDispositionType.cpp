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
# pragma implementation "RMM_DispositionType.h"
#endif

#include <RMM_DispositionType.h>
#include <RMM_Token.h>

using namespace RMM;

RDispositionType::RDispositionType()
    :    RHeaderBody()
{
    // Empty.
}

RDispositionType::RDispositionType(const RDispositionType & t)
    :    RHeaderBody(t)
{
    // Empty.
}

RDispositionType::RDispositionType(const QCString & s)
    :    RHeaderBody(s)
{
    // Empty.
}

    RDispositionType &
RDispositionType::operator = (const RDispositionType & t)
{
    if (this == &t) return *this; // Don't do a = a.
    
    parameterList_    = t.parameterList_;
    dispType_        = t.dispType_;
    filename_        = t.filename_;
    
    RHeaderBody::operator = (t);
    
    return *this;
}

    RDispositionType &
RDispositionType::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RDispositionType::operator == (RDispositionType & dt)
{
    parse();
    dt.parse();
    
    return (
        parameterList_    == dt.parameterList_    &&
        dispType_        == dt.dispType_            &&
        filename_        == dt.filename_);
}

RDispositionType::~RDispositionType()
{
    // Empty.
}

    RMM::DispType
RDispositionType::type()
{
    parse();
    return dispType_;
}

    QCString
RDispositionType::filename()
{
    parse();
    return filename_;
}


    void
RDispositionType::setFilename(const QCString & s)
{
    parse();
    filename_ = s;
}

    void
RDispositionType::_parse()
{
    // STUB
}

    void
RDispositionType::_assemble()
{
    // STUB
}
    
    void
RDispositionType::createDefault()
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
