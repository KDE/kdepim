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
#include <RMM_Mailbox.h>
#include <RMM_Token.h>

RMailbox::RMailbox()
	:	RAddress()
{
	rmmDebug("ctor");
}

RMailbox::RMailbox(const RMailbox & mailbox)
	:	RAddress(mailbox),
		phrase_		(mailbox.phrase_),
		route_		(mailbox.route_),
		localPart_	(mailbox.localPart_),
		domain_		(mailbox.domain_)
{
	rmmDebug("copy ctor");
}

RMailbox::~RMailbox()
{
	rmmDebug("dtor");
}

RMailbox::RMailbox(const QCString & s)
	:	RAddress(s)
{
	rmmDebug("ctor");
}

	RMailbox &
RMailbox::operator = (const RMailbox & mailbox)
{
	rmmDebug("operator =");
	if (this == &mailbox) return *this; // Avoid a = a
	
	phrase_		= mailbox.phrase_;
	route_		= mailbox.route_;
	localPart_	= mailbox.localPart_;
	domain_		= mailbox.domain_;
	
	RAddress::operator = (mailbox);
	
	assembled_	= false;
	return *this;
}

	RMailbox &
RMailbox::operator = (const QCString & s)
{
	rmmDebug("operator = (" + s + ")");
	
	RAddress::operator = (s);
	
	assembled_	= false;
	return *this;
}

	bool
RMailbox::operator == (RMailbox & m)
{
	parse();
	m.parse();

	return (
		phrase_ 	== m.phrase_	&&
		route_		== m.route_		&&
		localPart_	== m.localPart_	&&
		domain_		== m.domain_);
}

	QDataStream &
operator >> (QDataStream & s, RMailbox & mailbox)
{
	s	>> mailbox.phrase_
		>> mailbox.route_
		>> mailbox.localPart_
		>> mailbox.domain_;
	mailbox.parsed_		= true;
	mailbox.assembled_	= false;
	return s;
}
	
	QDataStream &
operator << (QDataStream & s, RMailbox & mailbox)
{
	mailbox.parse();
	s	<< mailbox.phrase_
		<< mailbox.route_
		<< mailbox.localPart_
		<< mailbox.domain_;
	return s;
}


	QCString
RMailbox::phrase()
{
	parse();
	return phrase_;
}


	void
RMailbox::setPhrase(const QCString & s)
{
	phrase_ = s.data();
	assembled_ = false;
}

	QCString
RMailbox::route()
{
	parse();
	return route_;
}

	void
RMailbox::setRoute(const QCString & s)
{
	route_ = s.data();
	assembled_ = false;
}

	QCString
RMailbox::localPart()
{
	parse();
	return localPart_;
}

	void
RMailbox::setLocalPart(const QCString & s)
{
	localPart_ = s.data();
	assembled_ = false;
}


	QCString
RMailbox::domain()
{
	parse();
	return domain_;
}

	void
RMailbox::setDomain(const QCString & s)
{
	domain_ = s.data();
	assembled_ = false;
}

	void
RMailbox::_parse()
{
	if (strRep_.find('@') == -1) { // Must contain '@' somewhere. (RFC822)
		rmmDebug("This is NOT a valid mailbox");
		return;
	}
	
	rmmDebug("It's a valid mailbox (probably). Tokenising");

	QStrList l;
	RTokenise(strRep_, " \n", l, true);

	bool hasRouteAddress(false);

	QStrListIterator it(l);

	for (; it.current(); ++it) {
		rmmDebug("TOKEN: " + QCString(it.current()));
		hasRouteAddress = (*(it.current()) == '<');
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
			rmmDebug("Phrase now: " + phrase_);
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
	rmmDebug("phrase:    \""	+ phrase_		+ "\"");
	rmmDebug("route:     \""	+ route_		+ "\"");
	rmmDebug("localpart: \""	+ localPart_	+ "\"");
	rmmDebug("domain:    \""	+ domain_		+ "\"");
}


	void
RMailbox::_assemble()
{
	strRep_ = "";
	rmmDebug("assemble() called");
	if (localPart_.isEmpty()) // This is 'phrase route-addr' style
		strRep_ = phrase_ + " " + route_;
	else
		strRep_ = localPart_ + "@" + domain_;
	rmmDebug("assembled to \"" + strRep_ + "\""); 
}

	void
RMailbox::createDefault()
{
	rmmDebug("createDefault() called");
	phrase_		= "";
	route_		= "";
	localPart_	= "foo";
	domain_		= "bar";
	strRep_		= "<foo@bar>";
	
	assembled_ = false;
}

