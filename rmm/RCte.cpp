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

#include <RMM_Cte.h>

RCte::RCte()
	:	RHeaderBody()
{
	rmmDebug("ctor");
}

RCte::RCte(const RCte & cte)
	:	RHeaderBody(cte)
{
	rmmDebug("ctor");
	assembled_	= false;
}

RCte::RCte(const QCString & s)
	:	RHeaderBody(s)
{
	rmmDebug("ctor with \"" + s + "\"");
}

RCte::~RCte()
{
	rmmDebug("dtor");
}

	RCte &
RCte::operator = (const RCte & cte)
{
	rmmDebug("operator =");
    if (this == &cte) return *this; // Don't do a = a.

	mechanism_ = cte.mechanism_;
	
	RHeaderBody::operator = (cte);
	
	return *this;
}

	RCte &
RCte::operator = (const QCString & s)
{
	rmmDebug("operator = \"" + s + "\"");
	RHeaderBody::operator = (s);
	return *this;
}

	void
RCte::parse()
{
	rmmDebug("parse() called");
	if (parsed_) return;
	rmmDebug("strRep_ = " + strRep_);
	
	mechanism_	= strRep_.stripWhiteSpace();
	
	parsed_		= true;
	assembled_	= false;
}

	void
RCte::assemble()
{
	parse();
	if (assembled_) return;
	rmmDebug("assemble() called");
	strRep_ = mechanism_;
	assembled_ = true;
}

	void
RCte::createDefault()
{
	rmmDebug("createDefault() called");
	mechanism_	= "base64";
	parsed_		= true;
	assembled_	= false;
}


	QCString
RCte::mechanism()	
{
	parse();
	return mechanism_;
}

	void
RCte::setMechanism(const QCString & m)
{
	mechanism_ = m;
	assembled_	= false;
}

