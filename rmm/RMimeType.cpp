/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rikkus@postmaster.co.uk
	
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

	const RMimeType &
RMimeType::operator = (const RMimeType & t)
{
	return *this;
}

	MimeType
RMimeType::type() const
{
	return type_;
}

	void
RMimeType::setType(MimeType t)
{
	type_ = t;
}

	void
RMimeType::setType(const QString & s)
{
}

	MimeSubType
RMimeType::subType() const
{
	return subType_;
}

	void
RMimeType::setSubType(MimeSubType t)
{
}

	void
RMimeType::setSubType(const QString & s)
{
}

	const QString &
RMimeType::boundary() const
{
	return boundary_;
}

	void
RMimeType::setBoundary(const QString & s)
{
	boundary_ = s;
}

	const QString &
RMimeType::name() const
{
	return name_;
}

	void
RMimeType::setName(const QString & s)
{
	name_ = s;
}

	void
RMimeType::parse()
{
	rmmDebug("parse() called");
}

	void
RMimeType::assemble()
{
	rmmDebug("assemble() called");
}

