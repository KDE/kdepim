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
#include <RMM_Mailbox.h>
#include <RMM_Token.h>

RMailbox::RMailbox()
{
	rmmDebug("ctor");
}

RMailbox::RMailbox(const RMailbox & mailbox)
	:	RAddress(),
		phrase_(mailbox.phrase_),
		route_(mailbox.route_),
		localPart_(mailbox.localPart_),
		domain_(mailbox.domain_)
{
	rmmDebug("copy ctor");
}

RMailbox::~RMailbox()
{
	rmmDebug("dtor");
}

const RMailbox & RMailbox::operator = (const RMailbox & mailbox)
{
	rmmDebug("operator =");
	if (this == &mailbox) return *this; // Avoid a = a
	
	phrase_		= mailbox.phrase_;
	route_		= mailbox.route_;
	localPart_	= mailbox.localPart_;
	domain_		= mailbox.domain_;
	
	RAddress::operator = (mailbox);
	
	return *this;
}

	QDataStream &
operator >> (QDataStream & s, RMailbox & mailbox)
{
	s >> mailbox.phrase_;
	s >> mailbox.route_;
	s >> mailbox.localPart_;
	s >> mailbox.domain_;
	return s;
}
	
	QDataStream &
operator << (QDataStream & s, const RMailbox & mailbox)
{
	s << mailbox.phrase_;
	s << mailbox.route_;
	s << mailbox.localPart_;
	s << mailbox.domain_;
	return s;
}


	const QCString &
RMailbox::phrase() const
{
	return phrase_;
}


	void
RMailbox::setPhrase(const QCString & s)
{
	phrase_ = s.data();
}

	const QCString &
RMailbox::route() const
{
	return route_;
}

	void
RMailbox::setRoute(const QCString & s)
{
	route_ = s.data();
}

	const QCString &
RMailbox::localPart() const
{
	return localPart_;
}

	void
RMailbox::setLocalPart(const QCString & s)
{
	localPart_ = s.data();
}


	const QCString &
RMailbox::domain() const
{
	return domain_;
}

	void
RMailbox::setDomain(const QCString & s)
{
	domain_ = s.data();
}

void RMailbox::parse()
{
	rmmDebug("parse() called");
	rmmDebug("strRep == \"" + strRep_ + "\"");

	if (strRep_.find('@') == -1) { // Must contain '@' somewhere. (RFC822)
		rmmDebug("This is NOT a valid mailbox");
		return;
	}
	
	rmmDebug("It's a valid mailbox (probably). Tokenising");

	QStrList l;
	RTokenise(strRep_, " \n", l, true);

	QStrListIterator it(l);
	bool hasRouteAddress = false;
	for (; it.current(); ++it) {
		if (*(it.current()) == '<') hasRouteAddress = true;
	}
	
	if (hasRouteAddress) { // It's phrase route-addr
		rmmDebug("phrase route-addr");

		// Deal with the phrase part. Just put in a string.
		phrase_ = "";
		int i = 0;
		QCString s = l.at(i++);
		
		// We're guaranteed to hit '<' since hasRouteAddress == true.
		while (s.at(0) != '<') {
			phrase_ += s;
			s = l.at(i++);
			if (s.at(0) != '<')
				phrase_ += ' ';
		}
		--i;
		
		phrase_ = phrase_.stripWhiteSpace();

		// So by now we are left with only the route part.
		route_ = "";

		for (Q_UINT32 n = i; n < l.count(); n++) {
			route_ += l.at(n);
			if (n + 1 < l.count()) route_ += ' ';
		}

	} else { // It's just addr-spec
		
		rmmDebug("addr-spec");
		
		while (strRep_.at(0) == '<')
			strRep_.remove(0, 1);
	
		while (strRep_.at(strRep_.length()) == '>')
			strRep_.remove(strRep_.length(), 1);

		// Re-use l. It's guaranteed to be cleared by RTokenise.
		RTokenise(strRep_, "@", l);
		
		rmmDebug("done tokenise");

		localPart_ = l.at(0);
		localPart_ = localPart_.stripWhiteSpace();
		if (l.count() == 2) {
			domain_ = l.at(1);
			domain_ = domain_.stripWhiteSpace();
		} else domain_ = "";

		// Easy, eh ?
	}
	dirty_ = true;
	rmmDebug("phrase: \"" + phrase_ + "\"");
	rmmDebug("route: \"" + route_ + "\"");
	rmmDebug("localpart: \"" + localPart_ + "\"");
	rmmDebug("domain: \"" + domain_ + "\"");
}


void RMailbox::assemble()
{
	strRep_ = "";
	rmmDebug("assemble() called");
	if (localPart_.isEmpty()) // This is 'phrase route-addr' style
		strRep_ = phrase_ + " " + route_;
	else
		strRep_ = localPart_ + "@" + domain_;
	rmmDebug("assembled to \"" + strRep_ + "\""); 
}

	bool
RMailbox::isValid() const
{
	return isValid_;
}

	void
RMailbox::createDefault()
{
	rmmDebug("createDefault() called");
	phrase_ = "";
	route_ = "";
	localPart_ = "foo";
	domain_ = "bar";
	strRep_ = "<foo@bar>";
}

