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


NameComponent::NameComponent()
	:	Entity()
{
}

NameComponent::NameComponent(const NameComponent & x)
	:	Entity					(x),
		attrTypeAndValueList_	(x.attrTypeAndValueList_)
{
}

NameComponent::NameComponent(const QCString & s)
	:	Entity(s)
{
}

	NameComponent &
NameComponent::operator = (NameComponent & x)
{
	if (*this == x) return *this;

	x.parse();
	attrTypeAndValueList_ = x.attrTypeAndValueList_;

	Entity::operator = (x);
	return *this;
}

	NameComponent &
NameComponent::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
NameComponent::operator == (NameComponent & x)
{
	x.parse();
	// TODO
	return false;
}

NameComponent::~NameComponent()
{
}

	void
NameComponent::_parse()
{
	QStrList l;
	
	RTokenise(strRep_, "+", l);
	
	QStrListIterator it(l);
	
	for (; it.current(); ++it) {
		
		AttrTypeAndValue * tv = new AttrTypeAndValue(it.current());
		
		attrTypeAndValueList_.append(tv);
		
	}
}

	void
NameComponent::_assemble()
{
	AttrTypeAndValueIterator it(attrTypeAndValueList_);
	
	bool firstTime = false;
	
	for (; it.current(); ++it) {
		
		if (!firstTime) {
			strRep_ += '+';
			firstTime = false;
		}
		
		strRep_ += it.current()->asString();
	}
}

