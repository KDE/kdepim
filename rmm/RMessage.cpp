/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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

#include <qstring.h>
#include <qstrlist.h>
#include <qregexp.h>

#include <RMM_Token.h>
#include <RMM_Defines.h>
#include <RMM_Message.h>
#include <RMM_Envelope.h>

RMessage::RMessage()
	:	RBodyPart()
{
	rmmDebug("ctor");
}

RMessage::RMessage(const RMessage & m)
	:	RBodyPart(m)
{
	rmmDebug("ctor");
}

RMessage::RMessage(const QCString & s)
	:	RBodyPart(s)
{
	rmmDebug("ctor");
}

RMessage::~RMessage()
{
	rmmDebug("dtor");
}

	RMM::MessageStatus
RMessage::status()
{
	return status_;
}

	void
RMessage::setStatus(RMM::MessageStatus s)
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
	if (this == &m) return *this;	// Avoid a = a.
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

