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

#include <RMM_ContentDisposition.h>
#include <RMM_Token.h>

using namespace RMM;

RContentDisposition::RContentDisposition()
    :    RHeaderBody()
{
    // Empty.
}

RContentDisposition::RContentDisposition(const RContentDisposition & t)
    :    RHeaderBody(t)
{
    // Empty.
}

RContentDisposition::RContentDisposition(const QCString & s)
    :    RHeaderBody(s)
{
    // Empty.
}

    RContentDisposition &
RContentDisposition::operator = (const RContentDisposition & t)
{
    if (this == &t) return *this; // Don't do a = a.
    
    parameterList_    = t.parameterList_;
    dispType_        = t.dispType_;
    filename_        = t.filename_.copy();
    
    RHeaderBody::operator = (t);
    
    return *this;
}

    RContentDisposition &
RContentDisposition::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RContentDisposition::operator == (RContentDisposition & dt)
{
    parse();
    dt.parse();
    
    return (
        parameterList_    == dt.parameterList_    &&
        dispType_        == dt.dispType_            &&
        filename_        == dt.filename_);
}

RContentDisposition::~RContentDisposition()
{
    // Empty.
}

    RMM::DispType
RContentDisposition::type()
{
    parse();
    return dispType_;
}

    QCString
RContentDisposition::filename()
{
    parse();
    return filename_;
}


    void
RContentDisposition::setFilename(const QCString & s)
{
    parse();
    filename_ = s.copy();
}

    void
RContentDisposition::_parse()
{
    QCString dispTypeAsString;

    int firstSeparatorPos = strRep_.find(';');

    if (-1 != firstSeparatorPos)
        parameterList_ = strRep_.mid(firstSeparatorPos + 1);

    // Unrecognized parameters should be ignored. Unrecognized disposition
    // types should be treated as `attachment'. 

    if (dispTypeAsString.isEmpty())
        dispType_ = DispositionTypeInline;
    else if (0 == stricmp(dispTypeAsString,"inline"))
        dispType_ = DispositionTypeInline;
    else
        dispType_ = DispositionTypeAttachment;

    // Get the filename - we care about it !

    QValueList<RParameter> l(parameterList_.list());

    for (QValueList<RParameter>::Iterator it(l.begin()); it != l.end(); ++it)

        if (0 == stricmp((*it).attribute(), "filename")) {

            filename_ = (*it).value();
            break;
        }
}

    void
RContentDisposition::_assemble()
{
	strRep_ = (dispType_==DispositionTypeAttachment) ? "attachment" : "inline";
	strRep_+=parameterList_.asString();    
}
    
    void
RContentDisposition::createDefault()
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
