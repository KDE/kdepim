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
# pragma implementation "RMM_HeaderBody.h"
#endif

#include <qstring.h>
#include <RMM_HeaderBody.h>
#include <RMM_Defines.h>

using namespace RMM;

RHeaderBody::RHeaderBody()
    :    RMessageComponent()
{
    // Empty.
}

RHeaderBody::RHeaderBody(const RHeaderBody & headerBody)
    :    RMessageComponent(headerBody)
{
    // Empty.
}

RHeaderBody::RHeaderBody(const QCString & s)
    :    RMessageComponent(s)
{
    // Empty.
}

    RHeaderBody &
RHeaderBody::operator = (const RHeaderBody & hb)
{
    if (this == &hb) return *this;
    
    strRep_ = hb.strRep_;    
    
    RMessageComponent::operator = (hb);
    return *this;
}

    RHeaderBody &
RHeaderBody::operator = (const QCString & s)
{
    RMessageComponent::operator = (s);
    return *this;
}

    bool
RHeaderBody::operator == (RHeaderBody & hb)
{
    return (RMessageComponent::operator == (hb));
}


RHeaderBody::~RHeaderBody()
{
    // Empty.
}

    void
RHeaderBody::_parse()
{
    rmmDebug("WARNING PARSE CALLED");
}

    void
RHeaderBody::_assemble()
{
    rmmDebug("WARNING ASSEMBLE CALLED");
}

    void
RHeaderBody::createDefault()
{
    rmmDebug("WARNING CREATEDEFAULT CALLED");
}
// vim:ts=4:sw=4:tw=78
