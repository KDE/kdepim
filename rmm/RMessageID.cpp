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
#include <qregexp.h>
#include <qstrlist.h>
#include <RMM_MessageID.h>
#include <RMM_Token.h>

RMessageID::RMessageID()
	:	RHeaderBody()
{
	rmmDebug("ctor");
}

RMessageID::RMessageID(const RMessageID & messageID)
	:	RHeaderBody(),
		localPart_(messageID.localPart_.data()),
		domain_(messageID.domain_.data())
{
	rmmDebug("ctor");
}

RMessageID::~RMessageID()
{
	rmmDebug("dtor");
}

	bool
RMessageID::operator == (RMessageID & msgID)
{
	msgID.assemble();
	return (
		localPart_	== msgID.localPart_ &&
		domain_		== msgID.domain_);
}

	bool
RMessageID::operator != (RMessageID & msgID)
{
	msgID.assemble();
	return (
		localPart_	!= msgID.localPart_ ||
		domain_		!= msgID.domain_);
}


	RMessageID &
RMessageID::operator = (const RMessageID & messageID)
{
	rmmDebug("operator =");
    if (this == &messageID) return *this; // Avoid a = a
	
	localPart_ = messageID.localPart_;
    domain_ = messageID.domain_;
    
	rmmDebug("operator = ...");
	rmmDebug("localPart_ == " + localPart_);
	rmmDebug("domain_ == " + domain_);
	
	RHeaderBody::operator = (messageID);
	
	assembled_ = false;
	return *this;
}

	QDataStream &
operator >> (QDataStream & s, RMessageID & mid)
{
	s	>> mid.localPart_
		>> mid.domain_;
	mid.assembled_ = false;
	return s;
}
		
	QDataStream &
operator << (QDataStream & s, RMessageID & mid)
{
	mid.parse();
	s	<< mid.localPart_
		<< mid.domain_;
	return s;
}
	
	QCString
RMessageID::localPart()
{
	parse();
    return localPart_;
}

	void
RMessageID::setLocalPart(const QCString & localPart)
{
	localPart_ = localPart;
	assembled_ = false;
}

	QCString
RMessageID::domain()
{
	parse();
    return domain_;
}

	void
RMessageID::setDomain(const QCString & domain)
{
    domain_ = domain;
	assembled_ = false;
}

	void
RMessageID::parse()
{
	rmmDebug("parse() called");
	if (parsed_) return;
	if (strRep_.isEmpty()) {
		rmmDebug("But there's nothing to parse !");
		return;
	}
	
	QStrList l;
	int ntokens = RTokenise(strRep_, "@", l);
	if (ntokens < 2) return;
	localPart_ = l.at(0);
	QCString b = l.at(1);
	localPart_.replace(QRegExp("^<"), "");
	domain_ = b.left(b.findRev('>'));
	rmmDebug("Done parse");
	parsed_ = true;
	assembled_ = false;
}

	void
RMessageID::assemble()
{
	rmmDebug("assemble() called");
	if (assembled_) return;
	strRep_ = "<" + localPart_ + "@" + domain_ + ">";
	assembled_ = true;
}

	void
RMessageID::createDefault()
{
	rmmDebug("createDefault() called");
	assembled_ = false;
}

