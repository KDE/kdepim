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

#include <RMM_Defines.h>
#include <RMM_MessageComponent.h>

using namespace RMM;

RMessageComponent::RMessageComponent()
    :   parsed_     (false),
        assembled_  (false)
{
    // Empty.
}

RMessageComponent::RMessageComponent(const RMessageComponent & mc)
    :   strRep_     (mc.strRep_),
        parsed_     (mc.parsed_),
        assembled_  (mc.assembled_)
{
    // Empty.
}

RMessageComponent::RMessageComponent(const QCString & s)
    :   strRep_(s),
        parsed_(false),
        assembled_(false)
{
    // Empty.
}

RMessageComponent::~RMessageComponent()
{
    // Empty.
}

    RMessageComponent &
RMessageComponent::operator = (const RMessageComponent & m)
{
    if (this == &m) return *this;    // Avoid a = a.
    assembled_  = m.assembled_;
    parsed_     = m.parsed_;
    strRep_     = m.strRep_;
    return *this;
}

    RMessageComponent &
RMessageComponent::operator = (const QCString & s)
{
    strRep_     = s;
    parsed_     = false;
    assembled_  = false;
    return *this;
}

    bool
RMessageComponent::operator == (RMessageComponent & mc)
{
    assemble();
    return (strRep_ == mc.asString());
}

    bool
RMessageComponent::operator == (const QCString & s)
{
    parse();
    return (strRep_ == s);
}

// vim:ts=4:sw=4:tw=78
