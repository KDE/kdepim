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

#include <qstring.h>

#include <RMM_Enum.h>
#include <RMM_Token.h>
#include <RMM_Parameter.h>
#include <RMM_DispositionType.h>

RDispositionType::RDispositionType()
{
	rmmDebug("ctor");
}

RDispositionType::RDispositionType(const RDispositionType & t)
	:	RHeaderBody(t)
{
	rmmDebug("ctor");
}

	RDispositionType &
RDispositionType::operator = (const RDispositionType & t)
{
	rmmDebug("operator =");
    if (this == &t) return *this; // Don't do a = a.
	
	parameterList_	= t.parameterList_;
	dispType_		= t.dispType_;
	filename_		= t.filename_;
	
	RHeaderBody::operator = (t);
	
	return *this;
}


RDispositionType::~RDispositionType()
{
	rmmDebug("dtor");
}

	RMM::DispType
RDispositionType::type()
{
	parse();
    return dispType_;
}

	void
RDispositionType::set(RMM::DispType t)
{
}

	const QCString &
RDispositionType::filename()
{
	parse();
	return filename_;
}


	void
RDispositionType::setFilename(const QCString & s)
{
	parse();
	filename_ = s;
	assembled_ = false;
}

	void
RDispositionType::parse()
{
}

	void
RDispositionType::assemble()
{
}
	
	void
RDispositionType::createDefault()
{
}

