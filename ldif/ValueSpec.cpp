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


ValueSpec::ValueSpec()
	:	Entity()
{
}

ValueSpec::ValueSpec(const ValueSpec & x)
	:	Entity		(x),
		valueType_	(x.valueType_),
		value_		(x.value_)
{
}

ValueSpec::ValueSpec(const QCString & s)
	:	Entity(s)
{
}

	ValueSpec &
ValueSpec::operator = (ValueSpec & x)
{
	if (*this == x) return *this;

	x.parse();
	
	valueType_	= x.valueType_;
	value_		= x.value_;
		
	Entity::operator = (x);
	return *this;
}

	ValueSpec &
ValueSpec::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
ValueSpec::operator == (ValueSpec & x)
{
	x.parse();

	return (valueType_	== x.valueType_ &&
			value_		== x.value_);
}

ValueSpec::~ValueSpec()
{
}

	void
ValueSpec::_parse()
{
	QCString s(strRep_);
	
	if (s.at(0) != ':')
		return; // Invalid.
	
	s.remove(0, 1);
	
	char c = s.at(0);
	
	switch (c) {
		case ' ':	default:	valueType_ = Plain;		break;
		case ':':				valueType_ = Base64;	break;
		case '<':				valueType_ = URL;		break;
	}
	
	s.remove(0, 1);
	
	while (s.at(0) == ' ')
		strRep_.remove(0, 1);
	
	while (s.at(s.length() - 1) == '\n')
		s.truncate(s.length() - 1);
	
	while (s.at(s.length() - 1) == '\r')
		s.truncate(s.length() - 1);
	
	value_ = s;
}

	void
ValueSpec::_assemble()
{
	strRep_ = ':';

	switch (valueType_) {
		case Plain:		default:	strRep_ += ' ';		break;
		case Base64:				strRep_ += ": ";	break;
		case URL:					strRep_ += "< ";	break;
	}
	
	strRep_ += value_;
	strRep_ += '\n';
}

