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

#ifdef __GNUG__
# pragma implementation "RMM_Cte.h"
#endif

#include <qstring.h>

#include <RMM_Cte.h>

namespace RMM {

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

	bool
RCte::operator == (RCte & c)
{
	parse();
	c.parse();

	return (mechanism_ == c.mechanism_);
}

	void
RCte::_parse()
{
	strRep_		= strRep_.stripWhiteSpace();
	
	if (!stricmp(strRep_, "7bit"))
		mechanism_ = RMM::CteType7bit;
	else if (!stricmp(strRep_, "8bit"))
		mechanism_ = RMM::CteType8bit;
	else if (!stricmp(strRep_, "base64"))
		mechanism_ = RMM::CteTypeBase64;
	else if (!stricmp(strRep_, "quoted-printable"))
		mechanism_ = RMM::CteTypeQuotedPrintable;
	else if (!strnicmp(strRep_, "x", 1))
		mechanism_ = RMM::CteTypeXtension;
	else 
		mechanism_ = RMM::CteTypeBinary;
}

	void
RCte::_assemble()
{
	switch (mechanism_) {

		case RMM::CteType7bit:
			strRep_ = "7bit";
			break;
			
		case RMM::CteType8bit:
			strRep_ = "8bit";
			break;
		
		case RMM::CteTypeBase64:
			strRep_ = "Base64";
			break;
		
		case RMM::CteTypeQuotedPrintable:
			strRep_ = "Quoted-Printable";
			break;
		
		case RMM::CteTypeXtension:
			break;
		
		case RMM::CteTypeBinary:
		default:
			strRep_ = "binary";
			break;
	}
}

	void
RCte::createDefault()
{
	rmmDebug("createDefault() called");
	mechanism_	= RMM::CteTypeBase64;
	parsed_		= true;
	assembled_	= false;
}


	RMM::CteType
RCte::mechanism()
{
	parse();
	return mechanism_;
}

	void
RCte::setMechanism(RMM::CteType t)
{
	mechanism_ = t;
	assembled_	= false;
}

};

