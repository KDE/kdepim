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

#include "RToken.h"
#include "ldif.h"

using namespace LDIF;


LdifAttrValRec::LdifAttrValRec()
	:	Entity()
{
}

LdifAttrValRec::LdifAttrValRec(const LdifAttrValRec & x)
	:	Entity			(x),
		dnSpec_			(x.dnSpec_),
		attrValSpecList_(x.attrValSpecList_)
{
}

LdifAttrValRec::LdifAttrValRec(const QCString & s)
	:	Entity(s)
{
}

	LdifAttrValRec &
LdifAttrValRec::operator = (LdifAttrValRec & x)
{
	if (*this == x) return *this;

	x.parse();
	
	dnSpec_				= x.dnSpec_;
	attrValSpecList_	= x.attrValSpecList_;
		
	Entity::operator = (x);
	return *this;
}

	LdifAttrValRec &
LdifAttrValRec::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
LdifAttrValRec::operator == (LdifAttrValRec & x)
{
	x.parse();
	
	return (dnSpec_ == x.dnSpec_ /* && .... */); // TODO
}

LdifAttrValRec::~LdifAttrValRec()
{
}

	void
LdifAttrValRec::_parse()
{
	QStrList l;
	
	RTokenise(strRep_, "\r\n", l);
	
	if (l.count() == 0)
		return; // Invalid.

	dnSpec_ = l.at(0);
	
	l.remove(0u); // Done with that one.
	
	QStrListIterator it(l);
	
	for (; it.current(); ++it) {
		
		AttrValSpec * s = new AttrValSpec(it.current());
		
		attrValSpecList_.append(s);
	}
}

	void
LdifAttrValRec::_assemble()
{
	strRep_ = dnSpec_.asString() + '\n';
	
	AttrValSpecIterator it(attrValSpecList_);
	
	for (; it.current(); ++it)
		strRep_ += it.current()->asString() + '\n';
}

