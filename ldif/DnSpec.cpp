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


DnSpec::DnSpec()
	:	Entity()
{
}

DnSpec::DnSpec(const DnSpec & x)
	:	Entity	(x),
		dn_		(x.dn_)
{
}

DnSpec::DnSpec(const QCString & s)
	:	Entity(s)
{
}

	DnSpec &
DnSpec::operator = (DnSpec & x)
{
	if (*this == x) return *this;
	
	x.parse();
	dn_	 =x.dn_;
	
	Entity::operator = (x);
	return *this;
}

	DnSpec &
DnSpec::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
DnSpec::operator == (DnSpec & x)
{
	x.parse();
	
	return dn_ == x.dn_;
}

DnSpec::~DnSpec()
{
}

	void
DnSpec::_parse()
{
	dn_ = strRep_.mid(3).stripWhiteSpace();
}

	void
DnSpec::_assemble()
{
	strRep_ = "dn: " + dn_.asString();
}

