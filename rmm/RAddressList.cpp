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

/*
	RMailMessages -	Class library for Internet Mail Messages.
					This library relies on the Qt toolkit (http://www.troll.no).
					Compliant with various RFCs. See docs for details.
	
	Copyright (C) 1998 Rik Hemsley <rik@kde.org>
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
 
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.
 
	You should have received a copy of the GNU General Public License
	along with this library; see the file COPYING.  If not, write to
	the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA. 
*/

#include <qlist.h>

#include <RMM_Address.h>
#include <RMM_AddressList.h>
#include <RMM_HeaderBody.h>
#include <RMM_Group.h>
#include <RMM_Mailbox.h>
#include <RMM_Token.h>

RAddressList::RAddressList()
{
	rmmDebug("ctor");
	setAutoDelete(true);
}


RAddressList::RAddressList(const RAddressList & list)
	:	QList<RAddress>(list),
		RHeaderBody()
{
	rmmDebug("ctor");
	// TODO: Copy elements from list into us.
	setAutoDelete(true);
}

RAddressList::~RAddressList()
{
	rmmDebug("dtor");
}
		

	const RAddressList &
RAddressList::operator = (const RAddressList & al)
{
	rmmDebug("operator =");
    if (this == &al) return *this; // Don't do a = a.
	
	QList<RAddress>::operator = (al);
	RHeaderBody::operator = (al);

	return *this;
}

	void
RAddressList::parse()
{
	rmmDebug("parse() called");
	rmmDebug("strRep_ = " + strRep_);

	clear();

	QStrList l;
	RTokenise(strRep_, ",\n\r", l);

//	rmmDebug("Found " + QString().setNum(l.count()) + " tokens");

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
	
	rmmDebug("Done my news");

	RAddressListIterator it(*this);

	for (; it.current(); ++it)
		it.current()->parse();
}

	void
RAddressList::assemble()
{
	rmmDebug("assemble() called");

	bool firstTime = true;
	
	RAddressListIterator it(*this);

	strRep_ = QString::null;
	
	for (; it.current(); ++it) {
		
		it.current()->assemble();
		
		if (!firstTime) {
			strRep_ += QCString(",\n    ");
			firstTime = false;
		}

		strRep_ += it.current()->asString();
	}
	
	isModified_ = false;
	rmmDebug("assembled to: \"" + strRep_ + "\"");
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

