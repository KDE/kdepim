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

#include "ldif.h"

using namespace LDIF;


AttrTypeAndValue::AttrTypeAndValue()
	:	Entity()
{
}

AttrTypeAndValue::AttrTypeAndValue(const AttrTypeAndValue & x)
	:	Entity	(x),
		type_	(x.type_),
		value_	(x.value_)
{
}

AttrTypeAndValue::AttrTypeAndValue(const QCString & s)
	:	Entity(s)
{
}

	AttrTypeAndValue &
AttrTypeAndValue::operator = (AttrTypeAndValue & x)
{
	if (*this == x) return *this;

	x.parse();
	
	type_	= x.type_;
	value_	= x.value_;
	
	Entity::operator = (x);
	return *this;
}

	AttrTypeAndValue &
AttrTypeAndValue::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
AttrTypeAndValue::operator == (AttrTypeAndValue & x)
{
	x.parse();
	return (type_	== x.type_ &&
			value_	== x.value_);
}

AttrTypeAndValue::~AttrTypeAndValue()
{
}

	void
AttrTypeAndValue::_parse()
{
	int split = strRep_.find('=');
	if (split == -1)
		return; // Invalid.
	
	type_	= strRep_.left(split);
	value_	= strRep_.mid(split + 1);
}

	void
AttrTypeAndValue::_assemble()
{
	strRep_ = type_ + "=" + value_;
}

