/*
	libldif - LDAP LDIF parsing library

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

#include <qstrlist.h>
#include <RToken.h>
#include "ldif.h"

using namespace LDIF;


LdifContent::LdifContent()
	:	Entity()
{
}

LdifContent::LdifContent(const LdifContent & x)
	:	Entity(x)
{
}

LdifContent::LdifContent(const QCString & s)
	:	Entity(s)
{
}

	LdifContent &
LdifContent::operator = (LdifContent & x)
{
	if (*this == x) return *this;
	
	x.parse();
	
	attrValRecList_	= x.attrValRecList_;
	versionSpec_	= x.versionSpec_;

	Entity::operator = (x);
	return *this;
}

	LdifContent &
LdifContent::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
LdifContent::operator == (LdifContent & x)
{
	x.parse();
	return (versionSpec_ == x.versionSpec_ /* && ..... */); // TODO
}

LdifContent::~LdifContent()
{
}

	void
LdifContent::_parse()
{
	lDebug("parse() called");
	QStrList l;
	
	RTokenise(strRep_, "\r\n", l);
	
	if (l.count() < 2) {
		// Invalid ldif !
		// Must have version line + at least one AttrValRec.
		lDebug("Invalid ldif");
		return;
	}
	
	versionSpec_ = l.at(0);
	versionSpec_.parse();
	
	// Trash the first (version) line as we have seen it.
	l.remove(0u);

	///////////////////////////////////////////////////////////////
	// AttrValRec lines

	lDebug("AttrValRec lines");
	
	QStrListIterator it(l);
	
	for (; it.current(); ++it) {
		
		lDebug("New AttrValRec using \"" + QCString(it.current()) + "\"");
		LdifAttrValRec * r = new LdifAttrValRec(it.current());

		r->parse();
		
		attrValRecList_.append(r);
	}
}

	void
LdifContent::_assemble()
{
	lDebug("Assembling ldif");

	strRep_ = versionSpec_.asString();
	
	LdifAttrValRecIterator it(attrValRecList_);
	
	for (; it.current(); ++it)
		strRep_ += it.current()->asString() + "\n";
}

