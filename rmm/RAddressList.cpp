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

#include <qlist.h>

#include <RMM_Address.h>
#include <RMM_AddressList.h>
#include <RMM_HeaderBody.h>
#include <RMM_Group.h>
#include <RMM_Mailbox.h>
#include <RMM_Token.h>

RAddressList::RAddressList()
	:	QList<RAddress>(),
		RHeaderBody()
{
	rmmDebug("ctor");
	setAutoDelete(true);
	assembled_	= false;
}


RAddressList::RAddressList(const RAddressList & list)
	:	QList<RAddress>(list),
		RHeaderBody()
{
	rmmDebug("ctor");
	setAutoDelete(true);
	assembled_	= false;
}

RAddressList::~RAddressList()
{
	rmmDebug("dtor");
}
		
	RAddressList &
RAddressList::operator = (const RAddressList & al)
{
	rmmDebug("operator =");
    if (this == &al) return *this; // Don't do a = a.
	
	QList<RAddress>::operator	= (al);
	RHeaderBody::operator		= (al);

	assembled_	= false;
	return *this;
}

	void
RAddressList::parse()
{
	rmmDebug("parse() called");
	if (parsed_) return;

	clear();

	QStrList l;
	RTokenise(strRep_, ",\n\r", l);

	if (l.count() == 0 && !strRep_.isEmpty()) { // Lets try what we have then.

		rmmDebug("new RAddress");
		RAddress * a = new RAddress;
		CHECK_PTR(a);
		a->set(strRep_);
		append(a);
		
	} else {

		QStrListIterator lit(l);

		for (; lit.current(); ++lit) {
			rmmDebug("new RAddress");
			RAddress * a = new RAddress;
			CHECK_PTR(a);
			a->set(lit.current());
			append(a);
		}
	}
	
	parsed_		= true;
	assembled_	= false;
}

	void
RAddressList::assemble()
{
	parse();
	if (assembled_) return;

	bool firstTime = true;
	
	RAddressListIterator it(*this);

	strRep_ = "";
	
	for (; it.current(); ++it) {
		
		it.current()->assemble();
		
		if (!firstTime) {
			strRep_ += QCString(",\n    ");
			firstTime = false;
		}

		strRep_ += it.current()->asString();
	}
	
	assembled_ = true;
}


	void
RAddressList::createDefault()
{
	rmmDebug("createDefault() called");
	if (count() == 0) {
		RAddress * a = new RAddress;
		a->createDefault();
		rmmDebug("AAAAAAAPPPENDING a");
		append(a);
	}
}

