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

#include <RMM_Mechanism.h>
#include <RMM_Enum.h>

RMechanism::RMechanism()
{
	rmmDebug("ctor");
}

RMechanism::RMechanism(const RMechanism & m)
	:	RHeaderBody()
{
	rmmDebug("ctor");
}

RMechanism::~RMechanism()
{
	rmmDebug("dtor");
}

	RMechanism &
RMechanism::operator = (const RMechanism & m)
{
	rmmDebug("operator =");
    if (this == &m) return *this; // Don't do a = a.
	
	RHeaderBody::operator = (m);
	return *this;
}

	void
RMechanism::parse()
{
	rmmDebug("parse() called");
}

	void
RMechanism::assemble()
{
	rmmDebug("assemble() called");
}

	void
RMechanism::createDefault()
{
	rmmDebug("createDefault() called");
}

