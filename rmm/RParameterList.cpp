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
#include <qlist.h>

#include <RMM_Parameter.h>
#include <RMM_ParameterList.h>
#include <RMM_Token.h>

RParameterList::RParameterList()
{
	rmmDebug("ctor");
	setAutoDelete(true);
}

RParameterList::RParameterList(const RParameterList & l)
	:	QList<RParameter>()
{
	rmmDebug("ctor");
	setAutoDelete(true);
}

RParameterList::~RParameterList()
{
	rmmDebug("dtor");
}

	const RParameterList &
RParameterList::operator = (const RParameterList & l)
{
	if (this == &l) return *this;
	QList<RParameter>::operator = (l);
	return *this;
}

	void
RParameterList::parse()
{
	rmmDebug("parse() called");
	rmmDebug("strRep_ = " + strRep_);

	clear();

	QStrList l;
	RTokenise(strRep_, ";", l);
	
	QStrListIterator it(l);
	
	for (; it.current(); ++it) {
		
		RParameter * p = new RParameter(QCString(it.current()).stripWhiteSpace());
		p->parse();
		QList::append(p);
	}
}

	void
RParameterList::assemble()
{
	rmmDebug("assemble() called");
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
	
	rmmDebug("assembled to: \"" + strRep_ + "\"");
}

	void
RParameterList::createDefault()
{
	rmmDebug("createDefault() called");
}

