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


AttrValSpec::AttrValSpec()
	:	Entity()
{
}

AttrValSpec::AttrValSpec(const AttrValSpec & x)
	:	Entity					(x),
		attributeDescription_	(x.attributeDescription_),
		valueSpec_				(x.valueSpec_)
{
}

AttrValSpec::AttrValSpec(const QCString & s)
	:	Entity(s)
{
}

	AttrValSpec &
AttrValSpec::operator = (AttrValSpec & x)
{
	if (*this == x) return *this;

	x.parse();

	attributeDescription_	= x.attributeDescription_,
	valueSpec_				= x.valueSpec_;

	Entity::operator = (x);
	return *this;
}

	AttrValSpec &
AttrValSpec::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
AttrValSpec::operator == (AttrValSpec & x)
{
	x.parse();
	return (attributeDescription_	== x.attributeDescription_ &&
			valueSpec_				== x.valueSpec_);
}

AttrValSpec::~AttrValSpec()
{
}

	void
AttrValSpec::_parse()
{
	int split = strRep_.find(':');
	if (split == -1)
		return; // Invalid.
	
	attributeDescription_	= strRep_.left(split);
	valueSpec_				= strRep_.mid(split);
	
	attributeDescription_.parse();
	valueSpec_.parse();
}

	void
AttrValSpec::_assemble()
{
	strRep_ = attributeDescription_.asString() + valueSpec_.asString(); 
}

