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


Dn::Dn()
	:	Entity()
{
	nameComponentList_.setAutoDelete(true);
}

Dn::Dn(const Dn & x)
	:	Entity				(x),
		encoded_			(x.encoded_),
		nameComponentList_	(x.nameComponentList_)
{
	nameComponentList_.setAutoDelete(true);
}

Dn::Dn(const QCString & s)
	:	Entity(s)
{
	nameComponentList_.setAutoDelete(true);
}

	Dn &
Dn::operator = (Dn & x)
{
	if (*this == x) return *this;

	x.parse();
	encoded_			= x.encoded_;
	nameComponentList_	= x.nameComponentList_;
		
	Entity::operator = (x);
	return *this;
}

	Dn &
Dn::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
Dn::operator == (Dn & x)
{
	x.parse();
	return (encoded_ = x.encoded_ /* && .... */); // TODO
}

Dn::~Dn()
{
}

	void
Dn::_parse()
{
	if (strRep_.isEmpty())
		return; // Invalid.
	
	QCString encoded(strRep_);
	
	if (strRep_[0] == ':') {
	
		// We are base64 encoded.
		encoded.remove(0, 1);
		
		while (encoded[0] == ' ')
			encoded.remove(0, 1);
		
		decodeBase64(encoded);
	}
	
	QStrList l;
	RTokenise(encoded, "+", l);
	QStrListIterator it(l);
	
	for (; it.current(); ++it) {
		NameComponent * n = new NameComponent(it.current());
		nameComponentList_.append(n);
	}
	
	encoded_ = false; // TODO: How to find out ?
}

	void
Dn::_assemble()
{
	strRep_.truncate(0);
	
	NameComponentIterator it(nameComponentList_);
	bool firstTime = true;
	
	for (; it.current(); ++it) {
		
		if (!firstTime) {
			strRep_ += '+';
			firstTime = false;
		}
		
		strRep_ += it.current()->asString();
	}
}

