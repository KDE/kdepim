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
#include <RMM_Message.h>

using namespace RMM;

RMessage::RMessage()
    :    RBodyPart()
{
    // Empty.
}

RMessage::RMessage(const RMessage & m)
    :    RBodyPart(m)
{
    // Empty.
}

RMessage::RMessage(const QCString & s)
    :    RBodyPart(s)
{
    // Empty.
}

RMessage::~RMessage()
{
    // Empty.
}

    MessageStatus
RMessage::status()
{
    return status_;
}

    void
RMessage::setStatus(MessageStatus s)
{
    status_ = s;
}

    QDataStream &
operator << (QDataStream & str, RMessage & m)
{
    str << m.asString(); return str;
}

    RMessage &
RMessage::operator = (const RMessage & m)
{
    if (this == &m) return *this;    // Avoid a = a.
    RBodyPart::operator = (m);
    return *this;
}

    RMessage &
RMessage::operator = (const QCString & s)
{
    RBodyPart::operator = (s);
    return *this;
}

    bool
RMessage::operator == (RMessage & m)
{
    parse();
    m.parse();

    return (RBodyPart::operator == (m));
}

    void
RMessage::_parse()
{
    RBodyPart::_parse();
}

    void
RMessage::_assemble()
{
    RBodyPart::_assemble();
}

    void
RMessage::createDefault()
{
    RBodyPart::createDefault();
}

    void
RMessage::addPart(RBodyPart &)
{
    // STUB
}

    void
RMessage::removePart(RBodyPart &)
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
