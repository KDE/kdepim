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

#include <RMM_ContentType.h>
#include <RMM_Token.h>

RContentType::RContentType()
{
	rmmDebug("ctor");
}

RContentType::RContentType(const RContentType & cte)
	:	RHeaderBody()
{
	rmmDebug("ctor");
}

RContentType::~RContentType()
{
	rmmDebug("dtor");
}

	const RContentType &
RContentType::operator = (const RContentType & cte)
{
	return *this;
}

	void
RContentType::parse()
{
	rmmDebug("parse() called");
	rmmDebug("strRep_ = " + strRep_);
	
	QString ts;
	int i = strRep_.find(";");
	
	if (i == -1)
	
		ts = strRep_;
	
	else {
	
		ts = strRep_.left(i);
		parameterList_ = strRep_.right(strRep_.length() - i + 1);
		parameterList_.parse();
	}
	
	int slash = ts.find('/');
	
	if (slash == -1) {
		rmmDebug("Invalid Content-Type");
		return;
	}
	
	type_ = ts.left(slash).stripWhiteSpace();
	subType_ = ts.right(ts.length() - slash - 1).stripWhiteSpace();
}

	void
RContentType::assemble()
{
	rmmDebug("assemble() called");
	
	strRep_ = type_ + "/" + subType_;
	
	parameterList_.assemble();
	
	if (parameterList_.count() == 0) return;
	
	strRep_ += QString(";\n    ");
	
	strRep_ += parameterList_.asString();

	rmmDebug("assembled to: \"" + strRep_ + "\"");
}

	void
RContentType::createDefault()
{
	rmmDebug("createDefault() called");
	type_ = "text";
	subType_ = "plain";
}

