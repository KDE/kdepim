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
# pragma implementation "RMM_Text.h"
#endif

#include <qstring.h>

#include <RMM_Text.h>

using namespace RMM;

RText::RText()
    :    RHeaderBody()
{
    rmmDebug("ctor");
    parsed_ = assembled_ = true;
}

RText::RText(const RText & r)
    :    RHeaderBody(r)
{
    parsed_ = assembled_ = true;
}

RText::RText(const QCString & s)
       :    RHeaderBody(s)
{
    rmmDebug("ctor with \"" + s + "\"");
    parsed_ = assembled_ = true;
}

RText::~RText()
{
    rmmDebug("dtor");
}

    RText &
RText::operator = (const RText & r)
{
    if (this == &r) return *this; // Avoid a = a
    RHeaderBody::operator = (r);
    return *this;
}

    RText &
RText::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RText::operator == (RText & t)
{
    parse();
    t.parse();

    return (RHeaderBody::operator == (t));
}

    void
RText::_parse()
{
}

    void
RText::_assemble()
{
}

    void
RText::createDefault()
{
    rmmDebug("createDefault() called");
}

// vim:ts=4:sw=4:tw=78
