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

#ifdef __GNUG__
# pragma implementation "RMM_ParameterList.h"
#endif

#include <qstring.h>
#include <qstrlist.h>
#include <qlist.h>

#include <RMM_Parameter.h>
#include <RMM_ParameterList.h>
#include <RMM_Token.h>

RParameterList::RParameterList()
	:	QList<RParameter>(),
		RHeaderBody()
{
	rmmDebug("ctor");
}

RParameterList::RParameterList(const RParameterList & l)
	:	QList<RParameter>(l),
		RHeaderBody(l)
{
	rmmDebug("ctor");
}

RParameterList::RParameterList(const QCString & s)
	:	QList<RParameter>(),
		RHeaderBody(s)
{
	rmmDebug("ctor");
}


RParameterList::~RParameterList()
{
	rmmDebug("dtor");
}

	RParameterList &
RParameterList::operator = (const RParameterList & l)
{
	if (this == &l) return *this;
	QList<RParameter>::operator = (l);
	assembled_ = false;
	return *this;
}

	RParameterList &
RParameterList::operator = (const QCString & s)
{
	strRep_ = s;
	parsed_ = false;
	return *this;
}

	bool
RParameterList::operator == (RParameterList & l)
{
	parse();
	l.parse();

	return false; // XXX: Write this
}

	void
RParameterList::_parse()
{
	clear();
	
	QStrList l;
	RTokenise(strRep_, ";", l, true, false);
	
	QStrListIterator it(l);
	
	for (; it.current(); ++it) {
		
		RParameter * p =
			new RParameter(QCString(it.current()).stripWhiteSpace());
		p->parse();
		QList::append(p);
	}
}

	void
RParameterList::_assemble()
{
	bool firstTime = true;
	
	RParameterListIterator it(*this);

	strRep_ = "";
	
	for (; it.current(); ++it) {
		
		it.current()->assemble();
		
		if (!firstTime) {
			strRep_ += QCString(";\n    ");
			firstTime = false;
		}

		strRep_ += it.current()->asString();
	}
}

	void
RParameterList::createDefault()
{
	rmmDebug("createDefault() called");
}

