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

VersionSpec::VersionSpec()
	:	Entity	(),
		number_	(0)
{
}

VersionSpec::VersionSpec(const VersionSpec & x)
	:	Entity	(x),
		number_	(0)
{
}

VersionSpec::VersionSpec(const QCString & s)
	:	Entity	(s),
		number_	(0)
{
}

	VersionSpec &
VersionSpec::operator = (VersionSpec & x)
{
	if (*this == x) return *this;
	
	x.parse();
	number_ = x.number_;

	Entity::operator = (x);
	return *this;
}

	VersionSpec &
VersionSpec::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
VersionSpec::operator == (VersionSpec & x)
{
	x.parse();
	return (number_ == x.number_);
}

VersionSpec::~VersionSpec()
{
}

	void
VersionSpec::_parse()
{
	int split = strRep_.find(':');
	
	if (split == -1) {
		number_ = 0; // Invalid. Use version number 0.
		return;
	}
	
	number_ = strRep_.mid(split + 1).toInt();
}

	void
VersionSpec::_assemble()
{
	strRep_ = "version: " + QCString().setNum(number_);
}

