/*
	libvcard - vCard parsing library for vCard version 3.0

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

#include <VCardParam.h>

#include <VCardEntity.h>

#include <VCardRToken.h>

using namespace VCARD;

Param::Param()
	:	Entity()
{
}

Param::Param(const Param & x)
	:	Entity(x)
{
}

Param::Param(const QCString & s)
	:	Entity(s)
{
}

	Param &
Param::operator = (Param & x)
{
	if (*this == x) return *this;

	Entity::operator = (x);
	return *this;
}

	Param &
Param::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
Param::operator == (Param & x)
{
	x.parse();
	return false;
}

Param::~Param()
{
}

	void
Param::_parse()
{
}

	void
Param::_assemble()
{
}

	void
Param::parseToList()
{
	QStrList l;
	RTokenise(strRep_, ",", l);
	
	QStrListIterator it(l);
	
	for (; it.current(); ++it) {
		
		QCString s(it.current());
		
		int split = s.find('=');

		if (split == -1)
			continue;
		
		SubParam * p = new SubParam(s.left(split), s.mid(split + 1));
		subParamList_.append(p);
	}
}

SubParam::SubParam()
	:	name_(""),
		value_("")
{
}

SubParam::SubParam(const QCString & name, const QCString & value)
	:	name_(name),
		value_(value)
{
}

	void
SubParam::setName(const QCString & name)
{
	name_ = name;
}

	void
SubParam::setValue(const QCString & value)
{
	value_ = value;
}

	QCString
SubParam::name()
{
	return name_;
}

	QCString
SubParam::value()
{
	return value_;
}

