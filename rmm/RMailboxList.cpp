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

#include <ctype.h>
#include <stdlib.h>
#include <iostream.h>

#include <qstring.h>

#include <RMM_Mailbox.h>
#include <RMM_MailboxList.h>
#include <RMM_Token.h>

RMailboxList::RMailboxList()
	:	QList<RMailbox>(),
		RHeaderBody()
		
{
	rmmDebug("ctor");
}

RMailboxList::RMailboxList(const RMailboxList & l)
	:	QList<RMailbox>(l),
		RHeaderBody()
{
	rmmDebug("ctor");
}

RMailboxList::~RMailboxList()
{
	rmmDebug("dtor");
}

	RMailboxList &
RMailboxList::operator = (const RMailboxList & l)
{
	rmmDebug("operator =");
    if (this == &l) return *this; // Don't do a = a.
	QList<RMailbox>::operator = (l);
	RHeaderBody::operator = (l);
	assembled_ = false;
	return *this;
}

	void
RMailboxList::parse()
{
	rmmDebug("parse() called");
	if (parsed_) return;
	// XXX Currently just adapted slightly from RAddressList - adjust further ?
	rmmDebug("strRep_ = " + strRep_);

	clear();

	QStrList ltemp;
	RTokenise(strRep_, ",\n\r", ltemp);
	
	QStrList l;
	
	// Now a little magic. We might have extra tokens that are quoted.
	// If so, we merge them with the following token.
	// Rik Hemsley <rik@kde.org> will be 1 token
	// "Rik Hemsley" <rik@kde.org> will be 2 tokens which should be
	// combined.
	QStrListIterator bit(ltemp);
	
	QCString s;
	
	for (; bit.current(); ++bit) {
		
		if (*(bit.current()) == '"') {
			s = bit.current();
			++bit;
			l.append(s + " " + bit.current());
			
		} else
			l.append(bit.current());
	}

	if (l.count() == 0 && !strRep_.isEmpty()) { // Lets try what we have then.

		rmmDebug("new RMailbox");
		RMailbox * m = new RMailbox;
		CHECK_PTR(m);
		m->set(strRep_);
		append(m);
		
	} else {

		QStrListIterator lit(l);

		for (; lit.current(); ++lit) {
			rmmDebug("new RMailbox");
			RMailbox * m = new RMailbox;
			CHECK_PTR(m);
			m->set(lit.current());
			append(m);
		}
	}
	
	parsed_ = true;
	assembled_ = false;
}

	void
RMailboxList::assemble()
{
	rmmDebug("assemble() called");
	if (assembled_) return;
	// XXX Just ripped from RAddressList - adjust further ?
	bool firstTime = true;
	
	RMailboxListIterator it(*this);

	strRep_ = "";
	
	for (; it.current(); ++it) {
		
		if (!firstTime) {
			strRep_ += QCString(",\n    ");
			firstTime = false;
		}

		strRep_ += it.current()->asString();
	}
	
	rmmDebug("assembled to: \"" + strRep_ + "\"");
	
	assembled_ = true;
}

	void
RMailboxList::createDefault()
{
	rmmDebug("createDefault() called");
	if (count() == 0) {
		RMailbox * m = new RMailbox;
		m->createDefault();
		append(m);
	}
	
	assembled_ = false;
}

