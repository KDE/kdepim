/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rikkus@postmaster.co.uk
	
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

#include <RMM_Token.h>
#include <RMM_Defines.h>
#include <RMM_Message.h>
#include <RMM_Envelope.h>
#include <RMM_Body.h>

RMessage::RMessage()
{
	rmmDebug("ctor");
}

RMessage::RMessage(const RMessage & m)
	:	REntity()
{
	rmmDebug("ctor");
}

RMessage::~RMessage()
{
	rmmDebug("dtor");
}

	const RMessage &
RMessage::operator = (const RMessage & m)
{
	envelope_ = m.envelope_;
	body_ = m.body_;
	return *this;
}

	REnvelope *
RMessage::envelope()
{
	return &envelope_;
}

	RBody *
RMessage::body()
{
	return &body_;
}

	int
RMessage::numberOfParts() const
{
	return body_.numberOfParts();
}

	void
RMessage::addPart(RBodyPart * bp)
{
	body_.addPart(bp);
	_update();
}

	void
RMessage::removePart(RBodyPart * part)
{
	body_.removePart(part);
	_update();
}

	void
RMessage::_update()
{
	type_ = (body_.numberOfParts() == 1) ? BasicMessage : MimeMessage;
}

	RMessage::MessageType
RMessage::type() const
{
	return type_;
}

	RBodyPart *
RMessage::part(int index)
{
	return body_.part(index);
}

	void
RMessage::parse()
{
	rmmDebug("parse() called");

//	int endOfHeaders = strRep_.find("\n[ \t]*[\r]\n");
	int endOfHeaders = strRep_.find("^$");
	
	if (endOfHeaders == -1) {
		rmmDebug("No end of headers !");
		return;
	}
	
	envelope_.set(strRep_.left(endOfHeaders));
	envelope_.parse();
	

	body_.set(strRep_.right(strRep_.length() - endOfHeaders));

	body_.parse();
}

	void
RMessage::assemble()
{
	rmmDebug("assemble() called");

	_update();

	envelope_.assemble();

	strRep_ = envelope_.asString();

	if (type_ == BasicMessage) {
		
		strRep_ += body_.asString();
	
	} else {
		
		strRep_ += body_.asString();
	}
}

	void
RMessage::createDefault()
{
	envelope_.createDefault();
	body_.createDefault();
}

