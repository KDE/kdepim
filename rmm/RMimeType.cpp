/*
	Empath - Mailer for KDE
	
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

#include <ctype.h>
#include <stdlib.h>
#include <iostream.h>

#include <qstring.h>

#include <RMM_Parameter.h>
#include <RMM_MimeType.h>
#include <RMM_Token.h>
#include <RMM_Utility.h>
#include <RMM_Enum.h>

RMimeType::RMimeType()
{
	rmmDebug("ctor");
}

RMimeType::RMimeType(const RMimeType & t)
	:	RHeaderBody()
{
	rmmDebug("ctor");
}

RMimeType::~RMimeType()
{
	rmmDebug("dtor");
}

	RMimeType &
RMimeType::operator = (const RMimeType & t)
{
	rmmDebug("operator =");
    if (this == &t) return *this; // Avoid a = a
	
	boundary_		= t.boundary_;
	name_			= t.name_;
	type_			= t.type_;
	subType_		= t.subType_;
	parameterList_	= t.parameterList_;
	
	RHeaderBody::operator = (t);
	return *this;
}

	bool
RMimeType::operator == (RMimeType & t)
{
	parse();
	t.parse();
	
	return (
		boundary_		== t.boundary_	&&
		name_			== t.name_		&&
		type_			== t.type_		&&
		subType_		== t.subType_	&&
		parameterList_	== t.parameterList_);
}

	RMM::MimeType
RMimeType::type()
{
	return type_;
}

	void
RMimeType::setType(RMM::MimeType t)
{
	type_ = t;
}

	void
RMimeType::setType(const QCString & s)
{
}

	RMM::MimeSubType
RMimeType::subType()
{
	return subType_;
}

	void
RMimeType::setSubType(RMM::MimeSubType t)
{
}

	void
RMimeType::setSubType(const QCString & s)
{
}

	QCString
RMimeType::boundary()
{
	return boundary_;
}

	void
RMimeType::setBoundary(const QCString & s)
{
	boundary_ = s;
}

	QCString
RMimeType::name()
{
	return name_;
}

	void
RMimeType::setName(const QCString & s)
{
	name_ = s;
}

	void
RMimeType::_parse()
{
}

	void
RMimeType::_assemble()
{
}

	void
RMimeType::createDefault()
{
	rmmDebug("createDefault called");
}

