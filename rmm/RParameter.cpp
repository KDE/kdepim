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
#include <string.h>
#include <qstring.h>
#include <RMM_Parameter.h>
#include <RMM_Token.h>

RParameter::RParameter()
{
	rmmDebug("ctor");
}

RParameter::RParameter(const RParameter & p)
	:	RMessageComponent()
{
	rmmDebug("ctor");
}

RParameter::~RParameter()
{
	rmmDebug("dtor");
}

	const RParameter &
RParameter::operator = (const RParameter & p)
{
	return *this;
}

	void
RParameter::parse()   
{
	rmmDebug("parse() called");
	
	int split = strRep_.find('=');
	
	if (split == -1) {
		rmmDebug("Invalid parameter");
		return;
	}
	
	attribute_	= strRep_.left(split).stripWhiteSpace();
	value_		= strRep_.right(strRep_.length() - split - 1).stripWhiteSpace();
}

	void
RParameter::assemble()
{
	rmmDebug("assemble() called");
	strRep_ = attribute_ + "=" + value_;
}

	void
RParameter::createDefault()
{
	attribute_ = value_ = "";
}

