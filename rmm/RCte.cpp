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

#include <qstring.h>

#include <RMM_Cte.h>

RCte::RCte()
{
	rmmDebug("ctor");
}

RCte::RCte(const RCte & cte)
	:	RHeaderBody()
{
	rmmDebug("ctor");
}

RCte::~RCte()
{
	rmmDebug("dtor");
}

	const RCte &
RCte::operator = (const RCte & cte)
{
	rmmDebug("operator =");
    if (this == &cte) return *this; // Don't do a = a.

	mechanism_ = cte.mechanism_;
	
	RHeaderBody::operator = (cte);

	return *this;
}

	void
RCte::parse()
{
	rmmDebug("parse() called");
	rmmDebug("strRep_ = " + strRep_);
	
	mechanism_ = strRep_.stripWhiteSpace();
}

	void
RCte::assemble()
{
	rmmDebug("assemble() called");
	strRep_ = mechanism_;
}

	void
RCte::createDefault()
{
	rmmDebug("createDefault() called");
	mechanism_ = "base64";
}

