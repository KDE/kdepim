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


LDAPString::LDAPString()
	:	Entity()
{
}

LDAPString::LDAPString(const LDAPString & x)
	:	Entity(x)
{
}

LDAPString::LDAPString(const QCString & s)
	:	Entity(s)
{
}

	LDAPString &
LDAPString::operator = (LDAPString & x)
{
	if (*this == x) return *this;

	Entity::operator = (x);
	return *this;
}

	LDAPString &
LDAPString::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
LDAPString::operator == (LDAPString & x)
{
	x.parse();
	return false;
}

LDAPString::~LDAPString()
{
}

	void
LDAPString::_parse()
{
}

	void
LDAPString::_assemble()
{
}

