/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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

using namespace RMM;

RHeaderBody::RHeaderBody()
    :    RMessageComponent()
{
    rmmDebug("ctor");
}

RHeaderBody::RHeaderBody(const RHeaderBody & headerBody)
    :    RMessageComponent(headerBody)
{
    rmmDebug("ctor");
}

RHeaderBody::RHeaderBody(const QCString & s)
    :    RMessageComponent(s)
{
    rmmDebug("ctor");
}

    RHeaderBody &
RHeaderBody::operator = (const RHeaderBody & hb)
{
    rmmDebug("operator =");
    if (this == &hb) return *this;
    
    strRep_ = hb.strRep_;    
    
    RMessageComponent::operator = (hb);
    assembled_    = false;
    return *this;
}

    RHeaderBody &
RHeaderBody::operator = (const QCString & s)
{
    rmmDebug("operator =");
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
    rmmDebug("dtor");
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
