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
#include <rmm/Token.h>
#include <rmm/Parameter.h>

using namespace RMM;

Parameter::Parameter()
    :    MessageComponent()
{
    // Empty.
}

Parameter::Parameter(const Parameter & p)
    :   MessageComponent   (p),
        attribute_          (p.attribute_.copy()),
        value_              (p.value_.copy())
{
    // Empty.
}

Parameter::Parameter(const QCString & s)
    :    MessageComponent(s)
{
    // Empty.
}

Parameter::~Parameter()
{
    // Empty.
}

    Parameter &
Parameter::operator = (const QCString & s)
{
    MessageComponent::operator = (s);
    return *this;
}

    Parameter &
Parameter::operator = (const Parameter & p)
{
    if (this == &p) return *this; // Don't do a = a.
    
    attribute_  = p.attribute_.copy();
    value_      = p.value_.copy();

    MessageComponent::operator = (p);
    return *this;
}

    bool
Parameter::operator == (Parameter & p)
{
    parse();
    p.parse();

    return (
        attribute_    == p.attribute_ &&
        value_        == p.value_);
}
    
    void
Parameter::_parse()   
{
    int split = strRep_.find('=');
    
    if (split == -1) {
        rmmDebug("Invalid parameter `" + strRep_ + "'");
        return;
    }
    
    attribute_    = strRep_.left(split).stripWhiteSpace();
    value_        = strRep_.right(strRep_.length() - split - 1).stripWhiteSpace();
    
//    rmmDebug("attribute == \"" + attribute_ + "\"");
//    rmmDebug("value     == \"" + value_ + "\"");
}

    void
Parameter::_assemble()
{
    strRep_ = attribute_ + "=" + value_;
}

    void
Parameter::createDefault()
{
    attribute_ = value_ = "";
}

    QCString
Parameter::attribute()
{
    parse();
    return attribute_;
}

    QCString
Parameter::value()
{
    parse();
    return value_;
}

    void
Parameter::setAttribute(const QCString & s)
{
    parse();
    attribute_ = s.copy();
}
    void
Parameter::setValue(const QCString & s)
{
    parse();
    value_ = s.copy();
}

// vim:ts=4:sw=4:tw=78
